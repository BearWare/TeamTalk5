/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#include "AudioUnit.h"

#include "codec/MediaUtil.h"
#include "myace/MyACE.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

#import <AVFoundation/AVFoundation.h>
#import <AudioUnit/AudioUnit.h>

using namespace std;

static OSStatus AudioInputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                   const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                   UInt32 numFrames, AudioBufferList *buffers);

static OSStatus AudioOutputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                    const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                    UInt32 numFrames, AudioBufferList *buffers);

static OSStatus VPIOInputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                  const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                  UInt32 numFrames, AudioBufferList *buffers);

static OSStatus VPIOOutputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                   const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                   UInt32 numFrames, AudioBufferList *buffers);

namespace soundsystem {

    enum iOSSoundDevice
    {
        REMOTEIO_DEVICE_ID                  = (0 & SOUND_DEVICEID_MASK),
        VOICEPROCESSINGIO_DEVICE_ID         = (1 & SOUND_DEVICEID_MASK)
    };

#define DEFAULT_DEVICE_ID (REMOTEIO_DEVICE_ID)

#define SPEAKER_DEVICE_ID 1
#define DEFAULT_SAMPLERATE 48000

    typedef SoundSystemBase< SoundGroup, struct AUInputStreamer,
                             struct AUOutputStreamer, struct AUDuplexStreamer > AudUnitBase;

    struct AUInputStreamer : InputStreamer
    {
        msg_queue_t samples_queue;

        AudioUnit audunit;
        bool recording = false;
        
        AUInputStreamer(StreamCapture* r, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : InputStreamer(r, sg, fs, sr, chs, sndsys, devid)
        , audunit(nil)
        {
            samples_queue.high_water_mark(1024*128);
            samples_queue.low_water_mark(1024*128);
        }
    };

    struct AUOutputStreamer : public OutputStreamer
    {
        AudioUnit audunit;
        bool playing;
        
        msg_queue_t samples_queue;

        AUOutputStreamer(StreamPlayer* p, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : OutputStreamer(p, sg, fs, sr, chs, sndsys, devid)
            , audunit(nil), playing(false)
        {
            samples_queue.high_water_mark(1024*128);
            samples_queue.low_water_mark(1024*128);
        }

        ~AUOutputStreamer()
        {
        }
        
        UInt32 FillBuffer(AudioBuffer& buf, UInt32 buf_usage)
        {
            assert(samples_queue.state() == msg_queue_t::ACTIVATED);
            int mslen;
            while(buf_usage < buf.mDataByteSize && (mslen = samples_queue.message_length()))
            {
                ACE_Message_Block* mb;
                ACE_Time_Value tv;
                int ret = samples_queue.dequeue(mb, &tv);
                int err = ACE_OS::last_error();
                assert(ret >= 0);
                if(ret < 0)
                    return buf_usage;
            
                size_t min_bytes = std::min(buf.mDataByteSize - buf_usage, (UInt32)mb->length());
                char* buf_ptr = reinterpret_cast<char*>(buf.mData);
                ACE_OS::memcpy(&buf_ptr[buf_usage], mb->rd_ptr(), min_bytes);
                mb->rd_ptr(min_bytes);
                if(mb->length() == 0)
                {
                    mb->release();
                }
                else
                {
                    ret = samples_queue.enqueue_head(mb, &tv);
                    assert(ret >= 0);
                }
                assert(samples_queue.message_length() == mslen - min_bytes);
                buf_usage += min_bytes;
            }
            return buf_usage;
        }
    };

    struct AUDuplexStreamer : DuplexStreamer
    {
        AudioUnit recorder;
        AudioUnit player;
        AUDuplexStreamer(StreamDuplex* d, int sg, int fs, int sr, 
                         int inchs, int outchs, SoundAPI out_sndsys,
                         int inputdeviceid, int outputdeviceid) 
            : DuplexStreamer(d, sg, fs, sr, inchs, outchs, out_sndsys, inputdeviceid, outputdeviceid)
            , recorder(nil), player(nil) {}
    };

    // Echo cancellation requires recording and playback to go through
    // the same Voice-Processing I/O unit (separate units stopped
    // working in iOS 26), so the input and output streamer share it.
    struct SharedVPIO
    {
        AudioUnit unit = nil;
        std::atomic<AUInputStreamer*> input{nullptr};
        std::atomic<AUOutputStreamer*> output{nullptr};
    };

#if TARGET_IPHONE_SIMULATOR
#else
bool EnableSpeakerOutput(bool enable)
{
    OSStatus status;
    UInt32 flag = 0;
    UInt32 propSize = sizeof(flag);
    if(enable)
    {
        // enable speaker instead of ear piece
        flag = kAudioSessionOverrideAudioRoute_Speaker;
        status = AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute,
                                         sizeof (flag), &flag);
        assert(status == noErr);
        MYTRACE(ACE_TEXT("Enabling speaker output\n"));
    }
    else
    {
        flag = kAudioSessionOverrideAudioRoute_None;
        status = AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute,
                                         sizeof (flag), &flag);
        assert(status == noErr);
        MYTRACE(ACE_TEXT("Disabling speaker output\n"));
    }

    return status == noErr;
}
#endif /* TARGET_IPHONE_SIMULATOR */

    class AudUnit : public AudUnitBase
    {
    public:
        AudUnit()
        {
#if TARGET_IPHONE_SIMULATOR

#else
            AVAudioSession *session = [AVAudioSession sharedInstance];

            if( [[AVAudioSession sharedInstance] respondsToSelector:@selector(requestRecordPermission)] )
            {
                [[AVAudioSession sharedInstance] requestRecordPermission];
            }
    
#endif

            Init();
        }

        virtual ~AudUnit()
        {
            Close();
            MYTRACE(ACE_TEXT("~AudUnit()\n"));
        }
        
        static std::shared_ptr<AudUnit> getInstance()
        {
            static std::shared_ptr<AudUnit> p(new AudUnit());
            return p;
        }

        bool Init()
        {
            AVAudioSession *session = [AVAudioSession sharedInstance];

#if TARGET_IPHONE_SIMULATOR

#else
/* the follow code causes Bluetooth headsets to stop working
  
// set preferred buffer size
OSStatus status;
Float32 preferredBufferSize = .04; // in seconds
status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(preferredBufferSize), &preferredBufferSize);
assert(status == noErr);

// get actual buffer size
Float32 audioBufferSize;
UInt32 size = sizeof (audioBufferSize);
status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &audioBufferSize);
assert(status == noErr);
*/
            [session setCategory:AVAudioSessionCategoryPlayAndRecord 
             withOptions:AVAudioSessionCategoryOptionAllowBluetooth error:nil];

#endif

            [session setActive:YES error:nil];

            MYTRACE(ACE_TEXT("Starting new AudioSession\n"));

            // MYTRACE_COND(status != noErr, ACE_TEXT("Failed to set property kAudioSessionProperty_AudioCategory"));

            // Float32 bufferSizeInSec = 0.02f;
            // status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration,
            //                                  sizeof(Float32), &bufferSizeInSec);
            // assert(status == noErr);
            // MYTRACE_COND(status != noErr, ACE_TEXT("Failed to set property kAudioSessionProperty_PreferredHardwareIOBufferDuration"));

            RefreshDevices();

            return true;
        }

        void Close()
        {
            AVAudioSession *session = [AVAudioSession sharedInstance];
            [session setActive:NO error:nil];
            MYTRACE(ACE_TEXT("Closing AudioSession\n"));
        }

        soundgroup_t NewSoundGroup()
        {
            return soundgroup_t(new SoundGroup());
        }

        void RemoveSoundGroup(soundgroup_t)
        {
        }

        void FillDevices(sounddevices_t& sounddevs)
        {
            DeviceInfo dev;
            dev.soundsystem = SOUND_API_AUDIOUNIT;

            for(size_t sr=0;sr<standardSampleRates.size();sr++)
            {
                dev.input_samplerates.insert(standardSampleRates[sr]);
                dev.output_samplerates.insert(standardSampleRates[sr]);
            }    

            dev.max_input_channels = 2;
            dev.max_output_channels = 2;
            dev.default_samplerate = DEFAULT_SAMPLERATE;

            dev.input_channels.insert(1);
            dev.input_channels.insert(2);
            dev.output_channels.insert(1);
            dev.output_channels.insert(2);

            // Remote i/o device

            dev.devicename = ACE_TEXT("Remote I/O Unit");
            dev.id = REMOTEIO_DEVICE_ID;
            sounddevs[dev.id] = dev;


            // voice processing i/o device
    
            dev.devicename = ACE_TEXT("Voice-Processing I/O Unit");
            dev.id = VOICEPROCESSINGIO_DEVICE_ID;
            sounddevs[dev.id] = dev;

            // //TODO: detect if iPad, then don't include this device.
            // //if(UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
            // {
            //     dev.devicename = ACE_TEXT("Speaker output");
            //     dev.id = SPEAKER_DEVICE_ID;
            //     dev.max_input_channels = 0;
            //     dev.input_channels.clear();

            //     sounddevs[dev.id] = dev;
            // }
        }

        bool GetDefaultDevices(int& inputdeviceid,
                               int& outputdeviceid)
        {
            GetDefaultDevices(SOUND_API_AUDIOUNIT, inputdeviceid, outputdeviceid);
            return true;
        }
 
        bool GetDefaultDevices(SoundAPI sndsys,
                               int& inputdeviceid,
                               int& outputdeviceid)
       {
           inputdeviceid = outputdeviceid = DEFAULT_DEVICE_ID;
           return true;
       }

#define kOutputBus 0
#define kInputBus 1

        AudioUnit NewInput(int inputdeviceid, int samplerate, int channels)
        {
            AudioStreamBasicDescription format = {};
            format.mSampleRate = samplerate;
            format.mFormatID = kAudioFormatLinearPCM;
            format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
            format.mBitsPerChannel = 8 * sizeof(short);
            format.mChannelsPerFrame = channels;
            format.mBytesPerFrame = channels * sizeof(short);
            format.mFramesPerPacket = 1;
            format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
            format.mReserved = 0;

            OSStatus status;
    
            AudioComponentDescription componentDescription = {};
            componentDescription.componentType = kAudioUnitType_Output;
            switch (inputdeviceid)
            {
            case REMOTEIO_DEVICE_ID :
                componentDescription.componentSubType = kAudioUnitSubType_RemoteIO;
                break;
            case VOICEPROCESSINGIO_DEVICE_ID :
                componentDescription.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
                break;
            default :
                return nil;
            }
            componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
            componentDescription.componentFlags = 0;
            componentDescription.componentFlagsMask = 0;

            UInt32 flag = 1;

            AudioUnit audioUnit = nil;
            AudioComponent component = AudioComponentFindNext(NULL, &componentDescription);
            status = AudioComponentInstanceNew(component, &audioUnit);
            assert(status == noErr);
            if(status != noErr)
                return nil;

            status = AudioUnitSetProperty(audioUnit, 
                                          kAudioOutputUnitProperty_EnableIO,
                                          kAudioUnitScope_Input, 
                                          kInputBus, 
                                          &flag, sizeof(flag));
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            status = AudioUnitSetProperty(audioUnit, 
                                          kAudioUnitProperty_StreamFormat,
                                          kAudioUnitScope_Output, 
                                          kInputBus, 
                                          &format, sizeof(format));
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            return audioUnit;

        fail:
            status = AudioUnitUninitialize(audioUnit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to destroy audio input\n"));
            status = AudioComponentInstanceDispose(audioUnit);
            assert(status == noErr);
            return nil;
        }

        AudioUnit NewOutput(int outputdeviceid, int samplerate, int channels)
        {
            AudioStreamBasicDescription format = {};
            format.mSampleRate = samplerate;
            format.mFormatID = kAudioFormatLinearPCM;
            format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
            format.mBitsPerChannel = 8 * sizeof(short);
            format.mChannelsPerFrame = channels;
            format.mBytesPerFrame = channels * sizeof(short);
            format.mFramesPerPacket = 1;
            format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
            format.mReserved = 0;

            OSStatus status;

            AudioComponentDescription componentDescription = {};
            componentDescription.componentType = kAudioUnitType_Output;
            switch (outputdeviceid)
            {
            case REMOTEIO_DEVICE_ID :
                componentDescription.componentSubType = kAudioUnitSubType_RemoteIO;
                break;
            case VOICEPROCESSINGIO_DEVICE_ID :
                componentDescription.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
                break;
            default :
                return nil;
            }
            componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
            componentDescription.componentFlags = 0;
            componentDescription.componentFlagsMask = 0;

            UInt32 flag = 1;

            AudioUnit audioUnit = nil;
            AudioComponent component = AudioComponentFindNext(NULL, &componentDescription);
            status = AudioComponentInstanceNew(component, &audioUnit);
            assert(status == noErr);
            if(status != noErr)
                goto fail;
    
            status = AudioUnitSetProperty(audioUnit,
                                          kAudioOutputUnitProperty_EnableIO, 
                                          kAudioUnitScope_Output, 
                                          kOutputBus,
                                          &flag, sizeof(flag));
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            status = AudioUnitSetProperty(audioUnit, 
                                          kAudioUnitProperty_StreamFormat, 
                                          kAudioUnitScope_Input,
                                          kOutputBus,
                                          &format, sizeof(format));
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            flag = 0;
            status = AudioUnitSetProperty(audioUnit,
                                          kAudioUnitProperty_ShouldAllocateBuffer,
                                          kAudioUnitScope_Output,
                                          kInputBus,
                                          &flag, sizeof(flag));
            assert(status == noErr);
            if(status != noErr)
                goto fail;
    
            return audioUnit;

        fail:
            status = AudioUnitUninitialize(audioUnit);
            assert(status == noErr);
            status = AudioComponentInstanceDispose(audioUnit);
            assert(status == noErr);
            return nil;
        }


        // Voice-Processing I/O unit with both recording and playback enabled
        AudioUnit NewSharedVPIO(int samplerate, int channels)
        {
            AudioUnit audioUnit = NewInput(VOICEPROCESSINGIO_DEVICE_ID, samplerate, channels);
            if(audioUnit == nil)
                return nil;

            // play back in the same format as recording
            AudioStreamBasicDescription format = {};
            UInt32 size = sizeof(format);
            UInt32 flag = 1;
            AURenderCallbackStruct inputCallback = { VPIOInputCallback, &m_vpio };
            AURenderCallbackStruct outputCallback = { VPIOOutputCallback, &m_vpio };

            OSStatus status;
            status = AudioUnitGetProperty(audioUnit, kAudioUnitProperty_StreamFormat,
                                          kAudioUnitScope_Output, kInputBus, &format, &size);
            if(status != noErr)
                goto fail;
            status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO,
                                          kAudioUnitScope_Output, kOutputBus, &flag, sizeof(flag));
            if(status != noErr)
                goto fail;
            status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat,
                                          kAudioUnitScope_Input, kOutputBus, &format, sizeof(format));
            if(status != noErr)
                goto fail;
            status = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_SetInputCallback,
                                          kAudioUnitScope_Output, kInputBus,
                                          &inputCallback, sizeof(inputCallback));
            if(status != noErr)
                goto fail;
            status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback,
                                          kAudioUnitScope_Input, kOutputBus,
                                          &outputCallback, sizeof(outputCallback));
            if(status != noErr)
                goto fail;
            status = AudioUnitInitialize(audioUnit);
            if(status != noErr)
                goto fail;

            return audioUnit;

        fail:
            MYTRACE(ACE_TEXT("Failed to create shared voice-processing unit, status %d\n"), (int)status);
            AudioComponentInstanceDispose(audioUnit);
            return nil;
        }

        // Get the shared voice-processing unit. Returns nil if it uses a
        // different format.
        AudioUnit SharedVPIOUnit(int samplerate, int channels)
        {
            if(m_vpio.unit == nil)
            {
                m_vpio.unit = NewSharedVPIO(samplerate, channels);
                m_vpio_samplerate = samplerate;
                m_vpio_channels = channels;
            }
            else if(samplerate != m_vpio_samplerate || channels != m_vpio_channels)
                return nil;
            return m_vpio.unit;
        }

        bool IsSharedVPIO(AudioUnit audioUnit) const
        {
            return audioUnit != nil && audioUnit == m_vpio.unit;
        }

        // Run the shared voice-processing unit while recording or playing
        bool UpdateSharedVPIO()
        {
            std::lock_guard<std::recursive_mutex> g(m_vpio_mutex);
            AUInputStreamer* input = m_vpio.input;
            AUOutputStreamer* output = m_vpio.output;
            if((input && input->recording) || (output && output->playing))
                return AudioOutputUnitStart(m_vpio.unit) == noErr;
            return AudioOutputUnitStop(m_vpio.unit) == noErr;
        }

        void DetachSharedVPIO(SoundStreamer* streamer)
        {
            std::lock_guard<std::recursive_mutex> g(m_vpio_mutex);

            // stopping waits for active callbacks to return
            AudioOutputUnitStop(m_vpio.unit);
            if(m_vpio.input == streamer)
                m_vpio.input = nullptr;
            if(m_vpio.output == streamer)
                m_vpio.output = nullptr;

            if(m_vpio.input || m_vpio.output)
            {
                UpdateSharedVPIO();
                return;
            }

            AudioUnitUninitialize(m_vpio.unit);
            AudioComponentInstanceDispose(m_vpio.unit);
            m_vpio.unit = nil;
        }

        inputstreamer_t NewStream(StreamCapture* capture, int inputdeviceid,
                                  int sndgrpid, int samplerate, int channels,
                                  int framesize)
        {
            if(inputdeviceid == VOICEPROCESSINGIO_DEVICE_ID)
            {
                std::lock_guard<std::recursive_mutex> g(m_vpio_mutex);
                AudioUnit sharedUnit;
                if(m_vpio.input == nullptr && (sharedUnit = SharedVPIOUnit(samplerate, channels)))
                {
                    inputstreamer_t streamer(new AUInputStreamer(capture, sndgrpid,
                                                                 framesize, samplerate,
                                                                 channels, SOUND_API_AUDIOUNIT,
                                                                 inputdeviceid));
                    streamer->audunit = sharedUnit;
                    m_vpio.input = streamer.get();
                    return streamer;
                }
            }

            AudioUnit audioUnit = NewInput(inputdeviceid, samplerate, channels);
            if(audioUnit == nil)
                return inputstreamer_t();

            inputstreamer_t streamer(new AUInputStreamer(capture, sndgrpid, 
                                                         framesize, samplerate,
                                                         channels, SOUND_API_AUDIOUNIT,
                                                         inputdeviceid));

            AURenderCallbackStruct callbackStruct = {};
            callbackStruct.inputProc = AudioInputCallback; // Render function
            callbackStruct.inputProcRefCon = streamer.get();
            OSStatus status;
            status = AudioUnitSetProperty(audioUnit, 
                                          kAudioOutputUnitProperty_SetInputCallback,
                                          kAudioUnitScope_Output, 
                                          kInputBus, 
                                          &callbackStruct, sizeof(callbackStruct));
            assert(status == noErr);
            if(status != noErr)
                goto fail;
    
            status = AudioUnitInitialize(audioUnit);
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            streamer->audunit = audioUnit;
    
            MYTRACE(ACE_TEXT("Opened and started input device %d with samplerate %d and channels %d\n"),
                    inputdeviceid, samplerate, channels);

            return streamer;

        fail:

            status = AudioUnitUninitialize(audioUnit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to destroy audio input\n"));
            status = AudioComponentInstanceDispose(audioUnit);
            assert(status == noErr);

            MYTRACE(ACE_TEXT("Failed to start input device %d, status %d\n"), inputdeviceid, (int)status);

            return inputstreamer_t();
        }

        bool StartStream(inputstreamer_t streamer)
        {
            if(IsSharedVPIO(streamer->audunit))
            {
                streamer->recording = true;
                return UpdateSharedVPIO();
            }

            OSStatus status;
            assert(streamer->audunit);
            status = AudioOutputUnitStart(streamer->audunit);
            assert(status == noErr);
            streamer->recording = (status == noErr);
            return status == noErr;
        }

        void CloseStream(inputstreamer_t streamer)
        {
            if(IsSharedVPIO(streamer->audunit))
            {
                streamer->recording = false;
                DetachSharedVPIO(streamer.get());
                return;
            }

            OSStatus status;

            assert(streamer->audunit);
            status = AudioOutputUnitStop(streamer->audunit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to stop audio input\n"));
            status = AudioUnitUninitialize(streamer->audunit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to close audio input\n"));
            status = AudioComponentInstanceDispose(streamer->audunit);
            assert(status == noErr);
        }

        bool IsStreamStopped(inputstreamer_t streamer)
        {
            assert(streamer->audunit);
            return !streamer->recording;
        }

        outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                   int sndgrpid, int samplerate, int channels, 
                                   int framesize)
        {
            outputstreamer_t streamer(new AUOutputStreamer(player, sndgrpid,
                                                           framesize, samplerate,
                                                           channels, SOUND_API_AUDIOUNIT,
                                                           outputdeviceid));
            streamer->playing = false;

            if(outputdeviceid == VOICEPROCESSINGIO_DEVICE_ID)
            {
                std::lock_guard<std::recursive_mutex> g(m_vpio_mutex);
                if(m_vpio.output == nullptr &&
                   (streamer->audunit = SharedVPIOUnit(samplerate, channels)))
                {
                    m_vpio.output = streamer.get();
                    return streamer;
                }
            }

            if (!NewStreamer(outputdeviceid, streamer))
                return outputstreamer_t();

            return streamer;
        }

        bool NewStreamer(int outputdeviceid, outputstreamer_t streamer)
        {
            AudioUnit audioUnit = NewOutput(outputdeviceid, streamer->samplerate,
                                            streamer->channels);
            if(audioUnit == nil)
                return false;

            streamer->audunit = audioUnit;

            // setup callback function
            AURenderCallbackStruct callbackStruct = {};
            callbackStruct.inputProc = AudioOutputCallback;
            callbackStruct.inputProcRefCon = streamer.get();

            OSStatus status;
            status = AudioUnitSetProperty(audioUnit, 
                                          kAudioUnitProperty_SetRenderCallback, 
                                          kAudioUnitScope_Input, 
                                          kOutputBus,
                                          &callbackStruct, sizeof(callbackStruct));
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            MYTRACE(ACE_TEXT("Opened output device %d with samplerate %d and channels %d\n"),
                    outputdeviceid, streamer->samplerate, streamer->channels);
            status = AudioUnitInitialize(audioUnit);
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            return true;

        fail:
            status = AudioUnitUninitialize(audioUnit);
            assert(status == noErr);
            status = AudioComponentInstanceDispose(audioUnit);
            assert(status == noErr);
            return false;
        }

        void CloseStream(outputstreamer_t streamer)
        {
            if(IsSharedVPIO(streamer->audunit))
            {
                streamer->playing = false;
                DetachSharedVPIO(streamer.get());
                return;
            }

            if (streamer->audunit)
            {
                // close streamer's audio unit instance
                OSStatus status;
                status = AudioOutputUnitStop(streamer->audunit);
                assert(status == noErr);
                status = AudioUnitUninitialize(streamer->audunit);
                assert(status == noErr);
                status = AudioComponentInstanceDispose(streamer->audunit);
                assert(status == noErr);
            }
        }

        bool StartStream(outputstreamer_t streamer)
        {
            streamer->playing = true;
            if(IsSharedVPIO(streamer->audunit))
                return UpdateSharedVPIO();

            if (streamer->audunit)
            {
                OSStatus status;
                status = AudioOutputUnitStart(streamer->audunit);

                MYTRACE_COND(status != noErr, ACE_TEXT("Failed to start output audio queue\n"));

                MYTRACE(ACE_TEXT("Start stream with samplerate %d and channels %d\n"),
                        streamer->samplerate, streamer->channels);

                return status == noErr;
            }
            return true;
        }

        bool StopStream(outputstreamer_t streamer)
        {
            streamer->playing = false;
            if(IsSharedVPIO(streamer->audunit))
                return UpdateSharedVPIO();

            if (streamer->audunit)
            {
                OSStatus status;
                status = AudioOutputUnitStop(streamer->audunit);
                MYTRACE_COND(status != noErr, ACE_TEXT("Failed to stop output audio queue\n"));
                return status == noErr;
            }
            return true;
        }

        bool IsStreamStopped(outputstreamer_t streamer)
        {
            return !streamer->playing;
        }

        duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                   int outputdeviceid, int sndgrpid,
                                   int samplerate, int input_channels, 
                                   int output_channels, int framesize)
        {
            return duplexstreamer_t();
        }

        void CloseStream(duplexstreamer_t streamer)
        {
        }

        bool StartStream(duplexstreamer_t streamer)
        {
            return false;
        }
        
        bool IsStreamStopped(duplexstreamer_t streamer)
        {
            return true;
        }

    private:
        SharedVPIO m_vpio;
        int m_vpio_samplerate = 0, m_vpio_channels = 0;
        std::recursive_mutex m_vpio_mutex;
    };

    soundsystem_t getAudUnit()
    {
        return AudUnit::getInstance();
    }

} //namespace

using soundsystem::AUInputStreamer;
using soundsystem::AUOutputStreamer;
using soundsystem::OutputStreamer;

static OSStatus AudioInputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                   const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                   UInt32 numFrames, AudioBufferList *buffers)
{
    AUInputStreamer* streamer = reinterpret_cast<AUInputStreamer*>(userData);

    ACE_Message_Block* mb;
    ACE_NEW_RETURN(mb, ACE_Message_Block(numFrames * streamer->channels * sizeof(short)), noErr);
    
    AudioBufferList bufList = {};
    bufList.mNumberBuffers = 1;
    bufList.mBuffers[0].mNumberChannels = streamer->channels;
    bufList.mBuffers[0].mDataByteSize = mb->size();
    bufList.mBuffers[0].mData = mb->wr_ptr();

    OSStatus status;
    status = AudioUnitRender(streamer->audunit, actionFlags, audioTimeStamp, busNumber, numFrames, &bufList);
    assert(status == noErr);
    if(status != noErr || bufList.mBuffers[0].mDataByteSize == 0)
    {
        mb->release();
        return noErr;
    }
    
    mb->wr_ptr(bufList.mBuffers[0].mDataByteSize);
    ACE_Time_Value tv;
    int ret = streamer->samples_queue.enqueue_tail(mb, &tv);
    if(ret < 0)
    {
        mb->release();
        return noErr;
    }
    
    UInt32 framebytes = PCM16_BYTES(streamer->channels, streamer->framesize);
    UInt32 copied = 0;
    std::vector<char> buffer(framebytes);
    while(streamer->samples_queue.message_length() >= framebytes - copied)
    {
        ret = streamer->samples_queue.dequeue(mb, &tv);
        assert(ret >= 0);
        if(ret < 0)
            return noErr;
        
        UInt32 min_bytes = std::min((UInt32)mb->length(), framebytes - copied);
        ACE_OS::memcpy(&buffer[copied], mb->rd_ptr(), min_bytes);
        mb->rd_ptr(min_bytes);

        copied += min_bytes;
        assert(copied <= framebytes);
        
        if(copied == framebytes)
        {
            streamer->recorder->StreamCaptureCb(*streamer, reinterpret_cast<short*>(&buffer[0]),
                                                streamer->framesize);
            copied = 0;
        }
        
        if(mb->length() == 0)
        {
            mb->release();
        }
        else
        {
            ret = streamer->samples_queue.enqueue_head(mb, &tv);
            assert(ret >= 0);
            if(ret < 0)
            {
                mb->release();
                return noErr;
            }
        }
    }
    
    return noErr;
}

static OSStatus AudioOutputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                    const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                    UInt32 numFrames, AudioBufferList *buffers)
{
    AUOutputStreamer* streamer = reinterpret_cast<AUOutputStreamer*>(userData);
    
    int ret;
    UInt32 buf_usage = 0;
    UInt32 buf_index = 0;
    
    while(true)
    {
        for(;buf_index < buffers->mNumberBuffers;)
        {
            AudioBuffer& buf = buffers->mBuffers[buf_index];
            buf_usage = streamer->FillBuffer(buf, buf_usage);
            if(buf_usage == buf.mDataByteSize)
            {
                buf_usage = 0;
                buf_index++;
            }
            else
            {
                break;
            }
        }
        
        //all iOS provided buffers have been filled, so return
        if(buf_index == buffers->mNumberBuffers)
            return noErr;
        
        //perform a new callback to get more data for buffers
        assert(streamer->samples_queue.message_length() == 0);
        assert(buf_usage < buffers->mBuffers[buf_index].mDataByteSize);
        
        int cbbytes = PCM16_BYTES(streamer->channels, streamer->framesize);
        ACE_Message_Block* mb;
        ACE_NEW_RETURN(mb, ACE_Message_Block(cbbytes), noErr);
        
        short* samples_buffer = reinterpret_cast<short*>(mb->wr_ptr());
        assert(streamer->player);
        streamer->player->StreamPlayerCb(*streamer, &samples_buffer[0], streamer->framesize);
        //soft volume also handles mute
        int mastervol = soundsystem::getAudUnit()->GetMasterVolume(streamer->sndgrpid);
        bool mastermute = soundsystem::getAudUnit()->IsAllMute(streamer->sndgrpid);
        SoftVolume(*streamer, &samples_buffer[0], streamer->framesize, mastervol, mastermute);
        mb->wr_ptr(cbbytes);
        
        ACE_Time_Value tv;
        ret = streamer->samples_queue.enqueue_tail(mb, &tv);
        assert(ret >= 0);
        if(ret < 0)
        {
            mb->release();
            return noErr;
        }
    }
    return noErr;
}

static OSStatus VPIOInputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                  const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                  UInt32 numFrames, AudioBufferList *buffers)
{
    auto vpio = reinterpret_cast<soundsystem::SharedVPIO*>(userData);
    AUInputStreamer* streamer = vpio->input;
    if(streamer == nullptr || !streamer->recording)
        return noErr;

    return AudioInputCallback(streamer, actionFlags, audioTimeStamp, busNumber, numFrames, buffers);
}

static OSStatus VPIOOutputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                   const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                   UInt32 numFrames, AudioBufferList *buffers)
{
    auto vpio = reinterpret_cast<soundsystem::SharedVPIO*>(userData);
    AUOutputStreamer* streamer = vpio->output;
    if(streamer && streamer->playing)
        return AudioOutputCallback(streamer, actionFlags, audioTimeStamp, busNumber, numFrames, buffers);

    // unit is running for recording only
    for(UInt32 i = 0; i < buffers->mNumberBuffers; ++i)
        ACE_OS::memset(buffers->mBuffers[i].mData, 0, buffers->mBuffers[i].mDataByteSize);
    *actionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    return noErr;
}

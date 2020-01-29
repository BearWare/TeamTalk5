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

#include <codec/MediaUtil.h>
#include <myace/MyACE.h>

#include <vector>
#include <assert.h>

#include <memory>
#include <algorithm>

#import <AVFoundation/AVFoundation.h>
#import <AudioUnit/AudioUnit.h>

using namespace std;

static OSStatus AudioInputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                   const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                   UInt32 numFrames, AudioBufferList *buffers);

static OSStatus AudioOutputCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                    const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                    UInt32 numFrames, AudioBufferList *buffers);

static OSStatus AudioMuxedCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
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

        std::vector<AudUnitBase::outputstreamer_t> streamers;
        std::recursive_mutex streamers_mtx;

        AUOutputStreamer(StreamPlayer* p, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : OutputStreamer(p, sg, fs, sr, chs, sndsys, devid)
            , audunit(nil), playing(false)
        {
            samples_queue.high_water_mark(1024*128);
            samples_queue.low_water_mark(1024*128);
        }

        ~AUOutputStreamer()
        {
            assert(streamers.empty());
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
        typedef std::vector<outputstreamer_t> muxed_streamers_t;
        muxed_streamers_t m_muxed_streamers;
        std::recursive_mutex m_mux_lock;

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
    
            AudioComponentDescription componentDescription;
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

            AudioComponentDescription componentDescription;
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


        inputstreamer_t NewStream(StreamCapture* capture, int inputdeviceid, 
                                  int sndgrpid, int samplerate, int channels,
                                  int framesize)
        {
            AudioUnit audioUnit = NewInput(inputdeviceid, samplerate, channels);
            if(audioUnit == nil)
                return inputstreamer_t();

            inputstreamer_t streamer(new AUInputStreamer(capture, sndgrpid, 
                                                         framesize, samplerate,
                                                         channels, SOUND_API_AUDIOUNIT,
                                                         inputdeviceid));

            AURenderCallbackStruct callbackStruct;
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
            OSStatus status;
            assert(streamer->audunit);
            status = AudioOutputUnitStart(streamer->audunit);
            assert(status == noErr);
            return status == noErr;
        }

        void CloseStream(inputstreamer_t streamer)
        {
            OSStatus status;

            assert(streamer->audunit);
            status = AudioOutputUnitStop(streamer->audunit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to stop audio input\n"));
            status = AudioUnitUninitialize(streamer->audunit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to close audio input\n"));
            status = AudioComponentInstanceDispose(streamer->audunit);
            assert(status == noErr);
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

            if (outputdeviceid == VOICEPROCESSINGIO_DEVICE_ID)
            {
                if (!NewMuxedStreamer(streamer))
                    return outputstreamer_t();
            }
            else
            {
                if (!NewStreamer(outputdeviceid, streamer))
                    return outputstreamer_t();
            }

            return streamer;
        }

        bool NewMuxedStreamer(outputstreamer_t streamer)
        {
            // retrieve shared player among all streams
            std::lock_guard<std::recursive_mutex> g(m_mux_lock);
            outputstreamer_t muxed = GetMuxedStreamer(streamer);
            if (muxed)
            {
                std::lock_guard<std::recursive_mutex> g(muxed->streamers_mtx);
                // add to muxed playback
                muxed->streamers.push_back(streamer);

                MYTRACE(ACE_TEXT("Added new stream to muxed output device with samplerate %d and channels %d\n"),
                        streamer->samplerate, streamer->channels);

                return true;
            }

            AudioUnit audioUnit = NewOutput(VOICEPROCESSINGIO_DEVICE_ID,
                                            streamer->samplerate, 
                                            streamer->channels);

            if (audioUnit == nil)
                return false;

            muxed.reset(new AUOutputStreamer(NULL,
                                             streamer->sndgrpid,
                                             streamer->framesize,
                                             streamer->samplerate,
                                             streamer->channels,
                                             streamer->soundsystem,
                                             VOICEPROCESSINGIO_DEVICE_ID));
            muxed->audunit = audioUnit;

            // setup callback function
            AURenderCallbackStruct callbackStruct;
            callbackStruct.inputProc = AudioMuxedCallback;
            callbackStruct.inputProcRefCon = muxed.get();
            OSStatus status;
            status = AudioUnitSetProperty(audioUnit, 
                                          kAudioUnitProperty_SetRenderCallback, 
                                          kAudioUnitScope_Input, 
                                          kOutputBus,
                                          &callbackStruct, sizeof(callbackStruct));
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            MYTRACE(ACE_TEXT("Opened new muxed output device with samplerate %d and channels %d\n"),
                    streamer->samplerate, streamer->channels);

            status = AudioUnitInitialize(audioUnit);
            assert(status == noErr);
            if(status != noErr)
                goto fail;

            status = AudioOutputUnitStart(audioUnit);
            MYTRACE_COND(status != noErr, ACE_TEXT("Failed to start output audio queue\n"));
            if (status != noErr)
                goto fail;

            MYTRACE(ACE_TEXT("Started muxed output with samplerate %d and channels %d\n"),
                    muxed->samplerate, muxed->channels);

            m_muxed_streamers.push_back(muxed);

            // add to muxed playback
            muxed->streamers.push_back(streamer);

            return true;

        fail:
            status = AudioUnitUninitialize(audioUnit);
            assert(status == noErr);
            status = AudioComponentInstanceDispose(audioUnit);
            assert(status == noErr);

            return false;
        }

        bool NewStreamer(int outputdeviceid, outputstreamer_t streamer)
        {
            AudioUnit audioUnit = NewOutput(outputdeviceid, streamer->samplerate,
                                            streamer->channels);
            if(audioUnit == nil)
                return false;

            streamer->audunit = audioUnit;

            // setup callback function
            AURenderCallbackStruct callbackStruct;
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
            AudioUnit audunit = nil;

            // remove from shared streamers
            std::lock_guard<std::recursive_mutex> g(m_mux_lock);
            outputstreamer_t muxed = GetMuxedStreamer(streamer);
            if (muxed)
            {
                std::lock_guard<std::recursive_mutex> g2(muxed->streamers_mtx);
                auto i = std::find(muxed->streamers.begin(), muxed->streamers.end(), streamer);
                if (i != muxed->streamers.end())
                    muxed->streamers.erase(i);

                if (muxed->streamers.empty())
                {
                    i = std::find(m_muxed_streamers.begin(), m_muxed_streamers.end(), muxed);
                    if (i != m_muxed_streamers.end())
                        m_muxed_streamers.erase(i);
                    audunit = muxed->audunit;
                }
                else
                {
                    // there are still active streams so don't delete
                }
            }
            else
            {
                audunit = streamer->audunit;
            }

            if (audunit)
            {
                // close streamer's audio unit instance
                OSStatus status;
                status = AudioOutputUnitStop(audunit);
                assert(status == noErr);
                status = AudioUnitUninitialize(audunit);
                assert(status == noErr);
                status = AudioComponentInstanceDispose(audunit);
                assert(status == noErr);
            }
        }

        bool StartStream(outputstreamer_t streamer)
        {
            streamer->playing = true;
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

        outputstreamer_t GetMuxedStreamer(const outputstreamer_t& streamer)
        {
            // assumes 'm_mux_lock' has already been acquired.
            muxed_streamers_t::const_iterator i = m_muxed_streamers.begin();
            for(;i!=m_muxed_streamers.end();++i)
            {
                if (streamer->sndgrpid == (*i)->sndgrpid &&
                    streamer->samplerate == (*i)->samplerate &&
                    streamer->framesize == (*i)->framesize &&
                    streamer->channels == (*i)->channels)
                {
                    return *i;
                }
            }
            return outputstreamer_t();
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

static OSStatus AudioMuxedCallback(void *userData, AudioUnitRenderActionFlags *actionFlags,
                                   const AudioTimeStamp *audioTimeStamp, UInt32 busNumber,
                                   UInt32 numFrames, AudioBufferList *buffers)
{
    AUOutputStreamer* streamer = reinterpret_cast<AUOutputStreamer*>(userData);

    std::lock_guard<std::recursive_mutex> g(streamer->streamers_mtx);
    if(streamer->streamers.empty())
        return noErr;

    return AudioOutputCallback(userData, actionFlags, audioTimeStamp, busNumber, numFrames, buffers);
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
        if (streamer->streamers.size()) // process muxed playback
        {
            assert(streamer->player == NULL);
            memset(samples_buffer, 0, cbbytes);
            std::vector<short> tmp_buffer(streamer->channels * streamer->framesize);
            std::vector<OutputStreamer*> streamers;
            for(auto i=streamer->streamers.begin();
                i!=streamer->streamers.end();++i)
            {
                if ((*i)->playing)
                    streamers.push_back((*i).get());
            }
            MuxPlayers(soundsystem::getAudUnit().get(), streamers, &tmp_buffer[0], samples_buffer);
        }
        else // process 'normal' playback
        {
            assert(streamer->player);
            streamer->player->StreamPlayerCb(*streamer, &samples_buffer[0], streamer->framesize);
            //soft volume also handles mute
            SoftVolume(soundsystem::getAudUnit().get(), *streamer, &samples_buffer[0], streamer->framesize);
        }

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

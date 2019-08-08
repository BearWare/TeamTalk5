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

#include "PortAudioWrapper.h"
#include <myace/MyACE.h>
#include <iostream>
#include <math.h>
#include <assert.h>

#if defined(ACE_WIN32)
#include <px_win_ds.h>    //the directx mixer
#include <Objbase.h>
#endif

using namespace std;
namespace soundsystem {

PortAudio::PortAudio()
{
    Init();
}

PortAudio::~PortAudio()
{
    Close();
    MYTRACE(ACE_TEXT("~PortAudio()\n"));
}

bool PortAudio::Init()
{
    PaError err = Pa_Initialize();
    assert(err == paNoError);
    RefreshDevices();
    return err == paNoError;
}

void PortAudio::Close()
{
    PaError err = Pa_Terminate();
    assert(err == paNoError);
}

PortAudio* PortAudio::getInstance()
{
    static soundsystem::PortAudio p;

#if defined(ACE_WIN32) //COM must be initialize for all threads which uses this class
    static ACE_Recursive_Thread_Mutex mtx;
    wguard_t g(mtx);

    static set<ACE_thread_t> com_threads;
    if(com_threads.find(ACE_Thread::self()) == com_threads.end())
    {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        com_threads.insert(ACE_Thread::self());
    }
#endif

    return &p;
}

soundgroup_t PortAudio::NewSoundGroup()
{
    return soundgroup_t(new PaSoundGroup());
}

void PortAudio::RemoveSoundGroup(soundgroup_t sndgrp)
{
}

bool PortAudio::SetAutoPositioning(int sndgrpid, bool enable)
{
    soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
    if (sndgrp)
    {
        sndgrp->autoposition = enable;
        return true;
    }
    return false;
}

bool PortAudio::IsAutoPositioning(int sndgrpid)
{
    soundgroup_t sndgrp = GetSoundGroup(sndgrpid);
    if(sndgrp)
        return sndgrp->autoposition;
    return false;
}

bool PortAudio::AutoPositionPlayers(int sndgrpid, bool all_players)
{
    assert(IsAutoPositioning(sndgrpid));
    if(IsAutoPositioning(sndgrpid))
    {
        std::vector<StreamPlayer*> players = GetPlayers(sndgrpid);
        for(size_t i=0;i<players.size();)
        {
            outputstreamer_t streamer = GetStream(players[i]);
            if(!streamer || (!streamer->autoposition && !all_players))
                players.erase(players.begin()+i);
            else ++i;
        }

        int count = (int)players.size()+1;

        for(int i=0; i<count-1; i++)
        {
            float x = cos( (float(i+1))/(float)count * 3.1415f);
            float y = sin((float(i+1))/(float)count * 3.1415f);
            SetPosition(players[i], x, y, 0.0f);
        }
        return true;
    }
    return false;
}

bool PortAudio::GetDefaultDevices(int& inputdeviceid, int& outputdeviceid)
{
    inputdeviceid = Pa_GetDefaultInputDevice();
    outputdeviceid = Pa_GetDefaultOutputDevice();
#if defined(WIN32)
    PaHostApiIndex hostApi;
    hostApi = Pa_HostApiTypeIdToHostApiIndex(paWASAPI);
    if(hostApi != paHostApiNotFound)
    {
        const PaHostApiInfo* hostapi = Pa_GetHostApiInfo(hostApi);
        if(hostapi)
        {
            inputdeviceid = hostapi->defaultInputDevice;
            outputdeviceid = hostapi->defaultOutputDevice;
        }
    }
#endif
    return inputdeviceid != paNoDevice && outputdeviceid != paNoDevice;
}

bool PortAudio::GetDefaultDevices(SoundAPI sndsys, int& inputdeviceid,
                                  int& outputdeviceid)
{
    const PaHostApiInfo* hostapi = NULL;
    PaHostApiIndex hostapiIndex = paHostApiNotFound;
    switch(sndsys)
    {
    case SOUND_API_DSOUND :
        hostapiIndex = Pa_HostApiTypeIdToHostApiIndex(paDirectSound);
        break;
    case SOUND_API_WINMM :
        hostapiIndex = Pa_HostApiTypeIdToHostApiIndex(paMME);
        break;
    case SOUND_API_ALSA :
        hostapiIndex = Pa_HostApiTypeIdToHostApiIndex(paALSA);
        break;
    case SOUND_API_COREAUDIO :
        hostapiIndex = Pa_HostApiTypeIdToHostApiIndex(paCoreAudio);
        break;
    case SOUND_API_WASAPI :
        hostapiIndex = Pa_HostApiTypeIdToHostApiIndex(paWASAPI);
        break;
    case SOUND_API_WDMKS :
        hostapiIndex = Pa_HostApiTypeIdToHostApiIndex(paWDMKS);
        break;
    case SOUND_API_OPENSLES_ANDROID :
    case SOUND_API_AUDIOUNIT :
    case SOUND_API_AUDIOTOOLKIT :
    case SOUND_API_NOSOUND :
        assert(0);
        return false;
    }
    if(hostapiIndex == paHostApiNotFound)
        return false;

    hostapi = Pa_GetHostApiInfo(hostapiIndex);
    if(hostapi)
    {
        inputdeviceid = hostapi->defaultInputDevice;
        outputdeviceid = hostapi->defaultOutputDevice;
        return true;
    }
    return false;
}

void PortAudio::FillDevices(sounddevices_t& sounddevs)
{
    int n_devices = Pa_GetDeviceCount();
    for(int i=0;i<n_devices; i++)
    {
        const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(i);
        assert(devinfo);
        if(!devinfo)
            continue;

        DeviceInfo device;
        device.devicename = devinfo->name;
#if defined(WIN32)
        if(devinfo->uniqueID)
            device.deviceid = devinfo->uniqueID;
#endif
        device.soundsystem = GetSoundSystem(devinfo);
        device.id = i;
        device.max_input_channels = devinfo->maxInputChannels;
        device.max_output_channels = devinfo->maxOutputChannels;
        device.default_samplerate = (int)devinfo->defaultSampleRate;

        PaStreamParameters streamParameters = {};
        streamParameters.device = i;
        streamParameters.sampleFormat = paInt16;
        streamParameters.suggestedLatency = 0;

        for(size_t j=0;j<standardSampleRates.size();j++)
        {
            //check input sample rates
            streamParameters.channelCount = devinfo->maxInputChannels;
            if(Pa_IsFormatSupported(&streamParameters, NULL, (double)standardSampleRates[j]) ==
               paFormatIsSupported)
                device.input_samplerates.insert((int)standardSampleRates[j]);

            //check output sample rates
            streamParameters.channelCount = devinfo->maxOutputChannels;
            if(Pa_IsFormatSupported(NULL, &streamParameters, (double)standardSampleRates[j]) ==
               paFormatIsSupported)
                device.output_samplerates.insert((int)standardSampleRates[j]);
        }

        for(int c=1;c<=devinfo->maxInputChannels;c++)
        {
            //check input channels
            streamParameters.channelCount = c;
            if(Pa_IsFormatSupported(&streamParameters, NULL, device.default_samplerate) ==
               paFormatIsSupported)
                device.input_channels.insert(c);
        }

        for(int c=1;c<=devinfo->maxOutputChannels;c++)
        {
            //check output channels
            streamParameters.channelCount = c;
            if(Pa_IsFormatSupported(NULL, &streamParameters, device.default_samplerate) ==
               paFormatIsSupported)
                device.output_channels.insert(c);
        }

#if defined(WIN32)
        device.wavedeviceid = devinfo->wavedeviceid;
        device.supports3d = devinfo->max3dBuffers > 0;
#else
        device.supports3d = false;
#endif
        sounddevs[device.id] = device;
    }
}

SoundAPI PortAudio::GetSoundSystem(const PaDeviceInfo* devinfo)
{
    if(devinfo->hostApi == Pa_HostApiTypeIdToHostApiIndex(paDirectSound))
        return SOUND_API_DSOUND;
    else if(devinfo->hostApi == Pa_HostApiTypeIdToHostApiIndex(paMME))
        return SOUND_API_WINMM;
    else if(devinfo->hostApi == Pa_HostApiTypeIdToHostApiIndex(paALSA))
        return SOUND_API_ALSA;
    else if(devinfo->hostApi == Pa_HostApiTypeIdToHostApiIndex(paCoreAudio))
        return SOUND_API_COREAUDIO;
    else if(devinfo->hostApi == Pa_HostApiTypeIdToHostApiIndex(paWASAPI))
        return SOUND_API_WASAPI;
    else if(devinfo->hostApi == Pa_HostApiTypeIdToHostApiIndex(paWDMKS))
        return SOUND_API_WDMKS;

    return SOUND_API_NOSOUND;
}

int InputStreamCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData)
{
    const short* recorded = static_cast<const short*> (inputBuffer);

    MYTRACE_COND(statusFlags & paInputUnderflow, ACE_TEXT("PORTAUDIO: paInputUnderFlow\n"));
    MYTRACE_COND(statusFlags & paInputOverflow, ACE_TEXT("PORTAUDIO: paInputOverflow\n"));

    PaInputStreamer* inputStreamer = static_cast<PaInputStreamer*> (userData);
    assert(inputStreamer);
    inputStreamer->recorder->StreamCaptureCb(*inputStreamer,
                                             recorded, framesPerBuffer);
    MYTRACE_COND(inputStreamer->soundsystem == SOUND_API_NOSOUND,
                 ACE_TEXT("No sound input callback"));
    return paContinue;
}

inputstreamer_t PortAudio::NewStream(StreamCapture* capture, int inputdeviceid, 
                                     int sndgrpid, int samplerate, int channels,
                                     int framesize)
{
    const PaDeviceInfo* indev = Pa_GetDeviceInfo(inputdeviceid);
    if(!indev)
        return inputstreamer_t();

    inputstreamer_t streamer(new PaInputStreamer(capture, sndgrpid, framesize,
                                                 samplerate, channels,
                                                 GetSoundSystem(indev), inputdeviceid));
#if defined(DEBUG)
    streamer->duplex = false;
#endif

    PaStreamParameters inputParameters;
    inputParameters.device = inputdeviceid;
    inputParameters.channelCount = channels;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = indev->defaultLowInputLatency;

    PaError err = Pa_OpenStream(&streamer->stream, &inputParameters, NULL,
                                (double)samplerate, framesize, paClipOff, 
                                InputStreamCallback,
                                static_cast<void*> (streamer.get()) );
    MYTRACE_COND(err != paNoError, ACE_TEXT("Failed to initialize input device %d\n"), inputdeviceid);
    if(err != paNoError)
    {
        return inputstreamer_t();
    }

    return streamer;
}

bool PortAudio::StartStream(inputstreamer_t streamer)
{
    assert(streamer->stream);
    PaError err = Pa_StartStream(streamer->stream);
    assert(err == paNoError);
    return err == paNoError;
}

void PortAudio::CloseStream(inputstreamer_t streamer)
{
#if defined(DEBUG)
    assert(!streamer->duplex);
#endif

    PaStream* stream = streamer->stream;
    assert(stream);        

    PaError err = Pa_StopStream(stream);
    assert(err == paNoError);

    err = Pa_CloseStream(stream);
    assert(err == paNoError);
}

int OutputStreamCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    assert(userData);
    PaOutputStreamer* streamer = static_cast<PaOutputStreamer*> (userData);

    MYTRACE_COND(statusFlags & paOutputUnderflow, ACE_TEXT("PORTAUDIO: paOutputUnderflow\n"));
    MYTRACE_COND(statusFlags & paOutputOverflow, ACE_TEXT("PORTAUDIO: paOutputOverflow\n"));

    bool bContinue = false;
    short* playback = static_cast<short*>(outputBuffer);

    bContinue = streamer->player->StreamPlayerCb(*streamer, playback, framesPerBuffer);
    SoftVolume(*streamer, playback, framesPerBuffer);

    MYTRACE_COND(streamer->soundsystem == SOUND_API_NOSOUND,
                 ACE_TEXT("No sound output callback"));

    if(bContinue)
        return paContinue;
    else
        return paComplete;
}

void OutputStreamCallbackEnded(void* userData)
{
    assert(userData);
    PaOutputStreamer* streamer = static_cast<PaOutputStreamer*> (userData);
    streamer->player->StreamPlayerCbEnded();
}


outputstreamer_t PortAudio::NewStream(StreamPlayer* player, int outputdeviceid,
                                      int sndgrpid, int samplerate, int channels,
                                      int framesize)
{
    PaStreamParameters outputParameters = {};
    outputParameters.device = outputdeviceid;
    outputParameters.channelCount = channels;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paInt16;
    const PaDeviceInfo* outdev = Pa_GetDeviceInfo( outputParameters.device );
    if(!outdev)
        return outputstreamer_t();
        
    outputParameters.suggestedLatency = outdev->defaultLowOutputLatency;

    //create stream holder
    outputstreamer_t streamer(new PaOutputStreamer(player, sndgrpid, framesize, samplerate,
                                                   channels, GetSoundSystem(outdev),
                                                   outputdeviceid));

    PaError err = Pa_OpenStream(&streamer->stream,
                                NULL,
                                &outputParameters,
                                (double)samplerate,
                                framesize,
                                paClipOff,
                                OutputStreamCallback,
                                static_cast<void*> (streamer.get()) );

    MYTRACE_COND(err != paNoError, ACE_TEXT("Failed to initialize output device %d\n"), outputdeviceid);
    if(err != paNoError)
    {
        return outputstreamer_t();
    }

    err = Pa_SetStreamFinishedCallback(streamer->stream, OutputStreamCallbackEnded);
    assert(err == paNoError);

    if(err != paNoError)
    {
        return outputstreamer_t();
    }

    //set master volume so it's relative to master volume
    SetVolume(player, VOLUME_DEFAULT);

    return streamer;
}

void PortAudio::CloseStream(outputstreamer_t streamer)
{
    assert(streamer->stream);
#if defined(DEBUG)
    assert(!streamer->duplex);
#endif
    PaStream* paStream = streamer->stream;

    PaError err = paNoError;
    //stop stream so we can unregister callback
    if(Pa_IsStreamStopped(paStream) == 0)
        err = Pa_AbortStream(paStream);
    assert(err == paNoError);

    //close the stream
    err = Pa_CloseStream(paStream);
    assert(err == paNoError);

    MYTRACE_COND(err != paNoError, ACE_TEXT("PORTAUDIO: Failed to close stream\n"));
}

bool PortAudio::StartStream(outputstreamer_t streamer)
{
    PaError err = Pa_StartStream(streamer->stream);
    assert(err == paNoError);
    return err == paNoError;
}

bool PortAudio::StopStream(outputstreamer_t streamer)
{
    PaStream* paStream = streamer->stream;
    assert(paStream);

    PaError err = Pa_StopStream(paStream);
    MYTRACE_COND(err != paNoError, ACE_TEXT("PORTAUDIO: Error stopping stream: %d\n"), err);
    return err == paNoError;
}

bool PortAudio::IsStreamStopped(outputstreamer_t streamer)
{
    return Pa_IsStreamStopped(streamer->stream)>0;
}

void PortAudio::SetSampleRate(StreamPlayer* player, int samplerate)
{
    outputstreamer_t streamer = GetStream(player);
    if (!streamer)
        return;

#if defined(WIN32)
    if(streamer->soundsystem == SOUND_API_DSOUND && streamer->stream)
        Px_DsSetFrequency(streamer->stream, (PxFrequency)samplerate);
#endif
}

int PortAudio::GetSampleRate(StreamPlayer* player)
{
    outputstreamer_t streamer = GetStream(player);
    if (!streamer)
        return 0;

#if defined(WIN32)
    if(streamer->soundsystem == SOUND_API_DSOUND && streamer->stream)
        return (int)Px_DsGetFrequency(streamer->stream);
#endif
    return 0;
}
void PortAudio::SetAutoPositioning(StreamPlayer* player, bool enable)
{
    outputstreamer_t streamer = GetStream(player);
    if (!streamer)
        return;
    streamer->autoposition = enable;
}

bool PortAudio::IsAutoPositioning(StreamPlayer* player)
{
    outputstreamer_t streamer = GetStream(player);
    if (!streamer)
        return false;

    return streamer->autoposition;
}

bool PortAudio::SetPosition(StreamPlayer* player, float x, float y, float z)
{
    outputstreamer_t streamer = GetStream(player);
    if (!streamer)
        return false;

#if defined(WIN32)
    if(streamer->soundsystem == SOUND_API_DSOUND && 
       streamer->channels == 1 &&
       streamer->stream)//only supported in mono
    {
        Px_DsSetPosition(streamer->stream, x, y, z);
        return true;
    }
#endif

    return false;
}

bool PortAudio::GetPosition(StreamPlayer* player, float& x, float& y, float& z)
{
    outputstreamer_t streamer = GetStream(player);
    if (!streamer)
        return false;

#if defined(WIN32)
    if(streamer->soundsystem == SOUND_API_DSOUND &&
       streamer->channels == 1 &&
       streamer->stream)//only supported in mono
    {
        Px_DsGetPosition(streamer->stream, &x, &y, &z);
        return true;
    }
#endif
    return false;
}

int DuplexStreamCallback(const void *inputBuffer, 
                         void *outputBuffer, unsigned long framesPerBuffer, 
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags, void *userData)
{
    MYTRACE_COND(statusFlags & paOutputUnderflow, ACE_TEXT("PORTAUDIO: paOutputUnderflow\n"));
    MYTRACE_COND(statusFlags & paOutputOverflow, ACE_TEXT("PORTAUDIO: paOutputOverflow\n"));

    MYTRACE_COND(statusFlags & paInputUnderflow, ACE_TEXT("PORTAUDIO: paInputUnderFlow\n"));
    MYTRACE_COND(statusFlags & paInputOverflow, ACE_TEXT("PORTAUDIO: paInputOverflow\n"));

    PaDuplexStreamer* dpxStream = static_cast<PaDuplexStreamer*>(userData);
    const short* recorded = reinterpret_cast<const short*>(inputBuffer);
    short* playback = reinterpret_cast<short*>(outputBuffer);

    DuplexCallback(*dpxStream, recorded, playback);

    return paContinue;
}

void DuplexStreamCallbackEnded(void* userData)
{
    PaDuplexStreamer* dpxStream = static_cast<PaDuplexStreamer*>(userData);
    MYTRACE(ACE_TEXT("PORTAUDIO: Duplex stream %p ended\n"), dpxStream);
    DuplexEnded(*dpxStream);
}

duplexstreamer_t PortAudio::NewStream(StreamDuplex* duplex, int inputdeviceid,
                                      int outputdeviceid, int sndgrpid,
                                      int samplerate, int input_channels, 
                                      int output_channels, int framesize)
{
    //input device init
    PaStreamParameters inputParameters; 
    inputParameters.device = inputdeviceid;
    const PaDeviceInfo* indev = Pa_GetDeviceInfo( inputParameters.device );
    if( !indev )
        return duplexstreamer_t();
    inputParameters.channelCount = input_channels;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = indev->defaultLowInputLatency;

    //output device init
    PaStreamParameters outputParameters;
    outputParameters.device = outputdeviceid;
    const PaDeviceInfo* outdev = Pa_GetDeviceInfo( outputParameters.device );
    if(!outdev)
        return duplexstreamer_t();
    outputParameters.channelCount = output_channels;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = outdev->defaultLowOutputLatency;

    duplexstreamer_t streamer(new PaDuplexStreamer(duplex, sndgrpid, framesize,
                                                   samplerate, input_channels, 
                                                   output_channels, GetSoundSystem(outdev),
                                                   inputdeviceid, outputdeviceid));

    //open stream
    PaError err = Pa_OpenStream(&streamer->stream, &inputParameters,
                                &outputParameters, (double)samplerate, 
                                framesize, paClipOff, 
                                DuplexStreamCallback,
                                static_cast<void*> (streamer.get()) );
    MYTRACE_COND(err != paNoError,
                 ACE_TEXT("Failed to initialize duplex devices, input %d, output %d\n"),
                 inputdeviceid, outputdeviceid);
    if(err != paNoError)
    {
        return duplexstreamer_t();
    }

    err = Pa_SetStreamFinishedCallback(streamer->stream, DuplexStreamCallbackEnded);
    assert(err == paNoError);
    return streamer;
}

bool PortAudio::StartStream(duplexstreamer_t streamer)
{
    assert(streamer->stream);
    PaError err = Pa_StartStream(streamer->stream);
    assert(err == paNoError);
    return err == paNoError;
}

void PortAudio::CloseStream(duplexstreamer_t streamer)
{
    assert(streamer->players.empty());

    PaStream* stream = streamer->stream;
    PaError err = Pa_CloseStream(streamer->stream);
    assert(err == paNoError);
}

} //namespace

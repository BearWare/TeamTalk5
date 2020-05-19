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

#include <avstream/DMOResampler.h> // need SetWaveMediaType()

#include <px_win_ds.h>    //the directx mixer

#include <Objbase.h>
#include <Mmsystem.h>
#include <propsys.h>
#include <atlbase.h>
#include <MMDeviceApi.h>
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

std::shared_ptr<PortAudio> PortAudio::getInstance()
{
    static std::shared_ptr<PortAudio> p(new PortAudio());

#if defined(ACE_WIN32) //COM must be initialize for all threads which uses this class
    static std::mutex mtx;
    std::lock_guard<std::mutex> g(mtx);

    static set<ACE_thread_t> com_threads;
    if(com_threads.find(ACE_Thread::self()) == com_threads.end())
    {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        com_threads.insert(ACE_Thread::self());
    }
#endif

    return p;
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
    return inputdeviceid != paNoDevice || outputdeviceid != paNoDevice;
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

        device.features |= SOUNDDEVICEFEATURE_DUPLEXMODE;

#if defined(WIN32)
        device.wavedeviceid = devinfo->wavedeviceid;
        if (devinfo->max3dBuffers > 0)
            device.features |= SOUNDDEVICEFEATURE_3DPOSITION;

        // CWMAudioAECCapture
        if (device.soundsystem == SOUND_API_WASAPI && device.input_channels.size())
        {
            device.features |= SOUNDDEVICEFEATURE_AEC;
            device.features |= SOUNDDEVICEFEATURE_AGC;
            device.features |= SOUNDDEVICEFEATURE_DENOISE;
        }
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

    streamer->stream = nullptr;
}

bool PortAudio::IsStreamStopped(inputstreamer_t streamer)
{
    assert(streamer->stream);
    return Pa_IsStreamStopped(streamer->stream) > 0;
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
    SoftVolume(PortAudio::getInstance().get(), *streamer, playback, framesPerBuffer);

    MYTRACE_COND(streamer->soundsystem == SOUND_API_NOSOUND,
                 ACE_TEXT("No sound output callback"));

    if(bContinue)
        return paContinue;
    else
        return paComplete;
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

    //set master volume so it's relative to master volume
    SetVolume(player, VOLUME_DEFAULT);

    MYTRACE(ACE_TEXT("Created PortAudio output stream %p\n"), streamer->stream);

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

    MYTRACE(ACE_TEXT("Closing PortAudio output stream %p\n"), streamer->stream);
    //close the stream
    err = Pa_CloseStream(paStream);
    assert(err == paNoError);
    MYTRACE(ACE_TEXT("Closed PortAudio output stream %p\n"), streamer->stream);
    streamer->stream = nullptr;
    MYTRACE_COND(err != paNoError, ACE_TEXT("PORTAUDIO: Failed to close stream\n"));
}

bool PortAudio::StartStream(outputstreamer_t streamer)
{
    assert(streamer->stream);
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

    assert(framesPerBuffer == dpxStream->framesize);

    /*
    static uint32_t cbsamples = 0, tick = GETTIMESTAMP();
    cbsamples += framesPerBuffer;
    auto now = GETTIMESTAMP();
    auto total = now - tick;
    auto duration = PCM16_SAMPLES_DURATION(cbsamples, dpxStream->samplerate);
    MYTRACE(ACE_TEXT("Samples duration: %u msec. Total: %u msec. Diff: %d msec\n"), duration, total, total - duration);
    */

    if (dpxStream->initialcallback)
    {
        // allow Pa_OpenStream() to return
        dpxStream->initialcallback = false;
        dpxStream->starttime = GETTIMESTAMP();
        return paContinue;
    }

    // check if duplex callback is over or underflowing
    uint32_t cbMSec = PCM16_SAMPLES_DURATION(framesPerBuffer, dpxStream->samplerate);
    uint32_t durationMSec = GETTIMESTAMP() - dpxStream->starttime;
    dpxStream->playedsamples_msec += cbMSec;
    MYTRACE_COND(std::abs(int(durationMSec - dpxStream->playedsamples_msec)) > cbMSec * 3,
                 ACE_TEXT("Duplex callback is off my %d msec\n"), durationMSec - dpxStream->playedsamples_msec);

#if defined(WIN32)
    if (dpxStream->winaec)
    {
        recorded = dpxStream->winaec->AcquireBuffer();
        MYTRACE_COND(!recorded, ACE_TEXT("No echo cancelled audio available\n"));

        if (recorded)
        {
            DuplexCallback(PortAudio::getInstance().get(), *dpxStream, recorded, playback);
            dpxStream->winaec->ReleaseBuffer();
        }
        else
        {
            std::vector<short> tmpbuf(dpxStream->framesize * dpxStream->input_channels);
            recorded = &tmpbuf[0];
            DuplexCallback(PortAudio::getInstance().get(), *dpxStream, recorded, playback);
        }

        auto echodiff_msec = dpxStream->playedsamples_msec - dpxStream->echosamples_msec;
        MYTRACE_COND(std::abs(int(echodiff_msec)) > cbMSec * 3,
                     ACE_TEXT("Duplex callback shows player is off by %d msec compared to echo cancellor\n"),
                     echodiff_msec);
    }
    else
    {
        DuplexCallback(PortAudio::getInstance().get(), *dpxStream, recorded, playback);
    }
#else
    DuplexCallback(PortAudio::getInstance().get(), *dpxStream, recorded, playback);
#endif
    return paContinue;
}

void DuplexStreamCallbackEnded(void* userData)
{
    PaDuplexStreamer* dpxStream = static_cast<PaDuplexStreamer*>(userData);
    MYTRACE(ACE_TEXT("PORTAUDIO: Duplex stream %p ended\n"), dpxStream);
    DuplexEnded(PortAudio::getInstance().get(), *dpxStream);
}

duplexstreamer_t PortAudio::NewStream(StreamDuplex* duplex, int inputdeviceid,
                                      int outputdeviceid, int sndgrpid,
                                      int samplerate, int input_channels,
                                      int output_channels, int framesize)
{
    const PaDeviceInfo* indev = Pa_GetDeviceInfo(inputdeviceid);
    const PaDeviceInfo* outdev = Pa_GetDeviceInfo(outputdeviceid);
    if (!indev || !outdev)
        return duplexstreamer_t();

    //input device init
    PaStreamParameters inputParameters;
    inputParameters.device = inputdeviceid;
    inputParameters.channelCount = input_channels;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = indev->defaultLowInputLatency;
    PaStreamParameters* tmpInputParameters = &inputParameters;

    //output device init
    PaStreamParameters outputParameters;
    outputParameters.device = outputdeviceid;
    outputParameters.channelCount = output_channels;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = outdev->defaultLowOutputLatency;

    duplexstreamer_t streamer(new PaDuplexStreamer(duplex, sndgrpid, framesize,
                                                   samplerate, input_channels,
                                                   output_channels, GetSoundSystem(outdev),
                                                   inputdeviceid, outputdeviceid));

#if defined(WIN32)
    // echo cancel only applies to WASAPI
    if (duplex->GetDuplexFeatures() & (SOUNDDEVICEFEATURE_AEC | SOUNDDEVICEFEATURE_AGC | SOUNDDEVICEFEATURE_DENOISE))
    {
        DeviceInfo outdev;
        if (!GetDevice(outputdeviceid, outdev))
            return duplexstreamer_t();
        assert(samplerate == outdev.default_samplerate);

        streamer->winaec.reset(new CWMAudioAECCapture(streamer.get(), duplex->GetDuplexFeatures()));
        if (!streamer->winaec->Open())
            return duplexstreamer_t();
        tmpInputParameters = nullptr;
    }
#endif

    //open stream
    PaError err = Pa_OpenStream(&streamer->stream, (tmpInputParameters ? tmpInputParameters : NULL),
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

    PaError err = Pa_CloseStream(streamer->stream);
    assert(err == paNoError);

    streamer->stream = nullptr;
}

bool PortAudio::IsStreamStopped(duplexstreamer_t streamer)
{
    assert(streamer->stream);
    return Pa_IsStreamStopped(streamer->stream) > 0;
}

bool PortAudio::UpdateStreamDuplexFeatures(duplexstreamer_t streamer)
{
    assert(streamer);
    SoundDeviceFeatures newfeatures = streamer->duplex->GetDuplexFeatures();

#if defined(WIN32)
    if (streamer->winaec)
        return streamer->winaec->GetFeatures() == newfeatures;
#endif
    return newfeatures == SOUNDDEVICEFEATURE_NONE;
}

#if defined(WIN32)
CWMAudioAECCapture::CWMAudioAECCapture(PaDuplexStreamer* duplex, SoundDeviceFeatures features)
: m_streamer(duplex)
, m_features(features)
{
}

CWMAudioAECCapture::~CWMAudioAECCapture()
{
    if (m_callback_thread)
    {
        m_stop.set_value(true);
        m_callback_thread->join();
    }
}

bool CWMAudioAECCapture::Open()
{
    auto result = m_started.get_future();
    m_callback_thread.reset(new std::thread(&CWMAudioAECCapture::Run, this));

    return result.get();
}

void CWMAudioAECCapture::Run()
{
    // CoInitialize(NULL); let PortAudio::getInstance() initialize COM

    // find MFPKEY_WMAAECMA_DEVICE_INDEXES
    LONG inDevIndex, outDevIndex;
    if (!FindDevs(inDevIndex, outDevIndex))
        return;

    CComPtr<IMediaObject> pDMO;
    CComPtr<IPropertyStore> pPS;

    if (FAILED(CoCreateInstance(CLSID_CWMAudioAEC, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (LPVOID*)&pDMO)))
    {
        m_started.set_value(false);
        return;
    }

    if (FAILED(pDMO->QueryInterface(IID_IPropertyStore, (LPVOID*)&pPS)))
    {
        m_started.set_value(false);
        return;
    }

    const int SAMPLERATE = 22050;
    const int CHANNELS = 1;
    media::AudioFormat infmt(SAMPLERATE, CHANNELS);
    const size_t BUFSIZE = PCM16_BYTES(SAMPLERATE * 3, CHANNELS);

    m_input_queue.high_water_mark(BUFSIZE);
    m_input_queue.low_water_mark(BUFSIZE);

    int inputsamples = CalcSamples(m_streamer->samplerate, m_streamer->framesize, SAMPLERATE);
    m_input_buffer.resize(inputsamples * CHANNELS);

    m_resampler = MakeAudioResampler(infmt, media::AudioFormat(m_streamer->samplerate, m_streamer->input_channels), inputsamples);
    if (!m_resampler)
    {
        m_started.set_value(false);
        return;
    }

    PROPVARIANT pvSysMode;
    PropVariantInit(&pvSysMode);
    pvSysMode.vt = VT_I4;
    pvSysMode.lVal = (m_features & SOUNDDEVICEFEATURE_AEC) ? SINGLE_CHANNEL_AEC : SINGLE_CHANNEL_NSAGC;
    if (FAILED(pPS->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode)))
    {
        m_started.set_value(false);
        return;
    }
    PropVariantClear(&pvSysMode);

    PROPVARIANT pvDeviceId;
    PropVariantInit(&pvDeviceId);
    pvDeviceId.vt = VT_I4;
    pvDeviceId.lVal = (outDevIndex << 16) | inDevIndex;
    if (FAILED(pPS->SetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, pvDeviceId)))
    {
        m_started.set_value(false);
        return;
    }
    PropVariantClear(&pvDeviceId);

    PROPVARIANT pvFeatrModeOn;
    PropVariantInit(&pvFeatrModeOn);
    pvFeatrModeOn.vt = VT_BOOL;
    pvFeatrModeOn.boolVal = VARIANT_TRUE;
    if (FAILED(pPS->SetValue(MFPKEY_WMAAECMA_FEATURE_MODE, pvFeatrModeOn)))
    {
        m_started.set_value(false);
        return;
    }
    PropVariantClear(&pvFeatrModeOn);

    // Toggle AGC
    if (m_features & SOUNDDEVICEFEATURE_AGC)
    {
        PROPVARIANT pvAGC;
        PropVariantInit(&pvAGC);
        pvAGC.vt = VT_BOOL;
        pvAGC.boolVal = VARIANT_TRUE;
        if (FAILED(pPS->SetValue(MFPKEY_WMAAECMA_FEATR_AGC, pvAGC)))
        {
            m_started.set_value(false);
            return;
        }
        PropVariantClear(&pvAGC);
    }

    if (m_features & SOUNDDEVICEFEATURE_DENOISE)
    {
        // Turn on/off noise suppression
        PROPVARIANT pvNoiseSup;
        PropVariantInit(&pvNoiseSup);
        pvNoiseSup.vt = VT_I4;
        pvNoiseSup.lVal = 1;
        if (FAILED(pPS->SetValue(MFPKEY_WMAAECMA_FEATR_NS, pvNoiseSup)))
        {
            m_started.set_value(false);
            return;
        }
        PropVariantClear(&pvNoiseSup);
    }

    DMO_MEDIA_TYPE mt = {};
    if (FAILED(MoInitMediaType(&mt, sizeof(WAVEFORMATEX))))
    {
        m_started.set_value(false);
        return;
    }

    if (!SetWaveMediaType(SAMPLEFORMAT_INT16, CHANNELS, SAMPLERATE, mt))
    {
        m_started.set_value(false);
        return;
    }

    if (FAILED(pDMO->SetOutputType(0, &mt, 0)))
    {
        m_started.set_value(false);
        SUCCEEDED(MoFreeMediaType(&mt));
        return;
    }

    SUCCEEDED(MoFreeMediaType(&mt));

    if (FAILED(pDMO->AllocateStreamingResources()))
    {
        m_started.set_value(false);
        return;
    }

    auto stopstate = m_stop.get_future();

    m_started.set_value(true);

    int iFrameSize = 0;
    PROPVARIANT pvFrameSize;
    PropVariantInit(&pvFrameSize);
    SUCCEEDED(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_FRAME_SIZE, &pvFrameSize));
    iFrameSize = pvFrameSize.lVal;
    PropVariantClear(&pvFrameSize);

    std::vector<BYTE> outputbuf(PCM16_BYTES(SAMPLERATE, CHANNELS));
    auto delayMSec = std::chrono::milliseconds(iFrameSize * 1000 / SAMPLERATE);
    bool error = false;
    uint64_t processedBytes = 0;
    do
    {
        ProcessAudioQueue();

        CComPtr<IMediaBuffer> ioutputbuf;
        HRESULT hr = CMediaBuffer::CreateBuffer(&outputbuf[0], 0, outputbuf.size(), (void**)&ioutputbuf);
        MYTRACE_COND(FAILED(hr), ACE_TEXT("Failed to create AEC output buffer\n"));
        if (FAILED(hr))
            break;

        DMO_OUTPUT_DATA_BUFFER dmodatabuf = {};
        dmodatabuf.pBuffer = ioutputbuf;
        DWORD dwStatus, dwOutputLen = 0;
        hr = pDMO->ProcessOutput(0, 1, &dmodatabuf, &dwStatus);

        switch(hr)
        {
        case S_FALSE:
            dwOutputLen = 0;
            break;
        case S_OK:
            dwOutputLen = 0;
            BYTE* outputbufptr;
            if (SUCCEEDED(ioutputbuf->GetBufferAndLength(&outputbufptr, &dwOutputLen)))
            {
                //MYTRACE(ACE_TEXT("Audio callback with %d msec\n"), PCM16_BYTES_DURATION(dwOutputLen, CHANNELS, SAMPLERATE));
                int samples = dwOutputLen / sizeof(short) / CHANNELS;
                processedBytes += dwOutputLen;
                m_streamer->echosamples_msec = PCM16_BYTES_DURATION(processedBytes, CHANNELS, SAMPLERATE);

                media::AudioFrame frm(infmt, reinterpret_cast<short*>(outputbufptr), samples);
                // if we already have a queue then just add to queue
                if (m_input_queue.message_count())
                {
                    ACE_Message_Block* mb = AudioFrameToMsgBlock(frm);
                    ACE_Time_Value tv;
                    if (m_input_queue.enqueue_tail(mb, &tv) < 0)
                    {
                        mb->release();
                        MYTRACE(ACE_TEXT("Failed to queue echo cancelled audio\n"));
                    }
                }
                else
                {
                    QueueAudioInput(frm);
                }
            }

            if (stopstate.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                break; // stop requested
            else continue; // see if there's more
        case E_FAIL:
            error = true;
            break;
        case E_INVALIDARG:
            error = true;
            break;
        case E_POINTER:
            error = true;
            break;
        case WMAAECMA_E_NO_ACTIVE_RENDER_STREAM:
            MYTRACE(ACE_TEXT("No audio rendered on device: %d\n"), outDevIndex);
            error = true;
            break;
        default:
            MYTRACE(ACE_TEXT("Unknown HRESULT from echo cancellor 0x%x\n"), hr);
            error = true;
            break;
        }

    } while (!error && stopstate.wait_for(delayMSec) == std::future_status::timeout);
}

void CWMAudioAECCapture::ProcessAudioQueue()
{
    {
        // if queue is full then just give up
        std::lock_guard<std::mutex> g(m_mutex);
        if(m_input_index == m_input_buffer.size())
            return;
    }

    ACE_Time_Value tv;
    ACE_Message_Block* mb;
    while (m_input_queue.dequeue(mb, &tv) >= 0)
    {
        media::AudioFrame frm(mb);
        bool ret = QueueAudioInput(frm);
        mb->release();
        if (!ret)
            break;
    }
}

bool CWMAudioAECCapture::QueueAudioInput(const media::AudioFrame& frm)
{
    const int CHANNELS = frm.inputfmt.channels;
    int samples = frm.input_samples;
    int copiedsamples = 0;

    std::lock_guard<std::mutex> g(m_mutex);

    while (samples > 0 && m_input_index < m_input_buffer.size())
    {
        int remainsamples = (m_input_buffer.size() - m_input_index) / CHANNELS;
        int copysamples = std::min(samples, remainsamples);

        int copybytes = PCM16_BYTES(copysamples, CHANNELS);
        std::memcpy(&m_input_buffer[m_input_index * CHANNELS], &frm.input_buffer[copiedsamples * CHANNELS], copybytes);
        m_input_index += copysamples * CHANNELS;
        assert(m_input_index <= m_input_buffer.size());

        if (m_input_index == m_input_buffer.size())
        {
            int n_resampled;
            assert(m_resampled_input == nullptr);
            m_resampled_input = m_resampler->Resample(&m_input_buffer[0], &n_resampled);
            assert(n_resampled <= m_streamer->framesize);
        }

        samples -= copysamples;
        copiedsamples += copysamples;
        assert(samples >= 0);
        assert(copiedsamples <= frm.input_samples);
    }

    // if there's still more samples left we queue them
    if (samples)
    {
        media::AudioFrame tmp = frm;
        tmp.input_buffer = &frm.input_buffer[copiedsamples * CHANNELS];
        tmp.input_samples = samples;

        ACE_Message_Block* mb = AudioFrameToMsgBlock(tmp);
        ACE_Time_Value tv;
        if (m_input_queue.enqueue_head(mb, &tv) < 0)
        {
            mb->release();
            MYTRACE(ACE_TEXT("Failed to reenqueue echo cancelled audio\n"));
        }
    }

    return samples == 0;
}

short* CWMAudioAECCapture::AcquireBuffer()
{
     size_t bytes = m_input_queue.message_length();
     bytes -= sizeof(media::AudioFrame) * m_input_queue.message_count();
     int duration = PCM16_BYTES_DURATION(bytes, m_streamer->input_channels, m_streamer->samplerate);
     //MYTRACE(ACE_TEXT("Acquire echo audio, duration: %d msec. Full %d%%. Bytes %u\n"),
     //        duration, 100 * m_input_index / m_input_buffer.size(), unsigned(m_input_queue.message_length()));
     
     if (m_resampled_input)
        return m_resampled_input;

    ProcessAudioQueue();
 
    return m_resampled_input;
}

void CWMAudioAECCapture::ReleaseBuffer()
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_resampled_input = nullptr;
    m_input_index = 0;
}

bool CWMAudioAECCapture::FindDevs(LONG& indevindex, LONG& outdevindex)
{
    mapsndid_t indevs, outdevs;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;
    UINT dwCount = 0;

    // initialize COM
    auto sndsys = GetInstance();

    // Now enumerate capture devices
    if (FAILED(spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator))))
        return false;

    if (FAILED(spEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &spEndpoints)))
        return false;

    if (FAILED(spEndpoints->GetCount(&dwCount)))
        return false;

    for(UINT index = 0; index < dwCount; index++)
    {
        WCHAR* pszDeviceId = NULL;
        PROPVARIANT value;
        CComPtr<IMMDevice> spDevice;
        CComPtr<IPropertyStore> spProperties;

        PropVariantInit(&value);
        if (SUCCEEDED(spEndpoints->Item(index, &spDevice)) && SUCCEEDED(spDevice->GetId(&pszDeviceId)))
            indevs[pszDeviceId] = index;

        PropVariantClear(&value);
        CoTaskMemFree(pszDeviceId);
    }
    
    // reset for reuse
    spEndpoints.Release();
    dwCount = 0;

    // Now enumerate speaker devices
    if (FAILED(spEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &spEndpoints)))
        return false;

    if (FAILED(spEndpoints->GetCount(&dwCount)))
        return false;

    for(UINT index = 0; index < dwCount; index++)
    {
        WCHAR* pszDeviceId = NULL;
        PROPVARIANT value;
        CComPtr<IMMDevice> spDevice;
        CComPtr<IPropertyStore> spProperties;

        PropVariantInit(&value);
        if (SUCCEEDED(spEndpoints->Item(index, &spDevice)) && SUCCEEDED(spDevice->GetId(&pszDeviceId)))
            outdevs[pszDeviceId] = index;

        PropVariantClear(&value);
        CoTaskMemFree(pszDeviceId);
    }

    DeviceInfo indev, outdev;
    sndsys->GetDevice(m_streamer->inputdeviceid, indev);
    sndsys->GetDevice(m_streamer->outputdeviceid, outdev);

    if (indevs.find(indev.deviceid) == indevs.end())
        return false;

    if (outdevs.find(outdev.deviceid) == outdevs.end())
        return false;

    indevindex = indevs[indev.deviceid];
    outdevindex = outdevs[outdev.deviceid];
    return true;
}

#endif

} //namespace

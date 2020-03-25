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

#include "OpenSLESWrapper.h"

#include <myace/MyACE.h>
#include <assert.h>

namespace soundsystem {

SLuint32 toSLSamplerate(int samplerate);
SLuint32 toSLSpeaker(int channels);
int detectMinumumBuffer(SLAndroidSimpleBufferQueueItf bq, 
                        std::vector<short>& buffer, int samplerate,
                        int framesize, int channels);

enum AndroidSoundDevice
{
    DEFAULT_DEVICE_ID           = (0 & SOUND_DEVICEID_MASK),
};

#define DEFAULT_SAMPLERATE 16000

OpenSLESWrapper::OpenSLESWrapper()
    : m_engineObject(NULL)
    , m_engineEngine(NULL)
{
    Init();
}

OpenSLESWrapper::~OpenSLESWrapper()
{
    Close();
    MYTRACE(ACE_TEXT("OpenSLESWrapper()::~OpenSLESWrapper()\n"));
}

bool OpenSLESWrapper::Init()
{
    SLresult result;

    SLEngineOption engineOption[] = {
        {(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE}
    };

    SLInterfaceID ids[] = { SL_IID_ANDROIDEFFECTCAPABILITIES };
    SLboolean req[] = { SL_BOOLEAN_FALSE };
    const SLuint32 n_ids = sizeof(ids)/sizeof(ids[0]);

    // create engine
    result = slCreateEngine(&m_engineObject, 1, engineOption, n_ids, ids, req);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to create OpenSL ES engine object\n"));
    if (SL_RESULT_SUCCESS != result)
        return false;

    // realize the engine
    result = (*m_engineObject)->Realize(m_engineObject, SL_BOOLEAN_FALSE);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to realize OpenSL ES engine object\n"));
    if (SL_RESULT_SUCCESS != result)
        return false;

    // get the engine interface, which is needed in order to create other objects
    result = (*m_engineObject)->GetInterface(m_engineObject, SL_IID_ENGINE, &m_engineEngine);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to get OpenSL ES engine interface\n"));
    if (SL_RESULT_SUCCESS != result)
        return false;

    // reinitialize sound groups so they can use the new m_engineEngine
    std::vector<soundgroup_t> grps = GetSoundGroups();
    for (auto sndgrp : grps)
    {
        if (InitOutputMixObject(sndgrp) == nullptr)
            MYTRACE(ACE_TEXT("Failed to restore sound group\n"));
    }

    // go through effect capabilities interfaces
    SLAndroidEffectCapabilitiesItf effectLibItf;
    result = (*m_engineObject)->GetInterface(m_engineObject, SL_IID_ANDROIDEFFECTCAPABILITIES, &effectLibItf);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to get effect interface from engine\n"));
    if (SL_RESULT_SUCCESS == result)
    {
        SLresult result;
        SLuint32 nbEffects = 0;
        result = (*effectLibItf)->QueryNumEffects(effectLibItf, &nbEffects);
        assert(SL_RESULT_SUCCESS == result);

        SLInterfaceID effectType, effectImplementation;
        for (SLuint32 i = 0 ; i < nbEffects ; i++ ) {

            const int FX_NAME_LENGTH = 128;
            SLchar effectName[FX_NAME_LENGTH];
            SLuint16 effectNameLength = FX_NAME_LENGTH-1;

            result = (*effectLibItf)->QueryEffect(effectLibItf, i,
                                                  &effectType, &effectImplementation, 
                                                  effectName, &effectNameLength);
            effectName[effectNameLength] = 0;
            MYTRACE(ACE_TEXT("Android Effect Capability: #%d name=%s\n"), i, effectName);
        }
    }

/* Audio IO capabilities not supported by NDK    
    // go through audio device capabilities interface
    SLAudioIODeviceCapabilitiesItf audioioItf;
    result = (*m_engineObject)->GetInterface(m_engineObject, SL_IID_AUDIOIODEVICECAPABILITIES, &audioioItf);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to get audioIO capabilities interface from engine\n"));
    if (SL_RESULT_SUCCESS == result)
    {
        SLuint32 audioDevIds[3];
        SLint32 nbAudioDevs = 3;//sizeof(audioDevIds)/sizeof(audioDevIds[0]);
        result = (*audioioItf)->GetAvailableAudioInputs(audioioItf, &nbAudioDevs, audioDevIds);
        MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to audio input interfaces from engine\n"));
        
        for(SLint32 i=0;i<nbAudioDevs;i++)
        {
            SLAudioInputDescriptor dev;
            if ((*audioioItf)->QueryAudioInputCapabilities(audioioItf, audioDevIds[i], &dev) ==
                SL_RESULT_SUCCESS)
            {
                MYTRACE(ACE_TEXT("Dev #%d, name: %s\n"), dev.deviceName);
            }
        }
    }
*/

    RefreshDevices();

    MYTRACE(ACE_TEXT("Initializing OpenSL ES: %s\n"), 
            result == SL_RESULT_SUCCESS? ACE_TEXT("success") : ACE_TEXT("failure"));

    return result == SL_RESULT_SUCCESS;
}

void OpenSLESWrapper::Close()
{
    if(m_engineObject)
    {
        std::vector<soundgroup_t> grps = GetSoundGroups();
        for (auto sndgrp : grps)
            CloseOutputMixObject(sndgrp);
        
        (*m_engineObject)->Destroy(m_engineObject);
        m_engineObject = NULL;
        m_engineEngine = NULL;
    }

    MYTRACE(ACE_TEXT("Closed OpenSL ES\n"));
}

std::shared_ptr<OpenSLESWrapper> OpenSLESWrapper::getInstance()
{
    static std::shared_ptr<OpenSLESWrapper> p(new OpenSLESWrapper());
    return p;
}

SLObjectItf OpenSLESWrapper::InitOutputMixObject(soundgroup_t& sndgrp)
{
    std::lock_guard<std::recursive_mutex> g(sndgrp->mutex);
    
    assert(sndgrp->refCount >= 0);
    if (sndgrp->refCount >= 1)
    {
        sndgrp->refCount++;
        assert(sndgrp->outputMixObject);
        MYTRACE("Updated output mix object %p, ref count: %d\n", sndgrp->outputMixObject, sndgrp->refCount);
        return sndgrp->outputMixObject;
    }

    SLresult result;
    SLObjectItf outputMixObject;
    assert(m_engineEngine);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids_env[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req_env[1] = {SL_BOOLEAN_FALSE};
    result = (*m_engineEngine)->CreateOutputMix(m_engineEngine, &outputMixObject, 
                                                1, ids_env, req_env);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to create OpenSLES output mux\n"));
    if(SL_RESULT_SUCCESS != result)
    {
        return nullptr;
    }

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to realize OpenSLES output mux\n"));
    if(SL_RESULT_SUCCESS != result)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        return nullptr;
    }

    // output mix object created successfully
    sndgrp->refCount = 1;
    sndgrp->outputMixObject = outputMixObject;
    
    MYTRACE("Create output mix object %p, ref count: %d\n", outputMixObject, sndgrp->refCount);
    return sndgrp->outputMixObject;
}

void OpenSLESWrapper::CloseOutputMixObject(soundgroup_t& sndgrp)
{
    std::lock_guard<std::recursive_mutex> g(sndgrp->mutex);

    if (!sndgrp->outputMixObject)
    {
        MYTRACE("Ignored close on output mix object. It was never created.\n");
        return;
    }
    
    sndgrp->refCount--;
    assert(sndgrp->refCount >= 0);

    MYTRACE("Output mix object %p decremented: %d\n", sndgrp->outputMixObject, sndgrp->refCount);
    
    if (sndgrp->refCount > 0)
        return;
    
    MYTRACE("Removing output mix object %p\n", sndgrp->outputMixObject);
    (*sndgrp->outputMixObject)->Destroy(sndgrp->outputMixObject);
    sndgrp->outputMixObject = nullptr;
}

soundgroup_t OpenSLESWrapper::NewSoundGroup()
{
    soundgroup_t sg(new SLSoundGroup());
    InitOutputMixObject(sg);
    return sg;
}

void OpenSLESWrapper::RemoveSoundGroup(soundgroup_t grp)
{
    assert(grp);
    CloseOutputMixObject(grp);
    assert(!grp->outputMixObject);
}

bool OpenSLESWrapper::GetDefaultDevices(int& inputdeviceid,
                                        int& outputdeviceid)
{
    return GetDefaultDevices(SOUND_API_OPENSLES_ANDROID,
                             inputdeviceid,
                             outputdeviceid);
}

bool OpenSLESWrapper::GetDefaultDevices(SoundAPI sndsys,
                                        int& inputdeviceid,
                                        int& outputdeviceid)
{
    if(sndsys == SOUND_API_OPENSLES_ANDROID)
    {
        inputdeviceid = outputdeviceid = DEFAULT_DEVICE_ID;
        return true;
    }
    return false;
}
        
// this callback handler is called every time a buffer finishes recording
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    SLInputStreamer* streamer = static_cast<SLInputStreamer*>(context);
    SLAndroidSimpleBufferQueueState state;

    std::lock_guard<std::recursive_mutex> g(streamer->mutex);
    
    SLresult result = (*bq)->GetState(bq, &state);
    assert(result == SL_RESULT_SUCCESS);
    if (result != SL_RESULT_SUCCESS)
        return;

    ACE_UINT32 buf_index = streamer->buf_index++ % ANDROID_INPUT_BUFFERS;
    assert(streamer->channels);
    assert(streamer->framesize);
    // MYTRACE(ACE_TEXT("Callback to recorder %p, capture %p, buf size %d, framesize %d\n"), 
    //         context, streamer->recorder, streamer->buffers[buf_index].size(), 
    //         streamer->framesize);
    int n_samples = streamer->buffers[buf_index].size() / streamer->channels;
    for(int sample_index=0; sample_index<n_samples;
        sample_index += (streamer->framesize * streamer->channels))
    {
        streamer->recorder->StreamCaptureCb(*streamer,
                                            &streamer->buffers[buf_index][sample_index],
                                            streamer->framesize);
    }
    result = (*bq)->Enqueue(bq, &streamer->buffers[buf_index][0],
                            streamer->buffers[buf_index].size()*sizeof(short));
    assert(SL_RESULT_SUCCESS == result);
}

inputstreamer_t OpenSLESWrapper::NewStream(StreamCapture* capture, 
                                           int inputdeviceid, int sndgrpid, 
                                           int samplerate, int channels,
                                           int framesize)
{
    SLresult result;

    soundgroup_t sg = GetSoundGroup(sndgrpid);
    if (!sg)
        return inputstreamer_t();

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    SLuint32 sl_samplerate = toSLSamplerate(samplerate);
    if(!sl_samplerate)
        return inputstreamer_t();
    SLuint32 sl_speaker = toSLSpeaker(channels);

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                     ANDROID_INPUT_BUFFERS};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, (SLuint32)channels, sl_samplerate,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   sl_speaker, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    SLObjectItf recorderObject = nullptr;
    SLRecordItf recorderRecord = nullptr;
    SLAndroidSimpleBufferQueueItf recorderBufferQueue = nullptr;
    inputstreamer_t streamer;
    int frames_per_callback = 0;

    const SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                 SL_IID_ANDROIDACOUSTICECHOCANCELLATION,
                                 SL_IID_ANDROIDNOISESUPPRESSION,
                                 SL_IID_ANDROIDAUTOMATICGAINCONTROL};
    
    const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE};
    
    const SLuint32 n_ids = sizeof(ids)/sizeof(ids[0]);
    result = (*m_engineEngine)->CreateAudioRecorder(m_engineEngine, 
                                                    &recorderObject, 
                                                    &audioSrc, &audioSnk, n_ids, ids, req);

    if (SL_RESULT_SUCCESS != result) {
        MYTRACE(ACE_TEXT("Failed to create OpenSL audio recorder\n"));
        return inputstreamer_t();
    }

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        MYTRACE(ACE_TEXT("Failed to realize OpenSL audio recorder\n"));
        goto failure;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, 
                                             &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    SLAndroidAcousticEchoCancellationItf aecItf;
    result = (*m_engineObject)->GetInterface(recorderObject, SL_IID_ANDROIDACOUSTICECHOCANCELLATION, &aecItf);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to get echo cancel interface from engine\n"));
    MYTRACE_COND(SL_RESULT_SUCCESS == result, ACE_TEXT("Succeeded to get echo cancel interface from engine\n"));

    SLAndroidNoiseSuppressionItf noiseItf;
    result = (*m_engineObject)->GetInterface(recorderObject, SL_IID_ANDROIDNOISESUPPRESSION, &noiseItf);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to get noise reduction interface from engine\n"));
    MYTRACE_COND(SL_RESULT_SUCCESS == result, ACE_TEXT("Succeeded to get noise reduction interface from engine\n"));

    SLAndroidAutomaticGainControlItf agcItf;
    result = (*m_engineObject)->GetInterface(recorderObject, SL_IID_ANDROIDAUTOMATICGAINCONTROL, &agcItf);
    MYTRACE_COND(SL_RESULT_SUCCESS != result, ACE_TEXT("Failed to get AGC interface from engine\n"));
    MYTRACE_COND(SL_RESULT_SUCCESS == result, ACE_TEXT("Succeeded to get AGC interface from engine\n"));
    
    // store input stream properties for callback
    streamer.reset(new SLInputStreamer(capture,
                                       sndgrpid,
                                       framesize,
                                       samplerate,
                                       channels,
                                       SOUND_API_OPENSLES_ANDROID,
                                       inputdeviceid));

    streamer->recorderObject = recorderObject;
    streamer->recorderRecord = recorderRecord;
    streamer->recorderBufferQueue = recorderBufferQueue;

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, 
                                                      bqRecorderCallback,
                                                      streamer.get());
    assert(SL_RESULT_SUCCESS == result);
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    //figure out how many 'framesize' we need in each callback
    frames_per_callback = detectMinumumBuffer(recorderBufferQueue,
                                              streamer->buffers[0],
                                              samplerate, framesize, 
                                              channels);
    if (frames_per_callback == 0)
        goto failure;

    MYTRACE(ACE_TEXT("Minimum samples for recording is %d\n"), frames_per_callback * framesize);
    
    for (size_t i=1;i<ANDROID_INPUT_BUFFERS;i++)
    {
        streamer->buffers[i].resize(frames_per_callback*framesize*channels);
        // enqueue an empty buffer to be filled by the recorder
        // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue,
                                                 &streamer->buffers[i][0],
                                                 streamer->buffers[i].size()*sizeof(short));
        MYTRACE_COND(SL_RESULT_SUCCESS != result,
                     ACE_TEXT("Failed to enqueue in recorder\n"));
        assert(SL_RESULT_SUCCESS == result);
        if(SL_RESULT_SUCCESS != result)
            goto failure;
    }

    MYTRACE(ACE_TEXT("Opened capture stream %p, samplerate %d, channels %d\n"),
            capture, samplerate, channels);

    return streamer;

failure:
    if(recorderObject)
        (*recorderObject)->Destroy(recorderObject);
    return inputstreamer_t();
}

bool OpenSLESWrapper::StartStream(inputstreamer_t streamer)
{
    SLresult result;

    // start recording
    result = (*streamer->recorderRecord)->SetRecordState(streamer->recorderRecord, 
                                                         SL_RECORDSTATE_RECORDING);
    assert(SL_RESULT_SUCCESS == result);

    return SL_RESULT_SUCCESS == result;
}

bool OpenSLESWrapper::StopStream(inputstreamer_t streamer)
{
    SLresult result;

    // start recording
    result = (*streamer->recorderRecord)->SetRecordState(streamer->recorderRecord, 
                                                         SL_RECORDSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);

    return SL_RESULT_SUCCESS == result;
}

void OpenSLESWrapper::CloseStream(inputstreamer_t streamer)
{
    SLresult result;

    // in case already recording, stop recording and clear buffer queue
    StopStream(streamer);

    {
        //wait for recorder callback to complete, otherwise OpenSLES
        //may hang
        std::lock_guard<std::recursive_mutex> g(streamer->mutex);
    }

    result = (*streamer->recorderBufferQueue)->Clear(streamer->recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    (*streamer->recorderObject)->Destroy(streamer->recorderObject);
    
    MYTRACE(ACE_TEXT("Closed capture stream %p\n"), streamer->recorder);
}

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    SLresult result;

    SLOutputStreamer* streamer = static_cast<SLOutputStreamer*>(context);
    bool more = true;
    std::lock_guard<std::recursive_mutex> g(streamer->mutex);

    ACE_UINT32 buf_index = streamer->buf_index++ % ANDROID_OUTPUT_BUFFERS;
    assert(streamer->channels);
    assert(streamer->framesize);
    int n_samples = streamer->buffers[buf_index].size() / streamer->channels;
    // MYTRACE(ACE_TEXT("bqPlayerCallback, samples %d, framesize %d\n"), n_samples, streamer->framesize);
    for(int sample_index=0; more && sample_index<n_samples; 
        sample_index += (streamer->framesize * streamer->channels))
    {
        more = streamer->player->StreamPlayerCb(*streamer,
                                                &streamer->buffers[buf_index][sample_index],
                                                streamer->framesize);
        //soft volume also handles mute
        SoftVolume(OpenSLESWrapper::getInstance().get(), *streamer, 
                   &streamer->buffers[buf_index][sample_index],
                   streamer->framesize);
    }
    result = (*bq)->Enqueue(bq, &streamer->buffers[buf_index][0],
                            streamer->buffers[buf_index].size()*sizeof(short));
    assert(SL_RESULT_SUCCESS == result);

    if (!more)
    {
        SLPlayItf playerPlay = streamer->playerPlay;

        // set the player's state to stopped
        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
        assert(SL_RESULT_SUCCESS == result);
    }
}

outputstreamer_t OpenSLESWrapper::NewStream(soundsystem::StreamPlayer* player, 
                                            int outputdeviceid,
                                            int sndgrpid, int samplerate, 
                                            int channels, int framesize)
{
    SLresult result;

    soundgroup_t sg = GetSoundGroup(sndgrpid);
    if (!sg)
        return outputstreamer_t();

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                       ANDROID_OUTPUT_BUFFERS};

    SLuint32 sl_samplerate = toSLSamplerate(samplerate);
    if(!sl_samplerate)
        return outputstreamer_t();
    
    SLuint32 sl_speaker = toSLSpeaker(channels);

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, (SLuint32)channels, sl_samplerate,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   sl_speaker, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    SLObjectItf outputMixObject = InitOutputMixObject(sg);
    if (!outputMixObject)
    {
        MYTRACE(ACE_TEXT("Failed to create OpenSL ES output mix\n"));
        return outputstreamer_t();
    }

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    SLObjectItf playerObject = nullptr;
    SLPlayItf playerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf playerBufferQueue = nullptr;
    outputstreamer_t streamer;
    int frames_per_callback = 0;

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
                                  /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                              /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*m_engineEngine)->CreateAudioPlayer(m_engineEngine, &playerObject, 
                                                  &audioSrc, &audioSnk,
                                                  3, ids, req);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to create OpenSL ES player object\n"));
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    // realize the player
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to realize OpenSL ES player object\n"));
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    // get the play interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to get OpenSL ES play interface\n"));
    assert(SL_RESULT_SUCCESS == result);
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    // get the buffer queue interface
    result = (*playerObject)->GetInterface(playerObject, 
                                           SL_IID_BUFFERQUEUE,
                                           &playerBufferQueue);
    MYTRACE_COND(SL_RESULT_SUCCESS != result,
                 ACE_TEXT("Failed to get OpenSL ES buffer interface\n"));
    assert(SL_RESULT_SUCCESS == result);
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    streamer.reset(new SLOutputStreamer(player, sndgrpid, 
                                        framesize, samplerate,
                                        channels, 
                                        SOUND_API_OPENSLES_ANDROID,
                                        outputdeviceid));
  
    streamer->playerObject = playerObject;
    streamer->playerPlay = playerPlay;
    streamer->playerBufferQueue = playerBufferQueue;

    // register callback on the buffer queue
    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue,
                                                    bqPlayerCallback,
                                                    streamer.get());
    assert(SL_RESULT_SUCCESS == result);
    if (result != SL_RESULT_SUCCESS)
        goto failure;

    //figure out how many 'framesize' we need in each callback
    frames_per_callback = detectMinumumBuffer(playerBufferQueue,
                                              streamer->buffers[0],
                                              samplerate, framesize, 
                                              channels);
    
    MYTRACE(ACE_TEXT("Minimum samples for playback is %d\n"), frames_per_callback * framesize);

    if(frames_per_callback == 0)
        goto failure;
    
    for (size_t i=1;i<ANDROID_OUTPUT_BUFFERS;i++)
    {
        streamer->buffers[i].resize(frames_per_callback*framesize*channels);
        // here we only enqueue one buffer because it is a long clip,
        // but for streaming playback we would typically enqueue at least 2 buffers to start
        result = (*playerBufferQueue)->Enqueue(playerBufferQueue,
                                               &streamer->buffers[i][0],
                                               streamer->buffers[i].size()*sizeof(short));
        MYTRACE_COND(SL_RESULT_SUCCESS != result,
                     ACE_TEXT("Failed to enqueue in player\n"));
        assert(SL_RESULT_SUCCESS == result);
        if(SL_RESULT_SUCCESS != result)
            goto failure;
    }

    // // get the volume interface
    // SLVolumeItf playerVolume;
    // result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &playerVolume);
    // assert(SL_RESULT_SUCCESS == result);

    // SLmillibel vol, max_vol;
    // result = (*playerVolume)->GetVolumeLevel(playerVolume, &vol);
    // assert(SL_RESULT_SUCCESS == result);
    // result = (*playerVolume)->GetMaxVolumeLevel(playerVolume, &max_vol);
    // assert(SL_RESULT_SUCCESS == result);

    // MYTRACE(ACE_TEXT("Volume %d, Max volume %d\n"), vol, max_vol);

    MYTRACE(ACE_TEXT("Opened playback, samplerate %d, channels %d, framesize %d\n"),
            samplerate, channels, framesize);

    return streamer;

failure:
    
    if(playerObject)
        (*playerObject)->Destroy(playerObject);

    if (outputMixObject)
        CloseOutputMixObject(sg);

    return outputstreamer_t();
}

void OpenSLESWrapper::CloseStream(outputstreamer_t streamer)
{
    StopStream(streamer);

    (*streamer->playerObject)->Destroy(streamer->playerObject);

    soundgroup_t sg = GetSoundGroup(streamer->sndgrpid);
    assert(sg);
    assert(sg->outputMixObject);
    CloseOutputMixObject(sg);
}

bool OpenSLESWrapper::StartStream(outputstreamer_t streamer)
{
    SLresult result;

    SLPlayItf playerPlay = streamer->playerPlay;
    assert(playerPlay);

    // set the player's state to playing
    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

    return SL_RESULT_SUCCESS == result;
}

bool OpenSLESWrapper::StopStream(outputstreamer_t streamer)
{
    SLresult result;

    SLPlayItf playerPlay = streamer->playerPlay;
    assert(playerPlay);

    // set the player's state to stopped
    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);

    //wait for player callback to complete
    std::lock_guard<std::recursive_mutex> g2(streamer->mutex);

    return SL_RESULT_SUCCESS == result;
}

bool OpenSLESWrapper::IsStreamStopped(outputstreamer_t streamer)
{
    SLresult result;

    SLPlayItf playerPlay = streamer->playerPlay;
    assert(playerPlay);

    SLuint32 state = 0;
    result = (*playerPlay)->GetPlayState(playerPlay, &state);
    assert(SL_RESULT_SUCCESS == result);
    return state == SL_PLAYSTATE_STOPPED;
}

void OpenSLESWrapper::FillDevices(sounddevices_t& sounddevs)
{
    SLresult result;

    DeviceInfo dev;
    dev.devicename = ACE_TEXT("Default sound device");
    dev.soundsystem = SOUND_API_OPENSLES_ANDROID;
    dev.id = DEFAULT_DEVICE_ID;

    assert(m_engineEngine);
    
    for(size_t sr=0;sr<standardSampleRates.size();sr++)
    {
        for(int c=1;c<=2;c++)
        {
            // configure audio recorder
            SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, 
                                              SL_IODEVICE_AUDIOINPUT,
                                              SL_DEFAULTDEVICEID_AUDIOINPUT, 
                                              NULL};
            SLDataSource audioSrc = {&loc_dev, NULL};

            SLuint32 sl_samplerate = toSLSamplerate(standardSampleRates[sr]);
            assert(sl_samplerate);
            SLuint32 sl_speaker = toSLSpeaker(c);

            // configure audio sink
            SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                             ANDROID_INPUT_BUFFERS};
            SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, (SLuint32)c, 
                                           sl_samplerate,
                                           SL_PCMSAMPLEFORMAT_FIXED_16, 
                                           SL_PCMSAMPLEFORMAT_FIXED_16,
                                           sl_speaker, 
                                           SL_BYTEORDER_LITTLEENDIAN};
            SLDataSink audioSnk = {&loc_bq, &format_pcm};

            // create audio recorder
            // (requires the RECORD_AUDIO permission)
            SLObjectItf recorderObject = NULL;

            const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
            const SLboolean req[1] = {SL_BOOLEAN_TRUE};
            result = (*m_engineEngine)->CreateAudioRecorder(m_engineEngine, 
                                                            &recorderObject, 
                                                            &audioSrc, &audioSnk, 1, id, req);
            if (SL_RESULT_SUCCESS != result) {
                MYTRACE(ACE_TEXT("Failed to query OpenSL audio recorder, channels=%d, samplerate=%d\n"), c, standardSampleRates[sr]);
                continue;
            }
    
            // // realize the audio recorder
            // result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
            // if (SL_RESULT_SUCCESS != result) {
            //     MYTRACE(ACE_TEXT("Failed to realize OpenSL audio recorder\n"));
            // }

            dev.input_channels.insert(c);
            if(c > dev.max_input_channels)
                dev.max_input_channels = c;
            dev.input_samplerates.insert(standardSampleRates[sr]);
            if(recorderObject)
                (*recorderObject)->Destroy(recorderObject);

            MYTRACE(ACE_TEXT("Query success for OpenSL audio recorder, channels=%d, samplerate=%d\n"), c, standardSampleRates[sr]);
        }
    }

    soundgroup_t sg(new SLSoundGroup());

    SLObjectItf outputMixObject = InitOutputMixObject(sg);
    for(size_t sr=0;sr<standardSampleRates.size() && outputMixObject;sr++)
    {
        for(int c=1;c<=2;c++)
        {
            // configure audio player
            SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                               ANDROID_OUTPUT_BUFFERS};

            SLuint32 sl_samplerate = toSLSamplerate(standardSampleRates[sr]);
            SLuint32 sl_speaker = toSLSpeaker(c);
 
            SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, (SLuint32)c, 
                                           sl_samplerate,
                                           SL_PCMSAMPLEFORMAT_FIXED_16, 
                                           SL_PCMSAMPLEFORMAT_FIXED_16,
                                           sl_speaker, 
                                           SL_BYTEORDER_LITTLEENDIAN};
            SLDataSource audioSrc = {&loc_bufq, &format_pcm};


            // configure audio sink
            SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
            SLDataSink audioSnk = {&loc_outmix, NULL};

            SLObjectItf playerObject = NULL;

            // create audio player
            const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
                                          /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
            const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                                      /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
            result = (*m_engineEngine)->CreateAudioPlayer(m_engineEngine, &playerObject, 
                                                          &audioSrc, &audioSnk,
                                                          3, ids, req);
            if(SL_RESULT_SUCCESS != result)
            {
                MYTRACE(ACE_TEXT("Failed to query OpenSL audio player, channels=%d, samplerate=%d\n"), c, standardSampleRates[sr]);
                continue;
            }

            // // realize the player
            // result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
            // assert(SL_RESULT_SUCCESS == result);

            if(playerObject)
                (*playerObject)->Destroy(playerObject);

            dev.output_channels.insert(c);
            if(c > dev.max_output_channels)
                dev.max_output_channels = c;
            dev.output_samplerates.insert(standardSampleRates[sr]);

            MYTRACE(ACE_TEXT("Query success for OpenSL audio player, channels=%d, samplerate=%d\n"), c, standardSampleRates[sr]);
        }
    }

    //find best sample rate
    dev.default_samplerate = DEFAULT_SAMPLERATE;

    for(std::set<int>::const_iterator i=dev.output_samplerates.begin();i!=dev.output_samplerates.end();i++)
    {
        if(*i > dev.default_samplerate)
            dev.default_samplerate = *i;
    }

    sounddevs[dev.id] = dev;

    if (outputMixObject)
        CloseOutputMixObject(sg);
}

SLuint32 toSLSamplerate(int samplerate)
{
    switch(samplerate)
    {
    case 8000 :
        return SL_SAMPLINGRATE_8;
    case 12000 :
        return SL_SAMPLINGRATE_12;
    case 16000 :
        return SL_SAMPLINGRATE_16;
    case 24000 :
        return SL_SAMPLINGRATE_24;
    case 32000 :
        return SL_SAMPLINGRATE_32;
    case 44100 :
        return SL_SAMPLINGRATE_44_1;
    case 48000 :
        return SL_SAMPLINGRATE_48;
    default :
        return 0;
    }
}

SLuint32 toSLSpeaker(int channels)
{
    switch(channels)
    {
    default :
    case 1 :
        return SL_SPEAKER_FRONT_CENTER;
    case 2 :
        return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
//        return SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT;
    }
}

int detectMinumumBuffer(SLAndroidSimpleBufferQueueItf bq, 
                        std::vector<short>& buffer, int samplerate,
                        int framesize, int channels)
{
    SLresult result;
    //detect how small a frame can be queued into the buffer (3
    //seconds of buffer space is maximum)
    int frames_per_callback = 1;
    while(frames_per_callback * framesize <= 3 * samplerate)
    {
        int n_samples = frames_per_callback * framesize;
        buffer.resize(n_samples * channels);
        result = (*bq)->Enqueue(bq, &buffer[0], buffer.size()*sizeof(short));
        if(result == SL_RESULT_SUCCESS)
            return frames_per_callback;
        else
            frames_per_callback++;
    }
    return 0;
}

} //namespace

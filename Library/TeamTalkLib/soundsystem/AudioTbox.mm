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

#include "AudioTbox.h"

#include <codec/MediaUtil.h>

#include <vector>
#include <assert.h>

#import <AudioToolbox/AudioToolbox.h>

using namespace soundsystem;
using namespace std;

#define AUDIO_BUFFERS 3
#define DEFAULT_DEVICE_ID 0

static void AudioInputCallback(void *userdata, AudioQueueRef audio_queue, 
                               AudioQueueBufferRef audio_buffer, 
                               const AudioTimeStamp *inStartTime,
                               UInt32 inNumberPacketDescriptions,
                               const AudioStreamPacketDescription *inPacketDescs);

static void AudioOutputCallback(void *userdata,
                                AudioQueueRef audio_queue,
                                AudioQueueBufferRef audio_buffer);
namespace soundsystem {

    struct ATInputStreamer : InputStreamer
    {
        AudioQueueRef audio_queue;
        ATInputStreamer() : audio_queue(NULL) {}
    };

    struct ATOutputStreamer : OutputStreamer
    {
        AudioQueueRef audio_queue;
        bool playing;
        ATOutputStreamer() : audio_queue(NULL), playing(false) { }
    };
}

AudioTbox::AudioTbox()
{
    Init();
}

AudioTbox::~AudioTbox()
{
    Close();
}

bool AudioTbox::Init()
{
    OSStatus status = AudioSessionInitialize(NULL, NULL, nil, NULL);
    status = AudioSessionSetActive(true);
    assert(status == noErr);

    UInt32 sessionCategory = kAudioSessionCategory_PlayAndRecord;
    status = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,
                                     sizeof(sessionCategory),
                                     &sessionCategory);
    MYTRACE_COND(status != noErr, ACE_TEXT("Failed to set property kAudioSessionProperty_AudioCategory"));

    Float32 bufferSizeInSec = 0.02f;
    status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration,
                                     sizeof(Float32), &bufferSizeInSec);
    MYTRACE_COND(status != noErr, ACE_TEXT("Failed to set property kAudioSessionProperty_PreferredHardwareIOBufferDuration"));

    RefreshDevices();

    return status == noErr;
}

void AudioTbox::Close()
{
}

AudioTbox* AudioTbox::getInstance()
{
    static AudioTbox aud;
    return &aud;
}

bool AudioTbox::CanRestart()
{
    return false;
}

SoundGroup* AudioTbox::GetSoundGroup(int sndgrpid)
{
    wguard_t g(m_sndgrp_lock);

    soundgroups_t::iterator i = m_sndgrps.find(sndgrpid);
    if(i != m_sndgrps.end())
        return &i->second;
    return NULL;
}

int AudioTbox::NewSoundGroup()
{
    static int sndgrpid = 0;
    return ++sndgrpid;
}

void AudioTbox::RemoveSoundGroup(int sndgrpid)
{
    wguard_t g(m_sndgrp_lock);
    m_sndgrps.erase(sndgrpid);
}

void AudioTbox::FillDevices(sounddevices_t& sounddevs)
{
    DeviceInfo dev;
    dev.devicename = ACE_TEXT("Default sound device");
    dev.soundsystem = SOUND_API_AUDIOTOOLKIT;
    dev.id = DEFAULT_DEVICE_ID;

    for(size_t sr=0;sr<standardSampleRates.size();sr++)
    {
        dev.input_samplerates.insert(standardSampleRates[sr]);
        dev.output_samplerates.insert(standardSampleRates[sr]);
    }    

    dev.max_input_channels = 2;
    dev.max_output_channels = 2;
    dev.default_samplerate = 16000;

    dev.input_channels.insert(1);
    dev.input_channels.insert(2);
    dev.output_channels.insert(1);
    dev.output_channels.insert(2);

    sounddevs[dev.id] = dev;
}

bool AudioTbox::GetDefaultDevices(int& inputdeviceid,
                                int& outputdeviceid)
{
    GetDefaultDevices(SOUND_API_AUDIOTOOLKIT, inputdeviceid, outputdeviceid);
    return true;
}
 
bool AudioTbox::GetDefaultDevices(SoundAPI sndsys,
                                int& inputdeviceid,
                                int& outputdeviceid)
{
    inputdeviceid = outputdeviceid = DEFAULT_DEVICE_ID;
    return true;
}

bool AudioTbox::OpenInputStream(StreamCapture* capture, int inputdeviceid, 
                              int sndgrpid, int samplerate, int channels,
                              int framesize)
{
    AudioStreamBasicDescription format = {0};
    format.mSampleRate = samplerate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    format.mBitsPerChannel = 8 * sizeof(short);
    format.mChannelsPerFrame = channels;
    format.mBytesPerFrame = channels * sizeof(short);
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
    format.mReserved = 0;

    wguard_t g(m_cap_lock);

    OSStatus status = 0;
    AudioQueueRef audio_queue;
    int buffer_size = PCM16_BYTES(framesize, channels);
    ATInputStreamer* streamer = NULL;
    ACE_NEW_RETURN(streamer, ATInputStreamer(), false);
    streamer->recorder = capture;
    streamer->framesize = framesize;
    streamer->samplerate = samplerate;
    streamer->channels = channels;
    streamer->sndgrpid = sndgrpid;
    streamer->soundsystem = SOUND_API_AUDIOTOOLKIT;

    status = AudioQueueNewInput(&format, AudioInputCallback, streamer, 
                                NULL, NULL, 0, &audio_queue);
    assert(status == 0);
    
    for (int i=0;i<AUDIO_BUFFERS && status == 0;i++)
    {
        AudioQueueBufferRef buf;
        status = AudioQueueAllocateBuffer(audio_queue, buffer_size,
                                          &buf);
        assert(status == 0);
        if(status == 0)
        {
            buf->mAudioDataByteSize = buffer_size;
            status = AudioQueueEnqueueBuffer(audio_queue,
                                             buf, 0, NULL);
            assert(status == 0);
            if(status == 0)
                memset(buf->mAudioData, 0, buffer_size);
            else goto fail;
        }
        else goto fail;
    }

    streamer->audio_queue = audio_queue;

    status = AudioQueueStart(streamer->audio_queue, NULL);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to start input audio queue: %d\n"), status);

    if(status != 0)
        goto fail;

    m_recorders[capture] = streamer;

    MYTRACE(ACE_TEXT("Opened and started input device %d with samplerate %d and channels %d\n"),
            inputdeviceid, samplerate, channels);

    return true;

fail:
    AudioQueueDispose(audio_queue, true);
    delete streamer;
    MYTRACE(ACE_TEXT("Failed to start input device %d, status %d\n"), inputdeviceid, status);
    return false;
    
}

bool AudioTbox::CloseInputStream(StreamCapture* capture)
{
    wguard_t g(m_cap_lock);

    ATInputStreamer* streamer = m_recorders[capture];
    if(!streamer)
        return false;
    
    m_recorders.erase(capture);
    // g.release();

    OSStatus status;

    status = AudioQueueStop(streamer->audio_queue, true);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to stop input audio queue\n"));
    status = AudioQueueDispose(streamer->audio_queue, true);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to dispose input audio queue\n"));

    delete streamer;

    return true;
    
}

bool AudioTbox::OpenOutputStream(StreamPlayer* player, int outputdeviceid,
                               int sndgrpid, int samplerate, int channels, 
                               int framesize)
{

    AudioStreamBasicDescription format = {0};
    format.mSampleRate = samplerate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    format.mBitsPerChannel = 8 * sizeof(short);
    format.mChannelsPerFrame = channels;
    format.mBytesPerFrame = channels * sizeof(short);
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
    format.mReserved = 0;

    wguard_t g(m_play_lock);

    OSStatus status = 0;
    AudioQueueRef audio_queue;
    int buffer_size = PCM16_BYTES(framesize, channels);
    ATOutputStreamer* streamer = NULL;
    ACE_NEW_RETURN(streamer, ATOutputStreamer(), false);
    streamer->player = player;
    streamer->framesize = framesize;
    streamer->samplerate = samplerate;
    streamer->channels = channels;
    streamer->sndgrpid = sndgrpid;
    streamer->soundsystem = SOUND_API_AUDIOTOOLKIT;

    status = AudioQueueNewOutput(&format, AudioOutputCallback, streamer, 
                                 NULL, NULL, 0, &audio_queue);
    assert(status == 0);
        
    for (int i=0;i<AUDIO_BUFFERS && status == 0;i++)
    {
        AudioQueueBufferRef buf;
        status = AudioQueueAllocateBuffer(audio_queue, buffer_size,
                                          &buf);
        assert(status == 0);
        if(status == 0)
        {
            buf->mAudioDataByteSize = buffer_size;
            status = AudioQueueEnqueueBuffer(audio_queue,
                                             buf, 0, NULL);
            assert(status == 0);
            if(status == 0)
                memset(buf->mAudioData, 0, buffer_size);
            else goto fail;
        }
        else goto fail;
    }

    streamer->audio_queue = audio_queue;

    MYTRACE(ACE_TEXT("Opened output device %d with samplerate %d and channels %d\n"),
            outputdeviceid, samplerate, channels);

    streamer->playing = !paused;

    m_players[player] = streamer;

    return true;

fail:
    AudioQueueDispose(audio_queue, true);
    delete streamer;
    return false;
}

bool AudioTbox::CloseOutputStream(StreamPlayer* player)
{
    wguard_t g(m_play_lock);

    ATOutputStreamer* streamer = m_players[player];
    if(!streamer)
        return false;
    m_players.erase(player);
    // g.release();

    OSStatus status;

    status = AudioQueueStop(streamer->audio_queue, true);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to stop audio queue\n"));
    status = AudioQueueDispose(streamer->audio_queue, true);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to dispose audio queue\n"));

    delete streamer;

    return true;
}

bool AudioTbox::StartStream(StreamPlayer* player)
{
    wguard_t g(m_play_lock);

    ATOutputStreamer* streamer = m_players[player];
    if(!streamer)
        return false;

    OSStatus status;
    status = AudioQueueStart(streamer->audio_queue, NULL);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to start output audio queue\n"));
    streamer->playing = true;

    MYTRACE(ACE_TEXT("Start stream with samplerate %d and channels %d\n"),
            streamer->samplerate, streamer->channels);


    return status == 0;
}

bool AudioTbox::StopStream(StreamPlayer* player)
{
    wguard_t g(m_play_lock);

    ATOutputStreamer* streamer = m_players[player];
    if(!streamer)
        return false;

    OSStatus status;
    status = AudioQueueStop(streamer->audio_queue, true);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to stop output audio queue\n"));
    streamer->playing = false;
    return status == 0;
}

bool AudioTbox::IsStreamStopped(StreamPlayer* player)
{
    wguard_t g(m_play_lock);

    ATOutputStreamer* streamer = m_players[player];
    if(!streamer)
        return false;

    return !streamer->playing;
}

bool AudioTbox::IsStreamActive(StreamPlayer* player)
{
    wguard_t g(m_play_lock);

    ATOutputStreamer* streamer = m_players[player];
    if(!streamer)
        return false;

    return streamer->playing;
}

InputStreamer* AudioTbox::GetInputStreamer(StreamCapture* capture)
{
    return m_recorders[capture];
}

OutputStreamer* AudioTbox::GetOutputStreamer(StreamPlayer* player)
{
    return m_players[player];
}

void AudioTbox::GetPlayers(int sndgrpid, std::vector<StreamPlayer*>& players)
{
    players_t::iterator i = m_players.begin();
    for(;i!=m_players.end();i++)
    {
        if(i->second->sndgrpid == sndgrpid)
            players.push_back(i->first);
    }
}

static void AudioInputCallback(void *userdata, AudioQueueRef audio_queue, 
                               AudioQueueBufferRef audio_buffer, 
                               const AudioTimeStamp *inStartTime,
                               UInt32 inNumberPacketDescriptions,
                               const AudioStreamPacketDescription *inPacketDescs)
{
    ATInputStreamer* streamer = reinterpret_cast<ATInputStreamer*>(userdata);
    short* buffer = reinterpret_cast<short*>(audio_buffer->mAudioData);
    streamer->recorder->StreamCaptureCb(*streamer, buffer, streamer->framesize);
    MYTRACE(ACE_TEXT("Audio queue callback at %u\n"), GETTIMESTAMP());
    OSStatus status;
    status = AudioQueueEnqueueBuffer(audio_queue, audio_buffer, 0, NULL);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to queue input buffer, status = %d\n"), status);
    
}

static void AudioOutputCallback(void *userdata, AudioQueueRef audio_queue,
                                AudioQueueBufferRef audio_buffer)
{
    ATOutputStreamer* streamer = reinterpret_cast<ATOutputStreamer*>(userdata);
    short* buffer = reinterpret_cast<short*>(audio_buffer->mAudioData);
    streamer->player->StreamPlayerCb(*streamer, buffer, streamer->framesize);

    OSStatus status;
    status = AudioQueueEnqueueBuffer(audio_queue, audio_buffer, 0, NULL);
    MYTRACE_COND(status != 0, ACE_TEXT("Failed to queue output buffer, status = %d\n"), status);
}

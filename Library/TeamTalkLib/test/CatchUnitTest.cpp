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

#include "catch.hpp"

#include <ace/ACE.h>
#include <ace/OS.h>

#include "TTUnitTest.h"

#include <codec/OggOutput.h>
#include <codec/OpusEncoder.h>
#include <codec/WaveFile.h>

#include <myace/MyACE.h>

#include <map>
#include <iostream>

using namespace std;

#if defined(WIN32)
#include <ace/Init_ACE.h>
#include <assert.h>
#include <Mmsystem.h>
#include <propsys.h>
#include <atlbase.h>
#include <MMDeviceApi.h>
#include <avstream/DMOResampler.h>
#include <avstream/PortAudioWrapper.h>

static class WinInit
{
public:
    WinInit()
    {
        int ret = ACE::init();
        assert(ret >= 0);
    }
    ~WinInit()
    {
        int ret = ACE::fini();
        assert(ret >= 0);
    }
} wininit;
#endif

TEST_CASE( "Init TT", "" ) {
    TTInstance* ttinst;
    REQUIRE( (ttinst = TT_InitTeamTalkPoll()) );
    REQUIRE( TT_CloseTeamTalk(ttinst) );
}

TEST_CASE( "Ogg Write", "" ) {
    SpeexEncFile spxfile;
    REQUIRE( spxfile.Open(ACE_TEXT("/foo.spx"), 1, DEFAULT_SPEEX_COMPLEXITY, 7, 32000, 48000, false) == false);
}

TEST_CASE( "Record mux") {
    std::vector<TTInstance*> clients(2);
    for (size_t i=0;i<clients.size();++i)
    {
        REQUIRE((clients[i] = TT_InitTeamTalkPoll()));
        REQUIRE(InitSound(clients[i], SHARED_INPUT));
        REQUIRE(Connect(clients[i], ACE_TEXT("127.0.0.1"), 10333, 10333));
        REQUIRE(Login(clients[i], ACE_TEXT("MyNickname"), ACE_TEXT("guest"), ACE_TEXT("guest")));

        if (i == 0)
        {
            AudioCodec audiocodec = {};
            audiocodec.nCodec = OPUS_CODEC;
            audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
            audiocodec.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
            audiocodec.opus.nFrameSizeMSec = 120;
#else
            audiocodec.opus.nFrameSizeMSec = 40;
#endif
            audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
            audiocodec.opus.nChannels = 2;
            audiocodec.opus.nComplexity = 10;
            audiocodec.opus.nSampleRate= 48000;
            audiocodec.opus.bDTX = true;
            audiocodec.opus.bFEC = true;
            audiocodec.opus.bVBR = false;
            audiocodec.opus.bVBRConstraint = false;

            Channel chan = MakeChannel(clients[i], ACE_TEXT("foo"), TT_GetRootChannelID(clients[i]), audiocodec);
            REQUIRE(WaitForCmdSuccess(clients[i], TT_DoJoinChannel(clients[i], &chan)));
        }
        else
        {
            REQUIRE(WaitForCmdSuccess(clients[i], TT_DoJoinChannelByID(clients[i], TT_GetMyChannelID(clients[0]), ACE_TEXT(""))));
        }
    }

    Channel chan;
    REQUIRE(TT_GetChannel(clients[1], TT_GetMyChannelID(clients[1]), &chan));

    REQUIRE(TT_EnableVoiceTransmission(clients[0], true));
    WaitForEvent(clients[0], CLIENTEVENT_NONE, nullptr, 100);
    REQUIRE(TT_EnableVoiceTransmission(clients[0], false));

    REQUIRE(TT_StartRecordingMuxedAudioFile(clients[1], &chan.audiocodec, ACE_TEXT("MyMuxFile.wav"), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(clients[0], STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceTransmission(clients[0], true));
    WaitForEvent(clients[0], CLIENTEVENT_NONE, nullptr, 2500);
    REQUIRE(TT_EnableVoiceTransmission(clients[0], false));

    REQUIRE(TT_DBG_SetSoundInputTone(clients[1], STREAMTYPE_VOICE, 600));
    REQUIRE(TT_EnableVoiceTransmission(clients[1], true));
    WaitForEvent(clients[1], CLIENTEVENT_NONE, nullptr, 2500);
    REQUIRE(TT_EnableVoiceTransmission(clients[1], false));

    WaitForEvent(clients[1], CLIENTEVENT_NONE, nullptr, 10000);

    REQUIRE(TT_StopRecordingMuxedAudioFile(clients[1]));

    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
}

TEST_CASE( "Last voice packet" )
{
    std::vector<TTInstance*> clients;
    auto txclient = TT_InitTeamTalkPoll();
    auto rxclient = TT_InitTeamTalkPoll();
    clients.push_back(txclient);
    clients.push_back(rxclient);

    REQUIRE(InitSound(txclient, SHARED_INPUT));
    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));

    REQUIRE(InitSound(rxclient, SHARED_INPUT));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));

    AudioCodec audiocodec = {};
    audiocodec.nCodec = OPUS_CODEC;
    audiocodec.opus.nApplication = OPUS_APPLICATION_VOIP;
    audiocodec.opus.nTxIntervalMSec = 240;
#if defined(OPUS_FRAMESIZE_120_MS)
    audiocodec.opus.nFrameSizeMSec = 120;
#else
    audiocodec.opus.nFrameSizeMSec = 60;
#endif
    audiocodec.opus.nBitRate = OPUS_MIN_BITRATE;
    audiocodec.opus.nChannels = 2;
    audiocodec.opus.nComplexity = 10;
    audiocodec.opus.nSampleRate= 48000;
    audiocodec.opus.bDTX = true;
    audiocodec.opus.bFEC = true;
    audiocodec.opus.bVBR = false;
    audiocodec.opus.bVBRConstraint = false;

    Channel chan = MakeChannel(txclient, ACE_TEXT("foo"), TT_GetRootChannelID(txclient), audiocodec);
    REQUIRE(WaitForCmdSuccess(txclient, TT_DoJoinChannel(txclient, &chan)));

    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannelByID(rxclient, TT_GetMyChannelID(txclient), ACE_TEXT(""))));

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 600));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, int(audiocodec.opus.nTxIntervalMSec * 5 + audiocodec.opus.nTxIntervalMSec * .5));
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    auto voicestop = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == 0)
        {
            return true;
        }
        
        return false;
    };

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestop));
    //WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, audiocodec.opus.nTxIntervalMSec * 2);

    TTCHAR curdir[1024] = {};
    ACE_OS::getcwd(curdir, 1024);
    REQUIRE(TT_SetUserMediaStorageDir(rxclient, TT_GetMyUserID(txclient), curdir, ACE_TEXT(""), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 0));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 1000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
    
    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
}

TEST_CASE( "MuxedAudioToFile" )
{
    std::vector<TTInstance*> clients;
    auto txclient = TT_InitTeamTalkPoll();
    auto rxclient = TT_InitTeamTalkPoll();
    clients.push_back(txclient);
    clients.push_back(rxclient);

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));

    Channel chan;
    REQUIRE(TT_GetChannel(rxclient, TT_GetMyChannelID(rxclient), &chan));
    REQUIRE(TT_StartRecordingMuxedAudioFile(rxclient, &chan.audiocodec, ACE_TEXT("MyMuxFile.wav"), AFF_WAVE_FORMAT));

    REQUIRE(TT_DBG_SetSoundInputTone(txclient, STREAMTYPE_VOICE, 500));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 2000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    // This tone is not being stored in 'MyMuxFile.wav' because the
    // audio block will bypass the audio encoder.
    REQUIRE(TT_DBG_SetSoundInputTone(rxclient, STREAMTYPE_VOICE, 600));
    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, nullptr, 2000);
    REQUIRE(TT_EnableVoiceTransmission(rxclient, false));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 2000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));

    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(rxclient, CLIENTEVENT_NONE, nullptr, 2000);
    REQUIRE(TT_EnableVoiceTransmission(rxclient, false));

    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 2000);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoUnsubscribe(rxclient, TT_GetMyUserID(txclient), SUBSCRIBE_VOICE)));
    
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 2000);
    
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoSubscribe(rxclient, TT_GetMyUserID(txclient), SUBSCRIBE_VOICE)));

    REQUIRE(TT_EnableVoiceTransmission(rxclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 2000);

    REQUIRE(TT_CloseSoundInputDevice(rxclient));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    WaitForEvent(txclient, CLIENTEVENT_NONE, nullptr, 2000);
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
    
    REQUIRE(TT_StopRecordingMuxedAudioFile(rxclient));

    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
}

TEST_CASE( "MuxedAudioBlock" )
{
    std::vector<TTInstance*> clients;
    auto txclient = TT_InitTeamTalkPoll();
    auto rxclient = TT_InitTeamTalkPoll();
    clients.push_back(txclient);
    clients.push_back(rxclient);

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));

    TTMessage msg;
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK, &msg));

    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
}

TEST_CASE( "MuxedAudioBlockUserEvent" )
{
    std::vector<TTInstance*> clients;
    auto txclient = TT_InitTeamTalkPoll();
    auto rxclient = TT_InitTeamTalkPoll();
    clients.push_back(txclient);
    clients.push_back(rxclient);

    REQUIRE(InitSound(txclient));
    REQUIRE(Connect(txclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(txclient, ACE_TEXT("TxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(txclient));

    REQUIRE(InitSound(rxclient, DEFAULT, SOUNDDEVICEID_IGNORE, SOUNDDEVICEID_DEFAULT));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));

    TTMessage msg;

    REQUIRE(TT_EnableAudioBlockEvent(rxclient, TT_MUXED_USERID, STREAMTYPE_VOICE, TRUE));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK, &msg));
    REQUIRE(TT_EnableVoiceTransmission(txclient, true));
    auto voicestart = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_VOICE)
        {
            return true;
        }
        
        return false;
    };
    auto voicestop = [&](TTMessage msg)
    {
        if (msg.nClientEvent == CLIENTEVENT_USER_STATECHANGE &&
            msg.user.nUserID == TT_GetMyUserID(txclient) &&
            (msg.user.uUserState & USERSTATE_VOICE) == USERSTATE_NONE)
        {
            return true;
        }
        
        return false;
    };

    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestart));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK, &msg));
    REQUIRE(TT_EnableVoiceTransmission(txclient, false));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_STATECHANGE, voicestop));
    REQUIRE(WaitForEvent(rxclient, CLIENTEVENT_USER_AUDIOBLOCK, &msg));

    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
}

TEST_CASE( "Opus Read File" )
{
    std::vector<TTInstance*> clients;
    auto rxclient = TT_InitTeamTalkPoll();
    clients.push_back(rxclient);

    REQUIRE(InitSound(rxclient));
    REQUIRE(Connect(rxclient, ACE_TEXT("127.0.0.1"), 10333, 10333));
    REQUIRE(Login(rxclient, ACE_TEXT("RxClient"), ACE_TEXT("guest"), ACE_TEXT("guest")));
    REQUIRE(JoinRoot(rxclient));
    AudioCodec codec;
#if defined(ENABLE_OPUSTOOLS) && 0
    codec.nCodec = OPUS_CODEC;
    codec.opus = {};
    codec.opus.nApplication = OPUS_APPLICATION_AUDIO;
    codec.opus.nBitRate = 64000;
    codec.opus.nChannels = 2;
    codec.opus.nComplexity = 9;
    codec.opus.nFrameSizeMSec = 5;
    codec.opus.nSampleRate = 24000;
    codec.opus.nTxIntervalMSec = 400;
    Channel chan = MakeChannel(rxclient, ACE_TEXT("opustools"), TT_GetMyChannelID(rxclient), codec);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));
#else
    codec.nCodec = SPEEX_VBR_CODEC;
    codec.speex_vbr = {};
    codec.speex_vbr.nBandmode = 1;
    codec.speex_vbr.nBitRate = 16000;
    codec.speex_vbr.nMaxBitRate = 32000;
    codec.speex_vbr.nQuality = 5;
    codec.speex_vbr.nTxIntervalMSec = 400;
    Channel chan = MakeChannel(rxclient, ACE_TEXT("speex"), TT_GetMyChannelID(rxclient), codec);
    REQUIRE(WaitForCmdSuccess(rxclient, TT_DoJoinChannel(rxclient, &chan)));
#endif
    const TTCHAR FILENAME[] = ACE_TEXT("MyMuxFile.ogg");
    REQUIRE(TT_GetChannel(rxclient, TT_GetMyChannelID(rxclient), &chan));
    REQUIRE(TT_StartRecordingMuxedAudioFile(rxclient, &chan.audiocodec, FILENAME, AFF_CHANNELCODEC_FORMAT));

    WaitForEvent(rxclient, CLIENTEVENT_NONE, nullptr, 2000);

    for(auto c : clients)
        REQUIRE(TT_CloseTeamTalk(c));
    
    OggFile of;
    REQUIRE(of.Open(FILENAME));
    ogg_page op;
    int pages = 0;
    REQUIRE(of.ReadOggPage(op));
    REQUIRE(op.header_len>0);
    REQUIRE(op.body_len>0);
    pages++;
    while (of.ReadOggPage(op))pages++;
    cout << "pages: " << pages << endl;
}

#if defined(WIN32)

TEST_CASE("CLSID_CWMAudioAEC")
{
    std::vector<SoundDevice> devs(100);
    INT32 nDevs = 100, indev, outdev;
    REQUIRE(TT_GetSoundDevices(&devs[0], &nDevs));
    nDevs = nDevs;
    REQUIRE(TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_WASAPI, &indev, &outdev));

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;
    UINT dwCount;

    REQUIRE(SUCCEEDED(spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator))));

    std::map<std::wstring, UINT> capdevs, spkdevs;

    REQUIRE(SUCCEEDED(spEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &spEndpoints)));
    REQUIRE(SUCCEEDED(spEndpoints->GetCount(&dwCount)));
    for (UINT index = 0; index < dwCount; index++)
    {
        WCHAR* pszDeviceId = NULL;
        PROPVARIANT value;
        CComPtr<IMMDevice> spDevice;
        CComPtr<IPropertyStore> spProperties;

        PropVariantInit(&value);
        REQUIRE(SUCCEEDED(spEndpoints->Item(index, &spDevice)));
        REQUIRE(SUCCEEDED(spDevice->GetId(&pszDeviceId)));

        capdevs[pszDeviceId] = index;

        PropVariantClear(&value);
        CoTaskMemFree(pszDeviceId);
    }
    spEndpoints.Release();

    REQUIRE(SUCCEEDED(spEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &spEndpoints)));
    REQUIRE(SUCCEEDED(spEndpoints->GetCount(&dwCount)));
    for(UINT index = 0; index < dwCount; index++)
    {
        WCHAR* pszDeviceId = NULL;
        PROPVARIANT value;
        CComPtr<IMMDevice> spDevice;
        CComPtr<IPropertyStore> spProperties;

        PropVariantInit(&value);
        REQUIRE(SUCCEEDED(spEndpoints->Item(index, &spDevice)));
        REQUIRE(SUCCEEDED(spDevice->GetId(&pszDeviceId)));

        spkdevs[pszDeviceId] = index;

        PropVariantClear(&value);
        CoTaskMemFree(pszDeviceId);
    }

    CComPtr<IMediaObject> pDMO;
    CComPtr<IPropertyStore> pPS;

    REQUIRE(SUCCEEDED(CoCreateInstance(CLSID_CWMAudioAEC, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (LPVOID*)&pDMO)));
    REQUIRE(SUCCEEDED(pDMO->QueryInterface(IID_IPropertyStore, (LPVOID*)&pPS)));

    const int SAMPLERATE = 22050;
    const int CHANNELS = 1;

    DMO_MEDIA_TYPE mt = {};
    HRESULT hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    REQUIRE(SUCCEEDED(hr));
    REQUIRE(SetWaveMediaType(SAMPLEFORMAT_INT16, CHANNELS, SAMPLERATE, mt));

    REQUIRE(SUCCEEDED(pDMO->SetOutputType(0, &mt, 0)));
    REQUIRE(SUCCEEDED(MoFreeMediaType(&mt)));

    REQUIRE(capdevs.find(devs[indev].szDeviceID) != capdevs.end());
    REQUIRE(spkdevs.find(devs[outdev].szDeviceID) != spkdevs.end());

    indev = capdevs[devs[indev].szDeviceID];
    outdev = spkdevs[devs[outdev].szDeviceID];

    PROPVARIANT pvDeviceId;
    PropVariantInit(&pvDeviceId);
    pvDeviceId.vt = VT_I4;
    pvDeviceId.lVal = (outdev << 16) | indev;
    REQUIRE(SUCCEEDED(pPS->SetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, pvDeviceId)));
    REQUIRE(SUCCEEDED(pPS->GetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, &pvDeviceId)));
    PropVariantClear(&pvDeviceId);

    REQUIRE(SUCCEEDED(pDMO->AllocateStreamingResources()));

    int iFrameSize;
    PROPVARIANT pvFrameSize;
    PropVariantInit(&pvFrameSize);
    REQUIRE(SUCCEEDED(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_FRAME_SIZE, &pvFrameSize)));
    iFrameSize = pvFrameSize.lVal;
    PropVariantClear(&pvFrameSize);
    
    WavePCMFile wavefile;
    REQUIRE(wavefile.NewFile(ACE_TEXT("Echo_cancelled.wav"), SAMPLERATE, CHANNELS));

    int delay = iFrameSize * 1000 / SAMPLERATE;
    int waitMSec = 10000;
    std::vector<BYTE> outputbuf(PCM16_BYTES(SAMPLERATE, CHANNELS));
    do
    {
        CComPtr<IMediaBuffer> ioutputbuf;
        REQUIRE(SUCCEEDED(CMediaBuffer::CreateBuffer(&outputbuf[0], 0, outputbuf.size(), (void**)&ioutputbuf)));
        DMO_OUTPUT_DATA_BUFFER dmodatabuf = {};
        dmodatabuf.pBuffer = ioutputbuf;
        DWORD dwStatus, dwOutputLen = 0;
        hr = pDMO->ProcessOutput(0, 1, &dmodatabuf, &dwStatus);

        BYTE* outputbufptr;
        switch (hr)
        {
            case S_FALSE :
                dwOutputLen = 0;
                break;
            case S_OK :
                hr = ioutputbuf->GetBufferAndLength(&outputbufptr, &dwOutputLen);
                REQUIRE(SUCCEEDED(hr));
                REQUIRE(wavefile.AppendSamples(reinterpret_cast<const short*>(outputbufptr), dwOutputLen / sizeof(short) / CHANNELS));
                break;
            case E_FAIL :
                REQUIRE(SUCCEEDED(hr));
                break;
            case E_INVALIDARG :
                REQUIRE(SUCCEEDED(hr));
                break;
            case E_POINTER :
                REQUIRE(SUCCEEDED(hr));
                break;
            case WMAAECMA_E_NO_ACTIVE_RENDER_STREAM :
                MYTRACE(ACE_TEXT("No audio rendered on device: %s\n"), devs[outdev].szDeviceID);
                REQUIRE(SUCCEEDED(hr));
                break;
            default :
                MYTRACE(ACE_TEXT("Unknown HRESULT from echo cancellor 0x%x\n"), hr);
                REQUIRE(SUCCEEDED(hr));
                break;
        }

        Sleep(delay);

    } while ((waitMSec -= delay) > 0);
}

TEST_CASE("CWMAudioAEC_Callback")
{
    using namespace soundsystem;
    class MyClass : public StreamDuplex
    {
    public:
        int callbacks = 0;
        void StreamDuplexEchoCb(const DuplexStreamer& streamer,
                                const short* input_buffer,
                                const short* prev_output_buffer, int samples)
        {
        }

        void StreamDuplexCb(const DuplexStreamer& streamer,
                            const short* input_buffer,
                            short* output_buffer, int samples)
        {
            MYTRACE(ACE_TEXT("Callback of %d samples\n"), samples);
            callbacks++;
        }

    } myduplex, myduplex2;

    auto sndsys = soundsystem::GetInstance();
    int sndgrpid = sndsys->OpenSoundGroup();
    int indev, outdev;
    REQUIRE(sndsys->GetDefaultDevices(SOUND_API_WASAPI, indev, outdev));
    // test when framesize > CWMAudioAEC framesize
    PaDuplexStreamer paduplex(&myduplex, sndgrpid, 48000 * .1, 48000, 1, 1, SOUND_API_WASAPI, indev, outdev);
    {
        CWMAudioAECCapture aec(&paduplex);
        REQUIRE(aec.Open());
        while (myduplex.callbacks <= 10)
            Sleep(1);
    }

    // test when framesize < CWMAudioAEC framesize
    PaDuplexStreamer paduplex2(&myduplex2, sndgrpid, 48000 * .005, 48000, 1, 1, SOUND_API_WASAPI, indev, outdev);
    {
        CWMAudioAECCapture aec(&paduplex2);
        REQUIRE(aec.Open());
        while(myduplex2.callbacks <= 200)
            Sleep(1);
    }

    sndsys->RemoveSoundGroup(sndgrpid);
}
#endif

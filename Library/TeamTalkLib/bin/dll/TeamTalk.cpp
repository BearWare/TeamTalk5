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

#include <TeamTalkDefs.h>

#if defined(WIN32)
#include <ace/config.h>
#include <win32/HotKey.h>
#include <win32/Mixer.h>
#include <win32/WinFirewall.h>
HINSTANCE hInstance = NULL;

#elif defined(__APPLE__)
#include <TargetConditionals.h>

#endif

#if defined(ENABLE_MINIDUMP)
#include "mdump.h"
#endif

#include <ace/Init_ACE.h>

#include <teamtalk/client/ClientNode.h>
#include <teamtalk/client/AudioContainer.h>
#include <teamtalk/ttassert.h>

#include <TeamTalk.h>
#include "TTClientMsg.h"
#include "Convert.h"

#include <queue>
#include <iostream>

#if defined(ENABLE_SOUNDSYSTEM)
#include <soundsystem/SoundLoopback.h>
#endif

#if defined(ENABLE_VIDCAP)
#include <vidcap/VideoCapture.h>
#endif

#ifdef NDEBUG
#pragma message("Compiling TeamTalk version " TEAMTALK_VERSION " = " TEAMTALK_VERSION)
#endif

using teamtalk::files_t;
using teamtalk::BMPPalette;
using teamtalk::ClientNode;
using teamtalk::clientchannel_t;
using teamtalk::ClientChannel;
using teamtalk::clientuser_t;
using teamtalk::DesktopViewer;
using teamtalk::desktop_viewer_t;
using teamtalk::SoundProperties;
using namespace vidcap;
using namespace std;

struct ClientInstance
{
    ClientNode* pClientNode;
    TTMsgQueue* pEventHandler;
    ACE_Recursive_Thread_Mutex mutex_video;
    typedef std::map<VideoFrame*, ACE_Message_Block*> video_frames_t;
    video_frames_t video_frames;

    VideoFrame* PushVideoFrame(ACE_Message_Block* mb)
    {
        VideoFrame* vid_frame = NULL;
        ACE_NEW_NORETURN(vid_frame, VideoFrame);
        if(!vid_frame)
        {
            mb->release();
            return NULL;
        }
        wguard_t g(mutex_video);
        video_frames[vid_frame] = mb;
        return vid_frame;
    }
    bool RemoveVideoFrame(VideoFrame* vid_frame)
    {
        wguard_t g(mutex_video);
        video_frames_t::iterator ii = video_frames.find(vid_frame);
        TTASSERT(ii != video_frames.end());
        if(ii != video_frames.end())
        {
            ii->second->release();
            delete ii->first;
            video_frames.erase(ii);
            return true;
        }
        return false;
    }

    ACE_Recursive_Thread_Mutex mutex_desktop;
    typedef std::map<DesktopWindow*, ACE_Message_Block*> desktopwindows_t;
    desktopwindows_t desktop_windows;
    DesktopWindow* PushDesktopWindow(int buf_size)
    {
        DesktopWindow* wnd_frame;

        ACE_Message_Block* mb;
        ACE_NEW_RETURN(mb, ACE_Message_Block(sizeof(DesktopWindow) + buf_size),
                       NULL);
        wnd_frame = reinterpret_cast<DesktopWindow*>(mb->rd_ptr());
        wnd_frame->frameBuffer = mb->rd_ptr() + sizeof(DesktopWindow);
        wnd_frame->nFrameBufferSize = buf_size;

        wguard_t g(mutex_desktop);
        desktop_windows[wnd_frame] = mb;
        return wnd_frame;
    }

    bool RemoveDesktopWindow(DesktopWindow* desktop_wnd)
    {
        wguard_t g(mutex_desktop);
        desktopwindows_t::iterator ii = desktop_windows.find(desktop_wnd);
        if(ii != desktop_windows.end())
        {
            ii->second->release();
            desktop_windows.erase(ii);
            return true;
        }
        return false;
    }

    ACE_Recursive_Thread_Mutex mutex_audblocks;
    typedef std::map<AudioBlock*, ACE_Message_Block*> audio_blocks_t;
    audio_blocks_t audio_blocks;

    AudioBlock* PushAudioBlock(ACE_Message_Block* mb)
    {
        AudioBlock* audblock;
        ACE_NEW_NORETURN(audblock, AudioBlock);
        if(!audblock)
        {
            mb->release();
            return NULL;
        }
        wguard_t g(mutex_audblocks);
        audio_blocks[audblock] = mb;
        return audblock;
    }

    bool RemoveAudioBlock(AudioBlock* audblock)
    {
        wguard_t g(mutex_audblocks);
        audio_blocks_t::iterator ii = audio_blocks.find(audblock);
        TTASSERT(ii != audio_blocks.end());
        if(ii != audio_blocks.end())
        {
            ii->second->release();
            delete ii->first;
            audio_blocks.erase(ii);
            return true;
        }
        return false;
    }

    ClientInstance()
    {
        pClientNode = NULL;
        pEventHandler = NULL;
    }

    ~ClientInstance()
    {
        MYTRACE_COND(video_frames.size(),
                     ACE_TEXT("ERROR: Leaking %d VideoFrame structs\n"), (int)video_frames.size());
        MYTRACE_COND(desktop_windows.size(),
                     ACE_TEXT("ERROR: Leaking %d DesktopWindow structs\n"), (int)desktop_windows.size());
        MYTRACE_COND(audio_blocks.size(),
                     ACE_TEXT("ERROR: Leaking %d AudioBlock structs\n"), (int)audio_blocks.size());
    }
};

typedef std::set<ClientInstance*> clients_t;

clients_t clients;
ACE_Recursive_Thread_Mutex clients_mutex;

#if defined(ENABLE_SOUNDSYSTEM)
typedef std::set<SoundLoopback*> soundloops_t;
ACE_Recursive_Thread_Mutex soundloops_mutex;
soundloops_t soundloops;
#endif


#if defined(WIN32)
BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      VOID* lpReserved)
{
    int ret = 0;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            hInstance = (HINSTANCE)hModule;
            ret = ACE::init();
            TTASSERT(ret >= 0);
            break;
        }
    case DLL_THREAD_ATTACH:
        {
            break;
        }
    case DLL_THREAD_DETACH:
        {
            break;
        }
    case DLL_PROCESS_DETACH:
        {
            //while(clients.size())
            //    TT_CloseTeamTalk(*clients.begin());
            ret = ACE::fini();
            TTASSERT(ret>=0);
            break;
        }
    }
    return TRUE;
}

void HOTKEY_USAGE(int num)
{
    static int HOTKEY_USAGE_COUNT = 0;

    if(HOTKEY_USAGE_COUNT == 0 && num>0)
    {
        //install the hotkey hook
        BOOL bMouseHook = InstallHook(hInstance, TRUE);
        TTASSERT(bMouseHook);
    }

    if(HOTKEY_USAGE_COUNT == 1 && num<0)
    {
        //remove key hook
        RemoveHook();
    }
    if(HOTKEY_USAGE_COUNT>=0)
        HOTKEY_USAGE_COUNT += num;
    TTASSERT(HOTKEY_USAGE_COUNT>=0);
}
#endif

ClientInstance* GET_CLIENT(TTInstance* pInstance)
{
    wguard_t g(clients_mutex);

    ClientInstance* pClient = static_cast<ClientInstance*>(pInstance);
    clients_t::iterator ite = clients.find(pClient);
    if(ite != clients.end())
        return *ite;
    return NULL;
}

ClientNode* GET_CLIENTNODE(TTInstance* pInstance)
{
    wguard_t g(clients_mutex);

    ClientInstance* pClient = static_cast<ClientInstance*>(pInstance);
    clients_t::iterator ite = clients.find(pClient);
    if(ite != clients.end())
        return (*ite)->pClientNode;
    return NULL;
}

//get ClientNode instance, lock mutex, return if not found
#define GET_CLIENTNODE_RET(pClientNode, pInstance, ret)     \
    pClientNode = GET_CLIENTNODE(pInstance);                \
    if(!pClientNode)return ret;                             \
    GUARD_REACTOR(pClientNode)

#define CLIENT ClientInstance*



TEAMTALKDLL_API const TTCHAR* TT_GetVersion()
{
    return ACE_TEXT( TEAMTALK_VERSION );
}

#if defined(WIN32)
TEAMTALKDLL_API TTInstance* TT_InitTeamTalk(IN HWND hWnd, IN UINT uMsg)
{
#if defined(USE_MINIDUMP)
    static MiniDumper mdump(ACE_TEXT("TeamTalk5.dll"));
#endif

#ifdef ENABLE_ENCRYPTION
    ACE_SSL_Context *context = ACE_SSL_Context::instance ();
    if(context->get_mode() !=  ACE_SSL_Context::SSLv23_client)
        context->set_mode(ACE_SSL_Context::SSLv23_client);
#endif

    ClientInstance* pClient = new ClientInstance;
    pClient->pEventHandler = new TTMsgQueue(hWnd, uMsg);
    pClient->pClientNode = new ClientNode(ACE_TEXT( TEAMTALK_VERSION ), 
                                          pClient->pEventHandler);

    wguard_t g(clients_mutex);
    clients.insert(pClient);

    return pClient;
}

TEAMTALKDLL_API TTBOOL TT_SwapTeamTalkHWND(IN TTInstance* lpTTInstance,
                                         IN HWND hWnd)
{
    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return FALSE;
    pClient->pEventHandler->SetHWND(hWnd);
    return TRUE;
}
#endif

TEAMTALKDLL_API TTInstance* TT_InitTeamTalkPoll()
{
#if defined(USE_MINIDUMP)
    static MiniDumper mdump(ACE_TEXT("TeamTalk4.dll"));
#endif

#ifdef ENABLE_ENCRYPTION
    ACE_SSL_Context *context = ACE_SSL_Context::instance ();
    if(context->get_mode() !=  ACE_SSL_Context::SSLv23_client)
        context->set_mode(ACE_SSL_Context::SSLv23_client);
#endif

#if !defined(WIN32)
    //avoid SIGPIPE
    static ACE_Sig_Action no_sigpipe ((ACE_SignalHandler) SIG_IGN);
    static ACE_Sig_Action original_action;
    no_sigpipe.register_action (SIGPIPE, &original_action);
#endif

    ClientInstance* pClient = new ClientInstance;
    pClient->pEventHandler = new TTMsgQueue();
    pClient->pClientNode = new ClientNode(ACE_TEXT( TEAMTALK_VERSION ), 
                                          pClient->pEventHandler);

    wguard_t g(clients_mutex);
    clients.insert(pClient);

    return pClient;
}

TEAMTALKDLL_API TTBOOL TT_CloseTeamTalk(IN TTInstance* lpTTInstance)
{
    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return FALSE;

    int ret = 0;

#if defined(WIN32)
    //make sure hotkey doesn't call us anymore
    HOTKEY->ClearAll(pClient->pEventHandler);
#endif

    //validate instance
    {
        ClientNode* pClientNode;
        GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    }

    TTASSERT(ret>=0);

    delete pClient->pClientNode;

    delete pClient->pEventHandler;

    wguard_t g(clients_mutex);
    clients.erase(pClient);
    delete pClient;

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_GetDefaultSoundDevices(OUT INT32* lpnInputDeviceID, 
                                               OUT INT32* lpnOutputDeviceID)
{
#if defined(ENABLE_SOUNDSYSTEM)
    int input, output;
    if(SOUNDSYSTEM->GetDefaultDevices(input, output))
    {
        if(lpnInputDeviceID)
            *lpnInputDeviceID = input;

        if(lpnOutputDeviceID)
            *lpnOutputDeviceID = output;

        return TRUE;
    }
#endif
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_GetDefaultSoundDevicesEx(IN SoundSystem nSndSystem, 
                                                 OUT INT32* lpnInputDeviceID, 
                                                 OUT INT32* lpnOutputDeviceID)
{
#if defined(ENABLE_SOUNDSYSTEM)
    int input, output;
    if(SOUNDSYSTEM->GetDefaultDevices((soundsystem::SoundAPI)nSndSystem, input, output))
    {
        if(lpnInputDeviceID)
            *lpnInputDeviceID = input;

        if(lpnOutputDeviceID)
            *lpnOutputDeviceID = output;

        return TRUE;
    }
#endif
    return FALSE;
}


TEAMTALKDLL_API TTBOOL TT_GetSoundDevices(IN OUT SoundDevice* pSoundDevices,
                                        IN OUT INT32* lpnHowMany)
{
    if(!lpnHowMany)
        return FALSE;

#if defined(ENABLE_SOUNDSYSTEM)
    std::vector< soundsystem::DeviceInfo > devices;
    SOUNDSYSTEM->GetSoundDevices(devices);
    if(!pSoundDevices)
    {
        *lpnHowMany = (INT32)devices.size();
        return TRUE;
    }

    size_t lessDevs = (size_t)*lpnHowMany < devices.size()?*lpnHowMany:devices.size();

    for(size_t i=0;i<lessDevs;i++)
    {
        ACE_OS::strsncpy(pSoundDevices[i].szDeviceName, 
                        devices[i].devicename.c_str(), 
                        TT_STRLEN);
        pSoundDevices[i].nDeviceID = devices[i].id;
        pSoundDevices[i].nMaxInputChannels = devices[i].max_input_channels;
        pSoundDevices[i].nMaxOutputChannels = devices[i].max_output_channels;
        pSoundDevices[i].nDefaultSampleRate = devices[i].default_samplerate;
        pSoundDevices[i].bSupports3D = devices[i].supports3d;
        pSoundDevices[i].nSoundSystem = (SoundSystem)devices[i].soundsystem;
        ACE_OS::strsncpy(pSoundDevices[i].szDeviceID, 
                        devices[i].deviceid.c_str(), 
                        TT_STRLEN);
#ifdef WIN32
        pSoundDevices[i].nWaveDeviceID = devices[i].wavedeviceid;
#endif
        set<int>::const_iterator is = devices[i].input_samplerates.begin();
        for(size_t s=0;s<TT_SAMPLERATES_MAX;s++)
        {
            if(is != devices[i].input_samplerates.end())
            {
                pSoundDevices[i].inputSampleRates[s] = *is;
                is++;
            }
            else
                pSoundDevices[i].inputSampleRates[s] = 0;
        }

        is = devices[i].output_samplerates.begin();
        for(size_t s=0;s<TT_SAMPLERATES_MAX;s++)
        {
            if(is != devices[i].output_samplerates.end())
            {
                pSoundDevices[i].outputSampleRates[s] = *is;
                is++;
            }
            else
                pSoundDevices[i].outputSampleRates[s] = 0;
        }

    }
    *lpnHowMany = (INT32)lessDevs;
    return TRUE;
#else
    return FALSE;
#endif
}

TEAMTALKDLL_API TTBOOL TT_RestartSoundSystem()
{
#if defined(ENABLE_SOUNDSYSTEM)
    return SOUNDSYSTEM->RestartSoundSystem();
#else
    return FALSE;
#endif
}

TEAMTALKDLL_API TTSoundLoop* TT_StartSoundLoopbackTest(IN INT32 nInputDeviceID, 
                                                       IN INT32 nOutputDeviceID,
                                                       IN INT32 nSampleRate,
                                                       IN INT32 nChannels,
                                                       IN TTBOOL bDuplexMode,
                                                       IN const SpeexDSP* lpSpeexDSP)
{
    bool agc_enable = false, denoise_enable = false, aec_enable = false;
    int noisesuppressdb = 0;

#if defined(ENABLE_SPEEX)
    SpeexAGC agc;
    SpeexAEC aec;
#endif

    if(lpSpeexDSP)
    {
#if defined(ENABLE_SPEEX)
        agc_enable = lpSpeexDSP->bEnableAGC;
        agc.gain_level = (float)lpSpeexDSP->nGainLevel;
        agc.max_increment = lpSpeexDSP->nMaxIncDBSec;
        agc.max_decrement = lpSpeexDSP->nMaxDecDBSec;
        agc.max_gain = lpSpeexDSP->nMaxGainDB;
        
        denoise_enable = lpSpeexDSP->bEnableDenoise;
        noisesuppressdb = lpSpeexDSP->nMaxNoiseSuppressDB;
        
        aec_enable = lpSpeexDSP->bEnableEchoCancellation;
        aec.suppress_level = lpSpeexDSP->nEchoSuppress;
        aec.suppress_active = lpSpeexDSP->nEchoSuppressActive;
#else
        if(lpSpeexDSP->bEnableAGC || lpSpeexDSP->bEnableDenoise ||
           lpSpeexDSP->bEnableEchoCancellation)
           return FALSE;
#endif
    }

#if defined(ENABLE_SOUNDSYSTEM)
    SoundLoopback* pSoundLoopBack;
    ACE_NEW_RETURN(pSoundLoopBack, SoundLoopback(), NULL);

    TTBOOL b;
    if(bDuplexMode)
    {
        b = pSoundLoopBack->StartDuplexTest(nInputDeviceID, 
                                            nOutputDeviceID, 
                                            nSampleRate, nChannels
#if defined(ENABLE_SPEEX)
                                            , agc_enable, agc,
                                            denoise_enable, 
                                            noisesuppressdb,
                                            aec_enable, 
                                            aec
#endif
                                            );
    }
    else
    {
        b = pSoundLoopBack->StartTest(nInputDeviceID, 
                                       nOutputDeviceID, 
                                       nSampleRate, nChannels
#if defined(ENABLE_SPEEX)
                                       , agc_enable, agc,
                                       denoise_enable, 
                                       noisesuppressdb,
                                       aec_enable, 
                                       aec
#endif
                                       );
    }

    if(!b)
    {
        delete pSoundLoopBack;
        return NULL;
    }
    else
    {
        wguard_t g(soundloops_mutex);
        soundloops.insert(pSoundLoopBack);
    }
    return pSoundLoopBack;
#endif
    return NULL;
}

TEAMTALKDLL_API TTBOOL TT_CloseSoundLoopbackTest(IN TTSoundLoop* lpTTSoundLoop)
{
#if defined(ENABLE_SOUNDSYSTEM)
    wguard_t g(soundloops_mutex);
    SoundLoopback* pSoundLoopBack = reinterpret_cast<SoundLoopback*>(lpTTSoundLoop);
    if(soundloops.find(pSoundLoopBack) != soundloops.end())
    {
        TTBOOL b = pSoundLoopBack->StopTest();
        delete pSoundLoopBack;
        soundloops.erase(pSoundLoopBack);
        return b;
    }
#endif
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_InitSoundDuplexDevices(IN TTInstance* lpTTInstance, 
                                               IN INT32 nInputDeviceID,
                                               IN INT32 nOutputDeviceID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->InitSoundDuplexDevices(nInputDeviceID, nOutputDeviceID);
}

TEAMTALKDLL_API TTBOOL TT_InitSoundInputDevice(IN TTInstance* lpTTInstance, 
                                             IN INT32 nInputDeviceID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->InitSoundInputDevice(nInputDeviceID);
}

TEAMTALKDLL_API TTBOOL TT_InitSoundOutputDevice(IN TTInstance* lpTTInstance, 
                                              IN INT32 nOutputDeviceID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->InitSoundOutputDevice(nOutputDeviceID);
}

TEAMTALKDLL_API TTBOOL TT_CloseSoundInputDevice(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->CloseSoundInputDevice();
}

TEAMTALKDLL_API TTBOOL TT_CloseSoundOutputDevice(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->CloseSoundOutputDevice();
}

TEAMTALKDLL_API TTBOOL TT_CloseSoundDuplexDevices(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->CloseSoundDuplexDevices();
}

TEAMTALKDLL_API INT32 TT_GetSoundInputLevel(IN TTInstance* lpTTInstance)
{
    INT32 nLevel = SOUND_VU_MIN;
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, nLevel);
    nLevel = pClientNode->GetCurrentVoiceLevel();
    return nLevel;
}

TEAMTALKDLL_API TTBOOL TT_SetSoundInputGainLevel(IN TTInstance* lpTTInstance, INT32 nLevel)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->SetVoiceGainLevel(nLevel);
    return TRUE;
}

TEAMTALKDLL_API INT32 TT_GetSoundInputGainLevel(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, SOUND_GAIN_MIN);
    return pClientNode->GetVoiceGainLevel();
}

TEAMTALKDLL_API TTBOOL TT_SetSoundInputPreprocess(IN TTInstance* lpTTInstance,
                                                const IN SpeexDSP* lpSpeexDSP)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    teamtalk::SpeexDSP spxdsp;
    Convert(*lpSpeexDSP, spxdsp);

    pClientNode->SetSoundPreprocess(spxdsp);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_GetSoundInputPreprocess(IN TTInstance* lpTTInstance,
                                                OUT SpeexDSP* lpSpeexDSP)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    teamtalk::SpeexDSP spxdsp = pClientNode->GetSoundProperties().speexdsp;
    Convert(spxdsp, *lpSpeexDSP);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_SetSoundOutputVolume(IN TTInstance* lpTTInstance, INT32 nVolume)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->SetSoundOutputVolume(nVolume);
}

TEAMTALKDLL_API INT32 TT_GetSoundOutputVolume(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, SOUND_VOLUME_MIN);
    return pClientNode->GetSoundOutputVolume();
}

TEAMTALKDLL_API TTBOOL TT_SetSoundOutputMute(IN TTInstance* lpTTInstance, TTBOOL bMuteAll)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->MuteAll(bMuteAll);
}

TEAMTALKDLL_API TTBOOL TT_EnableVoiceTransmission(IN TTInstance* lpTTInstance,
                                                IN TTBOOL bEnable)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->EnableVoiceTransmission(bEnable);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_EnableVoiceActivation(IN TTInstance* lpTTInstance,
                                              TTBOOL bEnable)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->EnableVoiceActivation(bEnable);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_SetVoiceActivationLevel(IN TTInstance* lpTTInstance, 
                                                IN INT32 nLevel)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(nLevel >= SOUND_VU_MIN || nLevel <= SOUND_VU_MAX)
    {
        pClientNode->SetVoiceActivationLevel(nLevel);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API INT32 TT_GetVoiceActivationLevel(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, SOUND_VU_MIN);

    return pClientNode->GetVoiceActivationLevel();
}

TEAMTALKDLL_API TTBOOL TT_SetVoiceActivationStopDelay(IN TTInstance* lpTTInstance,
                                                    IN INT32 nDelayMSec)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->SetVoiceActivationStoppedDelay(nDelayMSec);
    return TRUE;
}

TEAMTALKDLL_API INT32 TT_GetVoiceActivationStopDelay(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->GetVoiceActivationStoppedDelay();
}

TEAMTALKDLL_API TTBOOL TT_Enable3DSoundPositioning(IN TTInstance* lpTTInstance, 
                                              IN TTBOOL bEnable)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->EnableAutoPositioning(bEnable);
}

TEAMTALKDLL_API TTBOOL TT_AutoPositionUsers(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->AutoPositionUsers();
}

TEAMTALKDLL_API TTBOOL TT_EnableAudioBlockEvent(IN TTInstance* lpTTInstance,
                                              IN INT32 nUserID,
                                              IN StreamType nStreamType,
                                              IN TTBOOL bEnable)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    
    pClientNode->EnableAudioBlockCallback(nUserID, (teamtalk::StreamType)nStreamType, bEnable);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_StartRecordingMuxedAudioFile(IN TTInstance* lpTTInstance,
                                                     IN const AudioCodec* lpAudioCodec,
                                                     IN const TTCHAR* szAudioFileName,
                                                     IN AudioFileFormat uAFF)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    
    teamtalk::AudioCodec codec;
    if(!lpAudioCodec || !Convert(*lpAudioCodec, codec))
        return FALSE;

    return pClientNode->StartRecordingMuxedAudioFile(codec, szAudioFileName, 
                                                     (teamtalk::AudioFileFormat)uAFF);
}

TEAMTALKDLL_API TTBOOL TT_StopRecordingMuxedAudioFile(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    
    pClientNode->StopRecordingMuxedAudioFile();
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_StartVideoCaptureTransmission(IN TTInstance* lpTTInstance,
                                                      IN const VideoCodec* lpVideoCodec)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    teamtalk::VideoCodec codec;
    Convert(*lpVideoCodec, codec);
    if(lpVideoCodec)
        Convert(*lpVideoCodec, codec);
    else
        codec.codec = teamtalk::CODEC_NO_CODEC;

    return pClientNode->OpenVideoCaptureSession(codec);
}

TEAMTALKDLL_API TTBOOL TT_StopVideoCaptureTransmission(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    pClientNode->CloseVideoCaptureSession();
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_GetVideoCaptureDevices(IN OUT VideoCaptureDevice* lpVideoDevices,
                                               IN OUT INT32* lpnHowMany)
{
    if(!lpnHowMany)
        return FALSE;

#if defined(ENABLE_VIDCAP)
    vidcap_devices_t devs = VIDCAP->GetDevices();
    if(!lpVideoDevices)
    {
        *lpnHowMany = (INT32)devs.size();
        return TRUE;
    }
    size_t lessDevs = (size_t)*lpnHowMany < devs.size()?*lpnHowMany:devs.size();
    for(size_t i=0;i<lessDevs;i++)
    {
        ACE_OS::strsncpy(lpVideoDevices[i].szCaptureAPI, 
                         devs[i].api.c_str(),
                         TT_STRLEN);
        ACE_OS::strsncpy(lpVideoDevices[i].szDeviceName, 
                         devs[i].devicename.c_str(), 
                         TT_STRLEN);
        ACE_OS::strsncpy(lpVideoDevices[i].szDeviceID, 
                         devs[i].deviceid.c_str(), 
                         TT_STRLEN);

        memset(lpVideoDevices[i].videoFormats, 0, sizeof(lpVideoDevices->videoFormats));
        size_t lessFormats = sizeof(lpVideoDevices->videoFormats)/sizeof(lpVideoDevices->videoFormats[0]);
        TTASSERT(lessFormats == TT_VIDEOFORMATS_MAX);
        lessFormats = lessFormats<devs[i].vidcapformats.size()?lessFormats:devs[i].vidcapformats.size();
        lpVideoDevices[i].nVideoFormatsCount = (INT32)lessFormats;
        for(size_t j=0;j<lessFormats;j++)
        {
            lpVideoDevices[i].videoFormats[j].nWidth = devs[i].vidcapformats[j].width;
            lpVideoDevices[i].videoFormats[j].nHeight = devs[i].vidcapformats[j].height;
            lpVideoDevices[i].videoFormats[j].nFPS_Numerator = devs[i].vidcapformats[j].fps_numerator;
            lpVideoDevices[i].videoFormats[j].nFPS_Denominator = devs[i].vidcapformats[j].fps_denominator;
            lpVideoDevices[i].videoFormats[j].picFourCC = (FourCC)devs[i].vidcapformats[j].fourcc;
        }
    }
    *lpnHowMany = (INT32)lessDevs;
    return TRUE;
#else
    return FALSE;
#endif
}

TEAMTALKDLL_API TTBOOL TT_InitVideoCaptureDevice(IN TTInstance* lpTTInstance,
                                               IN const TTCHAR* szDeviceID,
                                               IN const VideoFormat* lpVideoFormat)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!lpVideoFormat)
        return FALSE;

    media::VideoFormat cap_format;
    Convert(*lpVideoFormat, cap_format);

    return pClientNode->InitVideoCapture(szDeviceID, cap_format);
}

TEAMTALKDLL_API TTBOOL TT_CloseVideoCaptureDevice(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->CloseVideoCapture();
    return TRUE;
}

#ifdef WIN32
TEAMTALKDLL_API TTBOOL TT_PaintVideoFrame(IN HDC hDC,
                                        IN INT32 XDest,
                                        IN INT32 YDest,
                                        IN INT32 nDestWidth,
                                        IN INT32 nDestHeight,
                                        IN VideoFrame* lpVideoFrame)
{
    if(!lpVideoFrame)
        return FALSE;

    TTASSERT(RGB32_BYTES(lpVideoFrame->nWidth, lpVideoFrame->nHeight) == lpVideoFrame->nFrameBufferSize);

    if(RGB32_BYTES(lpVideoFrame->nWidth, lpVideoFrame->nHeight) != lpVideoFrame->nFrameBufferSize)
        return FALSE;

    BITMAPINFOHEADER bmh = {0};
    BITMAPINFO bmi = {0};
    bmh.biSize = sizeof(bmh);
    bmh.biWidth = lpVideoFrame->nWidth;
    bmh.biHeight = lpVideoFrame->nHeight*-1; //flip image (top down)
    bmh.biPlanes = 1;
    bmh.biBitCount = 32;
    bmh.biCompression = BI_RGB;
    bmh.biSizeImage = 0;
    bmh.biXPelsPerMeter = 0;
    bmh.biYPelsPerMeter = 0;
    bmh.biClrUsed = 0;
    bmh.biClrImportant = 0;
    bmi.bmiHeader = bmh;
    BOOL b = StretchDIBits(hDC, XDest, YDest, nDestWidth, nDestHeight, 0, 0, 
                           bmh.biWidth, bmh.biHeight*-1, lpVideoFrame->frameBuffer, 
                           &bmi, DIB_RGB_COLORS, SRCCOPY);
    TTASSERT(b);
    return b;
}

TEAMTALKDLL_API TTBOOL TT_PaintVideoFrameEx(IN HDC hDC,
                                          IN INT32 XDest,
                                          IN INT32 YDest,
                                          IN INT32 nDestWidth,
                                          IN INT32 nDestHeight,
                                          IN INT32 XSrc,
                                          IN INT32 YSrc,
                                          IN INT32 nSrcWidth,
                                          IN INT32 nSrcHeight,
                                          IN VideoFrame* lpVideoFrame)
{
    if(!lpVideoFrame ||
       XSrc + nSrcWidth > lpVideoFrame->nWidth || 
       YSrc + nSrcHeight > lpVideoFrame->nHeight)
        return FALSE;

    TTASSERT(RGB32_BYTES(lpVideoFrame->nWidth, lpVideoFrame->nHeight) == lpVideoFrame->nFrameBufferSize);
    if(RGB32_BYTES(lpVideoFrame->nWidth, lpVideoFrame->nHeight) != lpVideoFrame->nFrameBufferSize)
        return FALSE;

    BITMAPINFOHEADER bmh = {0};
    BITMAPINFO bmi = {0};
    bmh.biSize = sizeof(bmh);
    bmh.biWidth = lpVideoFrame->nWidth;
    bmh.biHeight = lpVideoFrame->nHeight*-1; //flip image
    bmh.biPlanes = 1;
    bmh.biBitCount = 32;
    bmh.biCompression = BI_RGB;
    bmh.biSizeImage = 0;
    bmh.biXPelsPerMeter = 0;
    bmh.biYPelsPerMeter = 0;
    bmh.biClrUsed = 0;
    bmh.biClrImportant = 0;
    bmi.bmiHeader = bmh;
    BOOL b = StretchDIBits(hDC, XDest, YDest, nDestWidth, nDestHeight, XSrc,
                           YSrc, nSrcWidth, nSrcHeight, lpVideoFrame->frameBuffer, 
                           &bmi, DIB_RGB_COLORS, SRCCOPY);
    TTASSERT(b);
    return b;
}
#endif

TEAMTALKDLL_API VideoFrame* TT_AcquireUserVideoCaptureFrame(IN TTInstance* lpTTInstance,
                                                            IN INT32 nUserID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return NULL;

    ACE_Message_Block* mb;

    if(nUserID == 0)
    {
        mb = pClientNode->AcquireVideoCaptureFrame();
        if(!mb)
            return NULL;
    }
    else
    {
        //try and extract new one
        clientuser_t user = pClientNode->GetUser(nUserID);
        if(user.null())
            return NULL;
        g.release(); //don't hold lock while decoding
        mb = user->GetVideoCaptureFrame();
        g.acquire();
    }

    if(mb)
    {
        VideoFrame* lpVideoFrame = inst->PushVideoFrame(mb);
        if(lpVideoFrame)
        {
            media::VideoFrame* frm = reinterpret_cast<media::VideoFrame*>(mb->rd_ptr());
            Convert(*frm, *lpVideoFrame);
            return lpVideoFrame;
        }
        //else 'mb' is deleted by PushVideoFrame()
    }
    return NULL;
}

TEAMTALKDLL_API TTBOOL TT_ReleaseUserVideoCaptureFrame(IN TTInstance* lpTTInstance,
                                                     IN VideoFrame* lpVideoFrame)
{
    if(!lpVideoFrame)
        return FALSE;

    ClientNode* pClientNode; 
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return FALSE;
    return inst->RemoveVideoFrame(lpVideoFrame);
}

TEAMTALKDLL_API ClientFlags TT_GetFlags(IN TTInstance* lpTTInstance)
{    
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, ::CLIENT_CLOSED);
    return (ClientFlags)pClientNode->GetFlags();
}

TEAMTALKDLL_API TTBOOL TT_SetLicenseInformation(IN const TTCHAR szRegName[TT_STRLEN],
                                              IN const TTCHAR szRegKey[TT_STRLEN])
{
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_Connect(IN TTInstance* lpTTInstance,
                                  IN const TTCHAR* szHostAddress, 
                                  IN INT32 nTcpPort, 
                                  IN INT32 nUdpPort, 
                                  IN INT32 nLocalTcpPort, 
                                  IN INT32 nLocalUdpPort,
                                  IN TTBOOL bEncrypted)
{
    return TT_ConnectSysID(lpTTInstance, szHostAddress, nTcpPort, nUdpPort, 
                           nLocalTcpPort, nLocalUdpPort, bEncrypted, SERVER_WELCOME);
}

TEAMTALKDLL_API TTBOOL TT_ConnectSysID(IN TTInstance* lpTTInstance,
                                       IN const TTCHAR* szHostAddress, 
                                       IN INT32 nTcpPort, 
                                       IN INT32 nUdpPort, 
                                       IN INT32 nLocalTcpPort, 
                                       IN INT32 nLocalUdpPort,
                                       IN TTBOOL bEncrypted,
                                       IN const TTCHAR* szSystemID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!szHostAddress)
        return FALSE;

#if defined(ENABLE_ENCRYPTION)
    return pClientNode->Connect(bEncrypted, szHostAddress, nTcpPort, nUdpPort, 
                                szSystemID, ACE_TEXT(""), nLocalTcpPort, 
                                nLocalUdpPort);
#else
    return pClientNode->Connect(false, szHostAddress, nTcpPort, nUdpPort, 
                                szSystemID, ACE_TEXT(""), nLocalTcpPort, 
                                nLocalUdpPort);
#endif
}

TEAMTALKDLL_API TTBOOL TT_ConnectEx(IN TTInstance* lpTTInstance,
                                    IN const TTCHAR* szHostAddress,
                                    IN INT32 nTcpPort,
                                    IN INT32 nUdpPort,
                                    IN const TTCHAR* szBindIPAddr,
                                    IN INT32 nLocalTcpPort,
                                    IN INT32 nLocalUdpPort,
                                    IN TTBOOL bEncrypted)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(!szHostAddress || !szBindIPAddr)
        return FALSE;

#if defined(ENABLE_ENCRYPTION)
    return pClientNode->Connect(bEncrypted, szHostAddress, nTcpPort, nUdpPort, 
                                SERVER_WELCOME, szBindIPAddr, nLocalTcpPort, 
                                nLocalUdpPort);
#else
    return pClientNode->Connect(false, szHostAddress, nTcpPort, nUdpPort, 
                                SERVER_WELCOME, szBindIPAddr, nLocalTcpPort, 
                                nLocalUdpPort);
#endif
}



TEAMTALKDLL_API TTBOOL TT_Disconnect(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->Disconnect();
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_QueryMaxPayload(IN TTInstance* lpTTInstance,
                                        IN INT32 nUserID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    return pClientNode->StartMTUQuery();
}

TEAMTALKDLL_API TTBOOL TT_GetClientStatistics(IN TTInstance* lpTTInstance,
                                      OUT ClientStatistics* lpClientStatistics)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!lpClientStatistics)
        return FALSE;

    teamtalk::ClientStats stats;
    if(pClientNode->GetClientStatistics(stats))
    {
        Convert(stats, *lpClientStatistics);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API INT32 TT_DoPing(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    return pClientNode->DoPing(true);
}

TEAMTALKDLL_API INT32 TT_DoLogin(IN TTInstance* lpTTInstance,
                                 IN const TTCHAR* szNickname, 
                                 IN const TTCHAR* szUsername,
                                 IN const TTCHAR* szPassword)
{
    return TT_DoLoginEx(lpTTInstance, szNickname, szUsername, 
                        szPassword, ACE_TEXT(""));
}

TEAMTALKDLL_API INT32 TT_DoLoginEx(IN TTInstance* lpTTInstance,
                                   IN const TTCHAR* szNickname, 
                                   IN const TTCHAR* szUsername,
                                   IN const TTCHAR* szPassword,
                                   IN const TTCHAR* szClientName)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(szNickname && szUsername && szPassword && szClientName)
        return pClientNode->DoLogin(szNickname, szUsername, 
                                    szPassword, szClientName);
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoLogout(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    return pClientNode->DoLogout();
}

TEAMTALKDLL_API INT32 TT_DoJoinChannel(IN TTInstance* lpTTInstance,
                                       IN const Channel* lpChannel)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    teamtalk::ChannelProp prop;
    if(!lpChannel || !Convert(*lpChannel, prop))
        return -1;

    return pClientNode->DoJoinChannel(prop);
}

TEAMTALKDLL_API INT32 TT_DoJoinChannelByID(IN TTInstance* lpTTInstance,
                                           IN INT32 nChannelID, 
                                           IN const TTCHAR* szPassword)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    if(szPassword)
    {
        teamtalk::ChannelProp prop;
        prop.channelid = nChannelID;
        prop.passwd = szPassword;
        return pClientNode->DoJoinChannel(prop);
    }
    return -1;
}


TEAMTALKDLL_API INT32 TT_DoLeaveChannel(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    return pClientNode->DoLeaveChannel();
}

TEAMTALKDLL_API INT32 TT_DoChangeNickname(IN TTInstance* lpTTInstance,
                                      IN const TTCHAR* szNewNick)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    if( szNewNick )
        return pClientNode->DoChangeNickname(szNewNick);
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoChangeStatus(IN TTInstance* lpTTInstance,
                                        IN INT32 nStatusMode, 
                                        IN const TTCHAR* szStatusMessage)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(szStatusMessage)
        return pClientNode->DoChangeStatus(nStatusMode, szStatusMessage);
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoTextMessage(IN TTInstance* lpTTInstance,
                                       IN const TextMessage* lpTextMessage)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(lpTextMessage)
    {
        teamtalk::TextMessage msg;
        Convert(*lpTextMessage, msg);
        return pClientNode->DoTextMessage(msg);
    }
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoChannelOp(IN TTInstance* lpTTInstance,
                                     IN INT32 nUserID,
                                     IN INT32 nChannelID,
                                     IN TTBOOL bMakeOperator)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoChannelOperator(nUserID, nChannelID, ACE_TString(), 
                                          bMakeOperator);
}

TEAMTALKDLL_API INT32 TT_DoChannelOpEx(IN TTInstance* lpTTInstance,
                                       IN INT32 nUserID,
                                       IN INT32 nChannelID,
                                       IN const TTCHAR* szOpPassword,
                                       IN TTBOOL bMakeOperator)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    if(!szOpPassword)
        return -1;
    return pClientNode->DoChannelOperator(nUserID, nChannelID, 
                                          szOpPassword,
                                          bMakeOperator);
}

TEAMTALKDLL_API INT32 TT_DoKickUser(IN TTInstance* lpTTInstance,
                                    IN INT32 nUserID,
                                    IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    return pClientNode->DoKickUser(nUserID, nChannelID);
}

TEAMTALKDLL_API INT32 TT_DoSendFile(IN TTInstance* lpTTInstance,
                                    IN INT32 nChannelID,
                                    IN const TTCHAR* szLocalFilePath)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(szLocalFilePath && ACE_OS::filesize(szLocalFilePath)>=0)
        return pClientNode->DoFileSend(nChannelID, szLocalFilePath);
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoRecvFile(IN TTInstance* lpTTInstance,
                                    IN INT32 nChannelID,
                                    IN INT32 nFileID, 
                                    IN const TTCHAR* szLocalFilePath)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(szLocalFilePath)
    {
        clientchannel_t chan = pClientNode->GetChannel(nChannelID);
        teamtalk::RemoteFile remotefile;
        if(!chan.null() && chan->GetFile(nFileID, remotefile))
            return pClientNode->DoFileRecv(nChannelID, szLocalFilePath,
                                           remotefile.filename);
    }
    return -1;

}

TEAMTALKDLL_API INT32 TT_DoDeleteFile(IN TTInstance* lpTTInstance, 
                                      IN INT32 nChannelID,
                                      IN INT32 nFileID)                                          

{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    clientchannel_t chan = pClientNode->GetChannel(nChannelID);
    teamtalk::RemoteFile remotefile;
    if(!chan.null() && chan->GetFile(nFileID, remotefile))
        return pClientNode->DoFileDelete(nChannelID, remotefile.filename);
    return -1;
}

TEAMTALKDLL_API TTBOOL TT_GetServerProperties(IN TTInstance* lpTTInstance,
                                            OUT ServerProperties* lpProperties)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    teamtalk::ServerInfo prop;
    if(pClientNode->GetServerInfo(prop) && lpProperties)
    {
        Convert(prop, *lpProperties);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_GetServerUsers(IN TTInstance* lpTTInstance,
                                       IN OUT User* lpUsers,
                                       IN OUT INT32* lpnHowMany)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!lpnHowMany)
        return FALSE;

    set<int> userids;
    pClientNode->GetUsers(userids);
    if(!lpUsers)
    {
        *lpnHowMany = (INT32)userids.size();
        return TRUE;
    }
    set<int>::const_iterator ite = userids.begin();
    int i = 0;
    while(ite != userids.end() && i < *lpnHowMany)
    {
        clientuser_t user = pClientNode->GetUser(*ite);
        TTASSERT(!user.null());
        if(!user.null())
            Convert(*user, lpUsers[i++]);
        ite++;
    }
    *lpnHowMany = i;
    return TRUE;
}


TEAMTALKDLL_API INT32 TT_GetRootChannelID(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, 0);

    clientchannel_t channel = pClientNode->GetRootChannel();
    if(!channel.null())
        return channel->GetChannelID();

    return 0;
}

TEAMTALKDLL_API INT32 TT_GetMyChannelID(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, 0);

    return pClientNode->GetChannelID();
}

TEAMTALKDLL_API TTBOOL TT_GetChannel(IN TTInstance* lpTTInstance,
                                   IN INT32 nChannelID, 
                                   OUT Channel* lpChannel)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    teamtalk::ChannelProp chanprop;
    if(pClientNode->GetChannelProp(nChannelID, chanprop))
    {
        return Convert(chanprop, *lpChannel);
    }

    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_GetChannelPath(IN TTInstance* lpTTInstance,
                                       IN INT32 nChannelID, 
                                       OUT TTCHAR szChannelPath[TT_STRLEN])
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(szChannelPath)
    {
        clientchannel_t channel = pClientNode->GetChannel(nChannelID);
        if(!channel.null())
        {
            ACE_OS::strsncpy(szChannelPath, channel->GetChannelPath().c_str(), TT_STRLEN);
            return TRUE;
        }
    }
    return FALSE;
}

TEAMTALKDLL_API INT32 TT_GetChannelIDFromPath(IN TTInstance* lpTTInstance,
                                            IN const TTCHAR* szChannelPath)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(szChannelPath)
    {
        clientchannel_t channel = ChangeChannel(pClientNode->GetRootChannel(), 
                                                szChannelPath);
        if(!channel.null())
            return channel->GetChannelID();
    }
    return 0;
}

TEAMTALKDLL_API TTBOOL TT_GetChannelUsers(IN TTInstance* lpTTInstance,
                                        IN INT32 nChannelID,
                                        IN OUT User* lpUsers,
                                        IN OUT INT32* lpnHowMany)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientchannel_t chan = pClientNode->GetChannel(nChannelID);
    if(chan.null())
        return FALSE;

    if(lpUsers == NULL)
    {
        if(lpnHowMany)
        {
            *lpnHowMany = chan->GetUsersCount();
            return TRUE;
        }
        return FALSE;
    }

    if(!lpnHowMany)
        return FALSE;

    int less = (chan->GetUsersCount()>*lpnHowMany)?*lpnHowMany:chan->GetUsersCount();
    const ClientChannel::users_t& users = chan->GetUsers();
    for(int i=0;i<less;i++)
        Convert(*users[i], lpUsers[i]);
    *lpnHowMany = less;

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_SetUserMediaStorageDir(IN TTInstance* lpTTInstance,
                                                 IN INT32 nUserID,
                                                 IN const TTCHAR* szFolderPath,
                                                 IN const TTCHAR* szFileNameVars,
                                                 IN AudioFileFormat uAFF)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID, TRUE);
    if(!user.null())
    {
        if(!szFolderPath)
            szFolderPath = ACE_TEXT("");
        if(!szFileNameVars)
            szFileNameVars = ACE_TEXT("");

        user->SetAudioFolder(szFolderPath);
        user->SetAudioFileVariables(szFileNameVars);
        user->SetAudioFileFormat((teamtalk::AudioFileFormat)uAFF);
        return TRUE;
    }

    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_SetUserAudioStreamBufferSize(IN TTInstance* lpTTInstance,
                                                     IN INT32 nUserID,
                                                     IN StreamTypes uStreamType,
                                                     IN INT32 nMSec)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!user.null())
    {
        user->SetAudioStreamBufferSize((teamtalk::StreamType)uStreamType, nMSec);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API AudioBlock* TT_AcquireUserAudioBlock(IN TTInstance* lpTTInstance,
                                                     IN StreamType nStreamType,
                                                     IN INT32 nUserID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, NULL);

    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return NULL;

    const SoundProperties& prop = pClientNode->GetSoundProperties();

    RawAudio aud;
    ACE_Message_Block* mb = AUDIOCONTAINER::instance()->AcquireRawAudio(prop.soundgroupid,
                                                                        nUserID, 
                                                                        nStreamType, aud);
    if(!mb)
        return NULL;

    AudioBlock* lpAudioBlock = inst->PushAudioBlock(mb);
    lpAudioBlock->nStreamID = aud.stream_id;
    lpAudioBlock->nSampleRate = aud.samplerate;
    lpAudioBlock->nChannels = aud.channels;
    lpAudioBlock->lpRawAudio = aud.rawAudio;
    lpAudioBlock->nSamples = aud.samples;
    lpAudioBlock->uSampleIndex = aud.start_sample_no;

    return lpAudioBlock;
}

TEAMTALKDLL_API TTBOOL TT_ReleaseUserAudioBlock(IN TTInstance* lpTTInstance,
                                              IN AudioBlock* lpAudioBlock)
{
    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return FALSE;

    return inst->RemoveAudioBlock(lpAudioBlock);
}

TEAMTALKDLL_API TTBOOL TT_GetUser(IN TTInstance* lpTTInstance,
                                IN INT32 nUserID, 
                                OUT User* lpUser )
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!lpUser)
        return FALSE;

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(user.null())
        return FALSE;

    Convert(*user, *lpUser);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_GetUserStatistics(IN TTInstance* lpTTInstance,
                                          IN INT32 nUserID, 
                                          OUT UserStatistics* lpStats)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!lpStats || user.null())
        return FALSE;
    Convert(user->GetStatistics(), *lpStats);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_GetUserByUsername(IN TTInstance* lpTTInstance,
                                      IN const TTCHAR* szUsername, 
                                      OUT User* lpUser)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!szUsername || !lpUser)
        return FALSE;

    clientuser_t user = pClientNode->GetUserByUsername(szUsername);
    if(user.get())
        return TT_GetUser(lpTTInstance, user->GetUserID(), lpUser);
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_IsChannelOperator(IN TTInstance* lpTTInstance,
                                          IN INT32 nUserID, 
                                          IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientchannel_t chan = pClientNode->GetChannel(nChannelID);
    if(!chan.null())
        return chan->IsOperator(nUserID);

    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_GetServerChannels(IN TTInstance* lpTTInstance,
                                          IN OUT Channel* lpChannels,
                                          IN OUT INT32* lpnHowMany)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(!lpnHowMany)
        return FALSE;

    std::vector<teamtalk::ChannelProp> result;

    clientchannel_t chan = pClientNode->GetRootChannel();
    if(!chan.null())
    {
        queue<clientchannel_t> channels;
        channels.push(chan);
        while(channels.size())
        {
            chan = channels.front();
            channels.pop();
            result.push_back(chan->GetChannelProp());
            ClientChannel::channels_t subs = chan->GetSubChannels();
            for(size_t i=0;i<subs.size();i++)
                channels.push(subs[i]);
        }
    }

    if(!lpChannels)
        *lpnHowMany = INT32(result.size());
    else
    {
        int nMin = (*lpnHowMany < (INT32)result.size())? *lpnHowMany : INT32(result.size());
        *lpnHowMany = nMin;
        for(int i=0;i<nMin;i++)
            Convert(result[i], lpChannels[i]);
    }
    return TRUE;
}

TEAMTALKDLL_API INT32 TT_GetMyUserID(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->GetUserID();
}

TEAMTALKDLL_API TTBOOL TT_GetMyUserAccount(IN TTInstance* lpTTInstance,
                                         OUT UserAccount* lpUserAccount)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    if(!lpUserAccount)
        return FALSE;

    const teamtalk::UserAccount& account = pClientNode->GetMyUserAccount();
    Convert(account, *lpUserAccount);

    return TRUE;
}

TEAMTALKDLL_API UserRights TT_GetMyUserRights(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    const teamtalk::UserAccount& account = pClientNode->GetMyUserAccount();
    return account.userrights;
}


TEAMTALKDLL_API UserTypes TT_GetMyUserType(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, USERTYPE_NONE);
    const teamtalk::UserAccount& account = pClientNode->GetMyUserAccount();
    return account.usertype;
}

TEAMTALKDLL_API INT32 TT_GetMyUserData(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, 0);
    const teamtalk::UserAccount& account = pClientNode->GetMyUserAccount();
    return account.userdata;
}

TEAMTALKDLL_API TTBOOL TT_SetUserVolume(IN TTInstance* lpTTInstance,
                                      INT32 nUserID, 
                                      IN StreamType nStreamType,
                                      INT32 nVolume)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    nVolume = std::max(nVolume, (INT32)SOUND_VOLUME_MIN);
    nVolume = std::min(nVolume, (INT32)SOUND_VOLUME_MAX);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!user.null())
    {
        user->SetVolume((teamtalk::StreamType)nStreamType, nVolume);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_SetUserMute(IN TTInstance* lpTTInstance,
                                    IN INT32 nUserID, 
                                    IN StreamType nStreamType,
                                    IN TTBOOL bMute)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!user.null())
    {
        user->SetMute((teamtalk::StreamType)nStreamType, bMute);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_SetUserStoppedPlaybackDelay(IN TTInstance* lpTTInstance,
                                                    IN INT32 nUserID, 
                                                    IN StreamType nStreamType,
                                                    IN INT32 nDelayMSec)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!user.null())
    {
        user->SetPlaybackStoppedDelay((teamtalk::StreamType)nStreamType, nDelayMSec);
        return TRUE;
    }
    return FALSE;
}

/// Set the position of a user
TEAMTALKDLL_API TTBOOL TT_SetUserPosition(IN TTInstance* lpTTInstance,
                                        IN INT32 nUserID, 
                                        IN StreamType nStreamType,
                                        IN float x, 
                                        IN float y, 
                                        IN float z)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!user.null())
    {
        user->SetPosition((teamtalk::StreamType)nStreamType, x, y, z);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_SetUserStereo(IN TTInstance* lpTTInstance,
                                      IN INT32 nUserID,
                                      IN StreamType nStreamType,
                                      IN TTBOOL bLeftSpeaker, 
                                      IN TTBOOL bRightSpeaker)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(!user.null())
    {
        user->SetStereo((teamtalk::StreamType)nStreamType, bLeftSpeaker, bRightSpeaker);
        return TRUE;
    }

    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_StartStreamingMediaFileToChannel(IN TTInstance* lpTTInstance,
                                                         IN const TTCHAR* szMediaFilePath,
                                                         IN const VideoCodec* lpVideoCodec)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(!szMediaFilePath || !lpVideoCodec)
        return FALSE;

    teamtalk::VideoCodec vid_codec;
    Convert(*lpVideoCodec, vid_codec);

    return pClientNode->StartStreamingMediaFile(szMediaFilePath, vid_codec);
}

TEAMTALKDLL_API TTBOOL TT_StopStreamingMediaFileToChannel(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);
    pClientNode->StopStreamingMediaFile();
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_GetMediaFileInfo(IN const TTCHAR* szMediaFilePath,
                                         OUT MediaFileInfo* lpMediaFileInfo)
{
    if(!szMediaFilePath || !lpMediaFileInfo)
        return FALSE;

    MediaFileProp prop;
    if(GetMediaFileProp(szMediaFilePath, prop))
    {
        Convert(prop, *lpMediaFileInfo);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API VideoFrame* TT_AcquireUserMediaVideoFrame(IN TTInstance* lpTTInstance,
                                                          IN INT32 nUserID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return NULL;

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(user.null())
        return NULL;

    g.release(); //don't hold lock while decoding
    
    ACE_Message_Block* mb = user->GetVideoFileFrame();
    if(!mb)
        return NULL;

    g.acquire();
    
    VideoFrame* lpVideoFrame = inst->PushVideoFrame(mb);
    if(lpVideoFrame)
    {
        media::VideoFrame* frm = reinterpret_cast<media::VideoFrame*>(mb->rd_ptr());
        Convert(*frm, *lpVideoFrame);
        return lpVideoFrame;
    }
    //else 'mb' is deleted by PushVideoFrame()
    return NULL;
}

TEAMTALKDLL_API TTBOOL TT_ReleaseUserMediaVideoFrame(IN TTInstance* lpTTInstance,
                                                     IN VideoFrame* lpVideoFrame)
{
    if(!lpVideoFrame)
        return FALSE;

    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return FALSE;

    return inst->RemoveVideoFrame(lpVideoFrame);
}

int ConvertBitmap(const DesktopWindow& src_wnd, BitmapFormat outputFormat,
                  vector<char>& in_out_bmp, vector<char>& optional_out_bmp)
{
    size_t bmp_size = -1;
    switch(src_wnd.bmpFormat) //src format
    {
    default :
        assert(0);
        return -1;
    case BMP_RGB8_PALETTE :
    {
        teamtalk::DesktopSession src_session = 
            teamtalk::MakeDesktopSession(
                src_wnd.nWidth, src_wnd.nHeight, teamtalk::BMP_RGB8_PALETTE, 
                src_wnd.nBytesPerLine);

        switch(outputFormat)
        {
        case BMP_RGB8_PALETTE : //BMP_RGB8_PALETTE -> BMP_RGB8_PALETTE
        case BMP_RGB16_555 : //BMP_RGB8_PALETTE -> BMP_RGB16_555
        case BMP_RGB24 : //BMP_RGB8_PALETTE -> BMP_RGB24
        case BMP_RGB32 : //BMP_RGB8_PALETTE -> BMP_RGB32
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             (teamtalk::RGBMode)outputFormat);
            if(src_session.GetBitmapSize() != dst_session.GetBitmapSize())
            {
                optional_out_bmp.resize(dst_session.GetBitmapSize());
                size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                           src_session, 
                                                           optional_out_bmp, 
                                                           dst_session);
                TTASSERT(optional_out_bmp.size() == bmp_write);
                bmp_size = INT32(optional_out_bmp.size());
            }
            else
            {
                TTASSERT((size_t)src_session.GetBitmapSize() == in_out_bmp.size());
                bmp_size = INT32(in_out_bmp.size());
            }
        }
        break;
        }
    }
    break;
    case BMP_RGB16_555 :
    {
        teamtalk::DesktopSession src_session = 
            teamtalk::MakeDesktopSession(
                src_wnd.nWidth, src_wnd.nHeight, teamtalk::BMP_RGB16_555, 
                src_wnd.nBytesPerLine);

        switch(outputFormat)
        {
        case BMP_RGB8_PALETTE : //RGB16 -> BMP_RGB8_PALETTE
        case BMP_RGB16_555 :  //RGB16 -> RGB16
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             (teamtalk::RGBMode)outputFormat);
            if(src_session.GetBitmapSize() != dst_session.GetBitmapSize())
            {
                optional_out_bmp.resize(dst_session.GetBitmapSize());
                size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                           src_session, 
                                                           optional_out_bmp, 
                                                           dst_session);
                TTASSERT(optional_out_bmp.size() == bmp_write);
                bmp_size = INT32(optional_out_bmp.size());
            }
            else
            {
                TTASSERT((size_t)src_session.GetBitmapSize() == in_out_bmp.size());
                bmp_size = INT32(in_out_bmp.size());
            }
        }
        break;
        case BMP_RGB24 : //RGB16 -> RGB24
        case BMP_RGB32 : //RGB16 -> RGB32
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             (teamtalk::RGBMode)outputFormat);
            optional_out_bmp.resize(dst_session.GetBitmapSize());
            size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp, 
                                                       src_session, 
                                                       optional_out_bmp, 
                                                       dst_session);
            TTASSERT(optional_out_bmp.size() == bmp_write);
            bmp_size = INT32(optional_out_bmp.size());
        }
        break;
        default :
            return -1;
        }
    }
    break;
    case BMP_RGB24 :
    {
        teamtalk::DesktopSession src_session = 
            teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                         src_wnd.nHeight, 
                                         teamtalk::BMP_RGB24,
                                         src_wnd.nBytesPerLine);
        switch(outputFormat)
        {
        case BMP_RGB8_PALETTE :  //RGB24 -> BMP_RGB8_PALETTE
        case BMP_RGB16_555 : //RGB24 -> RGB16
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight,
                                             (teamtalk::RGBMode)outputFormat);

            size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                       src_session,
                                                       in_out_bmp,
                                                       dst_session);
            TTASSERT((size_t)dst_session.GetBitmapSize() == bmp_write);

            in_out_bmp.resize(dst_session.GetBitmapSize());
            bmp_size = INT32(in_out_bmp.size());
        }
        break;
        case BMP_RGB24 : //RGB24 -> RGB24
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             teamtalk::BMP_RGB24);
            if(src_session.GetBitmapSize() != dst_session.GetBitmapSize())
            {
                optional_out_bmp.resize(dst_session.GetBitmapSize());
                size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                           src_session, 
                                                           optional_out_bmp, 
                                                           dst_session);
                TTASSERT(optional_out_bmp.size() == bmp_write);
                bmp_size = INT32(optional_out_bmp.size());
            }
            else
            {
                TTASSERT((size_t)src_session.GetBitmapSize() == in_out_bmp.size());
                bmp_size = INT32(in_out_bmp.size());
            }
        }
        break;
        case BMP_RGB32 : //RGB24 -> RGB32
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             teamtalk::BMP_RGB32);
            optional_out_bmp.resize(dst_session.GetBitmapSize());
            size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp, src_session, 
                                                       optional_out_bmp, 
                                                       dst_session);
            TTASSERT((size_t)dst_session.GetBitmapSize() == bmp_write);
            bmp_size = optional_out_bmp.size();
        }
        break;
        default :
            return -1;
        }
    }
    break;
    case BMP_RGB32 :
    {
        teamtalk::DesktopSession src_session = 
            teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                         src_wnd.nHeight, 
                                         teamtalk::BMP_RGB32,
                                         src_wnd.nBytesPerLine);
        switch(outputFormat)
        {
        case BMP_RGB8_PALETTE : //RGB32 -> BMP_RGB8_PALETTE
        case BMP_RGB16_555 : //RGB32 -> RGB16
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             (teamtalk::RGBMode)outputFormat);
            size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                       src_session, 
                                                       in_out_bmp,
                                                       dst_session);
            TTASSERT((size_t)dst_session.GetBitmapSize() == bmp_write);

            in_out_bmp.resize(dst_session.GetBitmapSize());
            bmp_size = in_out_bmp.size();
        }
        break;
        case BMP_RGB24 : //RGB32 -> RGB24
        {
            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             teamtalk::BMP_RGB24);
            size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                       src_session, 
                                                       in_out_bmp,
                                                       dst_session);
            TTASSERT((size_t)dst_session.GetBitmapSize() == bmp_write);

            in_out_bmp.resize(dst_session.GetBitmapSize());
            bmp_size = in_out_bmp.size();
        }
        break;
        case BMP_RGB32 : //RGB32 -> RGB32
        {
            //{///////////////////////
            //teamtalk::DesktopSession dst_session1 = 
            //    teamtalk::MakeDesktopSession(src_wnd.nWidth,
            //                                 src_wnd.nHeight, 
            //                                 teamtalk::BMP_RGB8_PALETTE);
            //    optional_out_bmp.resize(dst_session1.GetBitmapSize());
            //    size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
            //                                               src_session, 
            //                                               optional_out_bmp, 
            //                                               dst_session1);

            //    in_out_bmp.resize(src_session.GetBitmapSize());
            //    bmp_write = teamtalk::ConvertBitmap(optional_out_bmp,
            //                                               dst_session1, 
            //                                               in_out_bmp, 
            //                                               src_session);
            //    optional_out_bmp.clear();
            //}////////////////////////////

            teamtalk::DesktopSession dst_session = 
                teamtalk::MakeDesktopSession(src_wnd.nWidth,
                                             src_wnd.nHeight, 
                                             teamtalk::BMP_RGB32);
            if(src_session.GetBitmapSize() != dst_session.GetBitmapSize())
            {
                optional_out_bmp.resize(dst_session.GetBitmapSize());
                size_t bmp_write = teamtalk::ConvertBitmap(in_out_bmp,
                                                           src_session, 
                                                           optional_out_bmp, 
                                                           dst_session);
                TTASSERT(optional_out_bmp.size() == bmp_write);
                bmp_size = optional_out_bmp.size();
            }
            else
            {
                TTASSERT((size_t)src_session.GetBitmapSize() == in_out_bmp.size());
                bmp_size = in_out_bmp.size();
            }
        }
        break;
        default :
            return -1;
        }
    }
    }

    return int(bmp_size);
}

TEAMTALKDLL_API INT32 TT_SendDesktopWindow(IN TTInstance* lpTTInstance,
                                           IN const DesktopWindow* lpDesktopWindow,
                                           IN BitmapFormat nConvertBmpFormat)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(!lpDesktopWindow)
        return -1;

    teamtalk::DesktopSession src_session = 
        teamtalk::MakeDesktopSession(lpDesktopWindow->nWidth, 
                                     lpDesktopWindow->nHeight, 
                                     (teamtalk::RGBMode)lpDesktopWindow->bmpFormat,
                                     lpDesktopWindow->nBytesPerLine);

    MYTRACE_COND(src_session.GetBitmapSize() != lpDesktopWindow->nFrameBufferSize,
                 ACE_TEXT("Rejected bitmap due to invalid size\n"));
    if(src_session.GetBitmapSize() != lpDesktopWindow->nFrameBufferSize)
        return -1;

    BitmapFormat destBmpFmt = lpDesktopWindow->bmpFormat;
    if(nConvertBmpFormat != BMP_NONE)
        destBmpFmt = nConvertBmpFormat;

    teamtalk::DesktopSession dst_session = 
        teamtalk::MakeDesktopSession(lpDesktopWindow->nWidth, 
                                     lpDesktopWindow->nHeight, 
                                     (teamtalk::RGBMode)destBmpFmt);

    if(src_session.GetBitmapSize() == dst_session.GetBitmapSize())
        return pClientNode->SendDesktopWindow(lpDesktopWindow->nWidth, 
                                              lpDesktopWindow->nHeight, 
                                              (teamtalk::RGBMode)lpDesktopWindow->bmpFormat,
                                              (teamtalk::DesktopProtocol)lpDesktopWindow->nProtocol, 
                                              reinterpret_cast<const char*>(lpDesktopWindow->frameBuffer),
                                              lpDesktopWindow->nFrameBufferSize);
    else
    {
        MYTRACE(ACE_TEXT("Warning: slow conversion of bitmap\n"));
        //FIXME: very inefficient!
        vector<char> buf, tmp_buf;
        const char* ptr = reinterpret_cast<const char*>(lpDesktopWindow->frameBuffer);
        buf.assign(ptr, ptr + lpDesktopWindow->nFrameBufferSize);
        if(ConvertBitmap(*lpDesktopWindow, destBmpFmt, buf, tmp_buf) <= 0)
            return -1;

        if(tmp_buf.size())
            return pClientNode->SendDesktopWindow(dst_session.GetWidth(), 
                                                  dst_session.GetHeight(), 
                                                  dst_session.GetRGBMode(),
                                                  (teamtalk::DesktopProtocol)lpDesktopWindow->nProtocol,
                                                  &tmp_buf[0], int(tmp_buf.size()));
        else
            return pClientNode->SendDesktopWindow(dst_session.GetWidth(), 
                                                  dst_session.GetHeight(), 
                                                  dst_session.GetRGBMode(),
                                                  (teamtalk::DesktopProtocol)lpDesktopWindow->nProtocol,
                                                  &buf[0], int(buf.size()));
    }
}

TEAMTALKDLL_API TTBOOL TT_CloseDesktopWindow(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    return pClientNode->CloseDesktopWindow();
}

TEAMTALKDLL_API unsigned char* TT_Palette_GetColorTable(IN BitmapFormat nBmpPalette,
                                                        IN INT32 nIndex)
{


    switch(nBmpPalette)
    {
    case BMP_RGB8_PALETTE :
        if(nIndex<0 || nIndex >= 256)
            return NULL;
        return BMPPalette::Instance()->m_rgb8_palette[nIndex];
    default :
        return NULL;
    }
}

#ifdef WIN32

TEAMTALKDLL_API HWND TT_Windows_GetDesktopActiveHWND()
{
    HWND hWnd = GetForegroundWindow();
    return hWnd;
}

TEAMTALKDLL_API HWND TT_Windows_GetDesktopHWND()
{
    return GetDesktopWindow();
}

struct CheckWindow
{
    HWND hWnd;
    int nIndex;
};

BOOL CALLBACK CheckWindowTitle( HWND hwnd, LPARAM lParam )
{
    if(!IsWindowVisible(hwnd))
        return TRUE;

    CheckWindow* lpWindow = reinterpret_cast<CheckWindow*>(lParam);
    if(lpWindow->nIndex--)
        return TRUE;

    lpWindow->hWnd = hwnd;
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_Windows_GetDesktopWindowHWND(IN INT32 nIndex,
                                                     OUT HWND* lpHWnd)
{
    CheckWindow wnd;
    wnd.hWnd = 0;
    wnd.nIndex = nIndex;

    EnumWindows( (WNDENUMPROC) CheckWindowTitle, (LPARAM) &wnd );
    if(wnd.hWnd != 0)
    {
        if(lpHWnd)
            *lpHWnd = wnd.hWnd;
    }
    return wnd.hWnd != 0;
}

TEAMTALKDLL_API TTBOOL TT_Windows_GetWindow(IN HWND hWnd,
                                          OUT ShareWindow* lpShareWindow)
{
    if(!lpShareWindow)
        return FALSE;

    RECT r;
    if(::GetWindowRect(hWnd, &r))
    {
        lpShareWindow->hWnd = hWnd;
        GetWindowText(hWnd, lpShareWindow->szWindowTitle, TT_STRLEN);
        lpShareWindow->nWndX = r.left;
        lpShareWindow->nWndY = r.top;
        lpShareWindow->nWidth = r.right-r.left;
        lpShareWindow->nHeight = r.bottom - r.top;
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API INT32 TT_SendDesktopWindowFromHWND(IN TTInstance* lpTTInstance,
                                                   IN HWND hWnd, 
                                                   IN BitmapFormat nBitmapFormat,
                                                   IN DesktopProtocol nDesktopProtocol)
{
    ACE_UNUSED_ARG(nDesktopProtocol);

    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    int nX, nY, nX2, nY2;       // coordinates of rectangle to grab
    int nWidth, nHeight;        // DIB width and height

    RECT r;
    if(!::GetWindowRect(hWnd, &r))
        return -1;

    nWidth = r.right - r.left;
    nHeight = r.bottom - r.top;

    if(nWidth <= 0 || nHeight <= 0)
        return -1;

    const ACE_TCHAR* szDriver = ACE_TEXT("DISPLAY");
    const ACE_TCHAR* szDeviceName = NULL;
/*
    HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    if(hMon)
    {
        MONITORINFOEX moninfo;
        moninfo.cbSize = sizeof(moninfo);
        if(GetMonitorInfo(hMon, &moninfo))
        {
            moninfo.cbSize = moninfo.cbSize;
            szDeviceName = moninfo.szDevice;
        }
    }
*/
    HDC hDC = CreateDC(szDriver, szDeviceName, NULL, NULL);     
    HDC hMemDC = ::CreateCompatibleDC(hDC);

    nX = r.left;
    nY = r.top;
    nX2 = r.right;
    nY2 = r.bottom;

    HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
    HGDIOBJ hObj = ::SelectObject(hMemDC, hBitmap);
    ::BitBlt(hMemDC, 0, 0, nWidth, nHeight, hDC, nX, nY, SRCCOPY);

	BITMAPINFO bmpinfo;
	ZeroMemory(&bmpinfo, sizeof(bmpinfo));
	bmpinfo.bmiHeader.biSize = sizeof(bmpinfo.bmiHeader);
	bmpinfo.bmiHeader.biBitCount = 0;

    std::vector<char> buf;

    int ret = ::GetDIBits(hMemDC, hBitmap, 0, 0, NULL, &bmpinfo, DIB_RGB_COLORS);
    if(ret)
    {
        buf.resize(bmpinfo.bmiHeader.biSizeImage);

        bmpinfo.bmiHeader.biCompression = 0;
        bmpinfo.bmiHeader.biHeight *= -1;

        ret = ::GetDIBits(hMemDC, hBitmap, 0, bmpinfo.bmiHeader.biHeight*-1, &buf[0], &bmpinfo, DIB_RGB_COLORS);
        bmpinfo.bmiHeader.biHeight *= -1;
    }

    BOOL b = DeleteObject(hBitmap);
    assert(b);
    b = DeleteDC(hMemDC);
    assert(b);
    b = DeleteDC(hDC);
    assert(b);
    if(!ret)
        return -1;

    teamtalk::RGBMode bmp_mode = teamtalk::BMP_NONE;
    const char* bmp_ptr = NULL;
    int bmp_size = 0;

    vector<char> tmp_bmp_buf;

    DesktopWindow src_wnd = {0};
    switch(bmpinfo.bmiHeader.biBitCount)
    {
    case 16 : 
        src_wnd.bmpFormat = BMP_RGB16_555;
        break;
    case 24 : 
        src_wnd.bmpFormat = BMP_RGB24;
        break;
    case 32 : 
        src_wnd.bmpFormat = BMP_RGB32;
        break;
    default :
        return -1;
    }

    src_wnd.frameBuffer = &buf[0];
    src_wnd.nFrameBufferSize = int(buf.size());
    src_wnd.nWidth = bmpinfo.bmiHeader.biWidth;
    src_wnd.nHeight = bmpinfo.bmiHeader.biHeight;
    src_wnd.nProtocol = nDesktopProtocol;

    return TT_SendDesktopWindow(lpTTInstance, &src_wnd, nBitmapFormat);
}

TEAMTALKDLL_API TTBOOL TT_PaintDesktopWindow(IN TTInstance* lpTTInstance,
                                           IN INT32 nUserID,
                                           IN HDC hDC,
                                           IN INT32 XDest,
                                           IN INT32 YDest,
                                           IN INT32 nDestWidth,
                                           IN INT32 nDestHeight)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    if(user.null() || user->GetDesktopSession().null())
        return FALSE;

    desktop_viewer_t viewer = user->GetDesktopSession();

    return TT_PaintDesktopWindowEx(lpTTInstance, nUserID, hDC, XDest, YDest, 
                                   nDestWidth, nDestHeight, 0, 0,
                                   viewer->GetWidth(), viewer->GetHeight());
}

struct MYBITMAPINFO
{
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[256];
};

TEAMTALKDLL_API TTBOOL TT_PaintDesktopWindowEx(IN TTInstance* lpTTInstance,
                                             IN INT32 nUserID,
                                             IN HDC hDC,
                                             IN INT32 XDest,
                                             IN INT32 YDest,
                                             IN INT32 nDestWidth,
                                             IN INT32 nDestHeight,
                                             IN INT32 XSrc,
                                             IN INT32 YSrc,
                                             IN INT32 nSrcWidth,
                                             IN INT32 nSrcHeight)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    clientuser_t user = pClientNode->GetUser(nUserID);
    teamtalk::DesktopWindow wnd;
    if(user.null() || user->GetDesktopSession().null())
        return FALSE;

    desktop_viewer_t viewer = user->GetDesktopSession();

    if(XSrc + nSrcWidth > viewer->GetWidth() || YSrc + nSrcHeight > viewer->GetHeight())
        return FALSE;

    BITMAPINFOHEADER bmh = {0};
    MYBITMAPINFO bmi = {0};
    bmh.biSize = sizeof(bmh);
    bmh.biWidth = viewer->GetWidth();
    bmh.biHeight = viewer->GetHeight()*-1; //flip image
    bmh.biPlanes = 1;
    bmh.biBitCount = (int)viewer->GetRGBMode() * 8;
    bmh.biCompression = BI_RGB;
    bmh.biSizeImage = viewer->GetBitmapSize();
    bmh.biXPelsPerMeter = 0;
    bmh.biYPelsPerMeter = 0;
    bmh.biClrUsed = 0;
    bmh.biClrImportant = 0;
    bmi.bmiHeader = bmh;

    BITMAPINFO* pBMI = reinterpret_cast<BITMAPINFO*>(&bmi);

    BOOL b;
    if(viewer->GetRGBMode() == BMP_RGB8_PALETTE)
    {
        RGBQUAD* rgb = reinterpret_cast<RGBQUAD*>(&BMPPalette::Instance()->m_rgb8_palette[0]);
        memcpy(pBMI->bmiColors, rgb, sizeof(BMPPalette::Instance()->m_rgb8_palette));
    }
    b = StretchDIBits(hDC, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, 
                      nSrcWidth, nSrcHeight, viewer->GetBitmap(), pBMI,
                      DIB_RGB_COLORS, SRCCOPY);
    TTASSERT(b);
    return b;
}

#elif defined(__APPLE__) && !TARGET_OS_IPHONE

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>

typedef vector<ShareWindow> shared_windows_t;

static void EnumerateWindowList(const void *inputDictionary, void *context)
{
    shared_windows_t& windows = *reinterpret_cast<shared_windows_t*>(context);
    ShareWindow wnd = {0};

    CFDictionaryRef entry = (CFDictionaryRef)inputDictionary;
    CFNumberRef sharingState; /* SInt32 */
    CFNumberRef windowNumber; /* SInt64 */
    CFNumberRef ownerPID;     /* SInt64 */
    CFStringRef windowTitle;
    CGRect rect;

    Boolean status;
    SInt32 sint32;
    SInt64 sint64;

    status = CFDictionaryGetValueIfPresent(entry, kCGWindowSharingState,
                                           (const void **)&sharingState);
    if (status)
    {
        status = CFNumberGetValue(sharingState, kCFNumberSInt32Type,
                                  (void *)&sint32);
        if (status && (sint32 == 0))
            return;
    }
    
    status = CFDictionaryGetValueIfPresent(entry, kCGWindowOwnerPID,
                                           (const void **)&ownerPID);
    if (status)
    {
        status = CFNumberGetValue(ownerPID, kCFNumberSInt64Type,
                                  (void *)&sint64); 
        if (status)
            wnd.nPID = sint64;
        else
            return;
    }
    else
        return;

    status = CFDictionaryGetValueIfPresent(entry, kCGWindowNumber,
                                           (const void **)&windowNumber);
    if (status)
    {
        status = CFNumberGetValue(windowNumber, kCFNumberSInt64Type,
                                  (void *)&sint64); 
        if (status)
            wnd.nWindowID = sint64;
    }

    status = CFDictionaryGetValueIfPresent(entry, kCGWindowOwnerName,
                                           (const void **)&windowTitle);
    if (status)
    {
        CFIndex length = CFStringGetLength(windowTitle);
        status = CFStringGetCString(windowTitle, wnd.szWindowTitle, TT_STRLEN-1,
                                    kCFStringEncodingUTF8);
    }

    CFDictionaryRef rectDictionary;
    status = CFDictionaryGetValueIfPresent(entry, kCGWindowBounds,
                                           (const void **)&rectDictionary);
    if (status)
    {
        status = CGRectMakeWithDictionaryRepresentation(rectDictionary, &rect);
        if (status)
        {
            wnd.nWindowX = rect.origin.x;
            wnd.nWindowY = rect.origin.y;
            wnd.nWidth = rect.size.width;
            wnd.nHeight = rect.size.height;
        }
    }

    //MYTRACE("String '%s' - %d\n", wnd.szWindowTitle, windows.size());

//     CGRect rect;
//     CGError err = CGSGetScreenRectForWindow(_CGSDefaultConnection(), 
//                                             windows[nIndex], &rect);
//     if (err != 0)
//         return FALSE;

    windows.push_back(wnd);
}

TTBOOL PopulateWindowTitles(shared_windows_t& windows)
{
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, 
                                                       kCGNullWindowID);
    if (!windowList) {
        return FALSE;
    }

    CFArrayApplyFunction(windowList, 
                         CFRangeMake(0, CFArrayGetCount(windowList)),
                         &EnumerateWindowList, &windows);
    CFRelease(windowList);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_MacOS_GetWindow(IN INT32 nIndex,
                                        OUT ShareWindow* lpShareWindow)
{
    shared_windows_t windows;

    if(!lpShareWindow)
        return FALSE;
    if(!PopulateWindowTitles(windows))
        return FALSE;
    if(nIndex >= (INT32)windows.size())
        return FALSE;

    *lpShareWindow = windows[nIndex];
    
//     lpShareWindow->nWindowID = windowIDs[nIndex];
//     lpShareWindow->nWidth = (INT32)rect.size.width;
//     lpShareWindow->nHeight = (INT32)rect.size.height;
//     ACE_OS::strsncpy(lpShareWindow->szWindowTitle, 
//                      data.windowTitles[nIndex].c_str(), TT_STRLEN);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_MacOS_GetWindowFromWindowID(IN INT64 nWindowID,
                                                    OUT ShareWindow* lpShareWindow)
{
    shared_windows_t windows;

    if(!lpShareWindow)
        return FALSE;
    if(!PopulateWindowTitles(windows))
        return FALSE;

    for(size_t i=0;i<windows.size();i++)
    {
        if(windows[i].nWindowID == nWindowID)
        {
            *lpShareWindow = windows[i];
            return TRUE;
        }
    }
    return FALSE;
}

TEAMTALKDLL_API INT32 TT_SendDesktopFromWindowID(IN TTInstance* lpTTInstance,
                                                 IN INT64 nWindowID, 
                                                 IN BitmapFormat nBitmapFormat,
                                                 IN DesktopProtocol nDesktopProtocol)
{
    CGImageRef image_ref = nil;
    CGDataProviderRef provider_ref = nil;
    CFDataRef data_ref = nil;
    int ret = -1, bmp_size = -1;
    DesktopWindow wnd = {0};
    CFIndex data_len;
    size_t row_bytes;
    size_t rows;
    const UInt8* data_ptr;
    vector<char> buf;
    vector<char> tmp_bmp_buf;

    image_ref = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow,
                                        nWindowID, kCGWindowImageBoundsIgnoreFraming);
    if(!image_ref)
        goto cleanup;

    row_bytes = CGImageGetBytesPerRow(image_ref);
    rows = CGImageGetHeight(image_ref);

    provider_ref = CGImageGetDataProvider(image_ref);
    if(!provider_ref)
        goto cleanup;

    data_ref = CGDataProviderCopyData(provider_ref);
    if(!data_ref)
        goto cleanup;

    data_len = CFDataGetLength(data_ref);
    data_ptr = CFDataGetBytePtr(data_ref);
    if(data_len == 0)
        goto cleanup;

    //FIXME: this copy shouldn't be nescessary
    buf.assign(data_ptr, data_ptr+data_len);

    wnd.nProtocol = nDesktopProtocol;
    wnd.nWidth = CGImageGetWidth(image_ref);
    wnd.nHeight = CGImageGetHeight(image_ref);
    wnd.nBytesPerLine = row_bytes;
    wnd.frameBuffer = &buf[0];
    wnd.nFrameBufferSize = row_bytes * wnd.nHeight;

    switch(CGImageGetBitsPerPixel(image_ref))
    {
    case 32 :
        wnd.bmpFormat = BMP_RGB32;
        bmp_size = ConvertBitmap(wnd, nBitmapFormat, buf, tmp_bmp_buf);
        break;
    case 24 :
        wnd.bmpFormat = BMP_RGB24;
        bmp_size = ConvertBitmap(wnd, nBitmapFormat, buf, tmp_bmp_buf);
        break;
    case 16 :
        wnd.bmpFormat = BMP_RGB16_555;
        bmp_size = ConvertBitmap(wnd, nBitmapFormat, buf, tmp_bmp_buf);
        break;
    default :
        goto cleanup;
        break;
    }

    if(bmp_size<=0)
        goto cleanup;

    //switch to converted bitmap format
    wnd.bmpFormat = nBitmapFormat;
    wnd.nBytesPerLine = 0;
    wnd.nFrameBufferSize = bmp_size;

    if(tmp_bmp_buf.size())
    {
        wnd.frameBuffer = &tmp_bmp_buf[0];
        ret = TT_SendDesktopWindow(lpTTInstance, &wnd, BMP_NONE);
    }
    else
    {
        wnd.frameBuffer = &buf[0];
        ret = TT_SendDesktopWindow(lpTTInstance, &wnd, BMP_NONE);
    }

cleanup:

    if(data_ref)
        CFRelease(data_ref);

    if(image_ref)
        CGImageRelease(image_ref);

    return ret;
}

#endif

TEAMTALKDLL_API TTBOOL TT_SendDesktopCursorPosition(IN TTInstance* lpTTInstance,
                                                  IN UINT16 nPosX,
                                                  IN UINT16 nPosY)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    return pClientNode->SendDesktopCursor(nPosX, nPosY);
}

TEAMTALKDLL_API TTBOOL TT_SendDesktopInput(IN TTInstance* lpTTInstance,
                                         IN INT32 nUserID,
                                         IN const DesktopInput* lpDesktopInputs,
                                         IN INT32 nDesktopInputCount)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(!lpDesktopInputs)
        return FALSE;

    std::vector<teamtalk::DesktopInput> inputs;
    for(int m=0;m<TT_DESKTOPINPUT_MAX && m<nDesktopInputCount;m++)
    {
        teamtalk::DesktopInput input;
        Convert(lpDesktopInputs[m], input);
        inputs.push_back(input);
    }
    return pClientNode->SendDesktopInput(nUserID, inputs);
}


TEAMTALKDLL_API DesktopWindow* TT_AcquireUserDesktopWindow(IN TTInstance* lpTTInstance, 
                                                           IN INT32 nUserID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return NULL;

    clientuser_t user = pClientNode->GetUser(nUserID);
    teamtalk::DesktopWindow wnd;
    if(user.null() || !user->GetDesktopWindow(wnd))
        return NULL;

    desktop_viewer_t viewer = user->GetDesktopSession();
    if(viewer.null())
        return NULL;

    DesktopWindow* lpDesktopWindow = inst->PushDesktopWindow(viewer->GetBitmapSize());
    lpDesktopWindow->nWidth = viewer->GetWidth();
    lpDesktopWindow->nHeight = viewer->GetHeight();
    lpDesktopWindow->bmpFormat = (BitmapFormat)viewer->GetRGBMode();
    lpDesktopWindow->nSessionID = viewer->GetSessionID();
    lpDesktopWindow->nBytesPerLine = viewer->GetBytesPerLine();
    lpDesktopWindow->nProtocol = DESKTOPPROTOCOL_ZLIB_1;

// static int x = 0;
// ACE_TCHAR buf[400];
// ACE_OS::sprintf(buf, ACE_TEXT("%d %dx%d.bmp"), x++, lpDesktopWindow->nWidth, lpDesktopWindow->nHeight);
// user->GetDesktopSession()->WriteBitmapToFile(buf);
    bool b = user->GetDesktopWindow(reinterpret_cast<char*>(lpDesktopWindow->frameBuffer),
                                    lpDesktopWindow->nFrameBufferSize);
    TTASSERT(b);
    return lpDesktopWindow;
}

TEAMTALKDLL_API DesktopWindow* TT_AcquireUserDesktopWindowEx(IN TTInstance* lpTTInstance, 
                                                             IN INT32 nUserID,
                                                             IN BitmapFormat nBitmapFormat)
{
    DesktopWindow* lpDesktopWindow = TT_AcquireUserDesktopWindow(lpTTInstance,
                                                                 nUserID);
    if(!lpDesktopWindow || lpDesktopWindow->bmpFormat == nBitmapFormat)
        return lpDesktopWindow;
    //TODO: 2 x copy, inefficient
    const char* in_buf = reinterpret_cast<const char*>(lpDesktopWindow->frameBuffer);
    vector<char> in_bmp(in_buf, in_buf+lpDesktopWindow->nFrameBufferSize), out_bmp;
    DesktopWindow deskwnd = *lpDesktopWindow;
    deskwnd.frameBuffer = &in_bmp[0];
    TT_ReleaseUserDesktopWindow(lpTTInstance, lpDesktopWindow);

    int size = ConvertBitmap(deskwnd, nBitmapFormat, in_bmp, out_bmp);
    if(size>0)
    {
        ClientInstance* inst = GET_CLIENT(lpTTInstance);
        TTASSERT(inst);
        if(!inst)
            return NULL;

        DesktopWindow* lpNewWindow = inst->PushDesktopWindow(size);
        lpNewWindow->nWidth = deskwnd.nWidth;
        lpNewWindow->nHeight = deskwnd.nHeight;
        lpNewWindow->bmpFormat = nBitmapFormat;
        lpNewWindow->nSessionID = deskwnd.nSessionID;
        lpNewWindow->nBytesPerLine = deskwnd.nBytesPerLine;
        lpNewWindow->nProtocol = deskwnd.nProtocol;
        if(out_bmp.size())
            ACE_OS::memcpy(lpNewWindow->frameBuffer, &out_bmp[0], size);
        else
            ACE_OS::memcpy(lpNewWindow->frameBuffer, &in_bmp[0], size);
            
        return lpNewWindow;
    }
    else
    {
        return NULL;
    }
}

TEAMTALKDLL_API TTBOOL TT_ReleaseUserDesktopWindow(IN TTInstance* lpTTInstance, 
                                                 IN DesktopWindow* lpDesktopWindow)
{
    ClientInstance* inst = GET_CLIENT(lpTTInstance);
    TTASSERT(inst);
    if(!inst)
        return FALSE;

    return (TTBOOL)inst->RemoveDesktopWindow(lpDesktopWindow);
}


#if defined(WIN32) /* Windows HOTKEY */

TEAMTALKDLL_API TTBOOL TT_HotKey_Register(IN TTInstance* lpTTInstance,
                                        IN INT32 nHotKeyID, 
                                        IN const INT32* lpnVKCodes,
                                        IN INT32 nVKCodeCount)
{

    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return FALSE;

    HotKeyHook hotkey;
    hotkey.hotkeyid = nHotKeyID;
    hotkey.active = false;
    for(int i=0;i<nVKCodeCount;i++)
        hotkey.keys.insert(lpnVKCodes[i]);
    hotkey.listener = pClient->pEventHandler;

    if(!HOTKEY->HotKeyExists(pClient->pEventHandler, nHotKeyID))
        HOTKEY_USAGE(1);
    HOTKEY->RegisterHotKey(hotkey);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_HotKey_Unregister(IN TTInstance* lpTTInstance,
                                          INT32 nHotKeyID)
{
    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return FALSE;

    if(HOTKEY->HotKeyExists(pClient->pEventHandler, nHotKeyID))
    {
        HOTKEY->UnregisterHotKey(pClient->pEventHandler, nHotKeyID);
        HOTKEY_USAGE(-1);
    }
    return TRUE;
}

TEAMTALKDLL_API INT32 TT_HotKey_IsActive(IN TTInstance* lpTTInstance,
                                        IN INT32 nHotKeyID)
{
    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return -1;
    return HOTKEY->IsHotKeyActive(pClient->pEventHandler, nHotKeyID);
}

TEAMTALKDLL_API TTBOOL TT_HotKey_InstallTestHook(IN TTInstance* lpTTInstance,
                                               IN HWND hWnd, UINT uMsg)
{
    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return FALSE;

    HOTKEY_USAGE(1);

    pClient->pEventHandler->SetKeyHWND(hWnd, uMsg);
    HOTKEY->AddKeyTester(pClient->pEventHandler);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_HotKey_RemoveTestHook(IN TTInstance* lpTTInstance)
{
    CLIENT pClient = GET_CLIENT(lpTTInstance);
    if(!pClient)
        return FALSE;

    HOTKEY_USAGE(-1);

    pClient->pEventHandler->SetKeyHWND(NULL, 0);
    HOTKEY->RemoveKeyTester(pClient->pEventHandler);
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TT_HotKey_GetKeyString(IN TTInstance* lpTTInstance,
                                            IN INT32 nVKCode,
                                            OUT TTCHAR szKeyName[TT_STRLEN])
{
#ifndef UNDER_CE

#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC 0
#endif

    UINT scancode = MapVirtualKey(nVKCode, MAPVK_VK_TO_VSC);
    return ::GetKeyNameText( scancode << 16 , szKeyName, TT_STRLEN)>0;

#else
    return FALSE;
#endif
}

#endif /* WIN32 hotkeys */



TEAMTALKDLL_API INT32 TT_DBG_SIZEOF(IN TTType nType)
{
    switch(nType)
    {
    case __AUDIOCODEC :
        return sizeof(AudioCodec);
    case __BANNEDUSER :
        return sizeof(BannedUser);
    case __VIDEOFORMAT :
        return sizeof(VideoFormat);
    case __OPUSCODEC :
        return sizeof(OpusCodec);
    case __CHANNEL :
        return sizeof(Channel);
    case __CLIENTSTATISTICS :
        return sizeof(ClientStatistics);
    case __REMOTEFILE :
        return sizeof(RemoteFile);
    case __FILETRANSFER :
        return sizeof(FileTransfer);
    case __SERVERPROPERTIES :
        return sizeof(ServerProperties);
    case __SERVERSTATISTICS :
        return sizeof(ServerStatistics);
    case __SOUNDDEVICE :
        return sizeof(SoundDevice);
    case __SPEEXCODEC :
        return sizeof(SpeexCodec);
    case __TEXTMESSAGE :
        return sizeof(TextMessage);
    case __WEBMVP8CODEC :
        return sizeof(WebMVP8Codec);
    case __TTMESSAGE :
        return sizeof(TTMessage);
    case __USER :
        return sizeof(User);
    case __USERACCOUNT :
        return sizeof(UserAccount);
    case __USERSTATISTICS :
        return sizeof(UserStatistics);
    case __VIDEOCAPTUREDEVICE :
        return sizeof(VideoCaptureDevice);
    case __VIDEOCODEC :
        return sizeof(VideoCodec);
    case __AUDIOCONFIG :
        return sizeof(AudioConfig);
    case __SPEEXVBRCODEC :
        return sizeof(SpeexVBRCodec);
    case __VIDEOFRAME :
        return sizeof(VideoFrame);
    case __AUDIOBLOCK :
        return sizeof(AudioBlock);
    case __AUDIOFORMAT :
        return sizeof(AudioFormat);
    case __MEDIAFILEINFO :
        return sizeof(MediaFileInfo);
    case __DESKTOPINPUT :
        return sizeof(DesktopInput);
    case __CLIENTERRORMSG :
        return sizeof(ClientErrorMsg);
    case __TTBOOL :
        return sizeof(TTBOOL);
    case __INT32 :
        return sizeof(INT32);
    case __MEDIAFILESTATUS :
        return sizeof(MediaFileStatus);
    case __SPEEXDSP :
        return sizeof(SpeexDSP);
    case __STREAMTYPE :
        return sizeof(StreamType);
    }
    return 0;
}

TEAMTALKDLL_API VOID* TT_DBG_GETDATAPTR(IN TTMessage* pMsg)
{
    return &pMsg->nPayloadSize;
}

TEAMTALKDLL_API TTBOOL TT_DBG_SetSoundInputTone(IN TTInstance* lpTTInstance,
                                              IN StreamTypes uStreamTypes,
                                              IN INT32 nFrequency)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    pClientNode->SetSoundInputTone(uStreamTypes, nFrequency);
    return TRUE;
}


TEAMTALKDLL_API TTBOOL TT_GetChannelFiles(IN TTInstance* lpTTInstance,
                                        IN INT32 nChannelID, 
                                        IN OUT RemoteFile* lpRemoteFiles,
                                        IN OUT INT32* lpnHowMany)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(lpnHowMany)
    {
        teamtalk::ChannelProp chan;
        if(!pClientNode->GetChannelProp(nChannelID, chan))
            return FALSE;

        if(lpRemoteFiles == NULL)
        {
            *lpnHowMany = (INT32)chan.files.size();
            return TRUE;
        }
        
        size_t less = (size_t)*lpnHowMany < chan.files.size()? *lpnHowMany : chan.files.size();
        for(size_t i=0;i<less;i++)
        {
            TT_GetChannelFile(lpTTInstance, nChannelID, chan.files[i].fileid, &lpRemoteFiles[i]);
        }
        *lpnHowMany = (INT32)less;
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_GetChannelFile(IN TTInstance* lpTTInstance,
                                           IN INT32 nChannelID, 
                                           IN INT32 nFileID, 
                                           OUT RemoteFile* lpRemoteFile)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(lpRemoteFile)
    {
        clientchannel_t chan = pClientNode->GetChannel(nChannelID);
        teamtalk::RemoteFile ttremotefile;
        if(!chan.null() && chan->GetFile(nFileID, ttremotefile, false))
        {
            Convert(ttremotefile, *lpRemoteFile);
            return TRUE;
        }
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_GetFileTransferInfo(IN TTInstance* lpTTInstance,
                                            IN INT32 nTransferID, 
                                            OUT FileTransfer* lpTransfer)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    if(lpTransfer)
    {
        teamtalk::FileTransfer transfer;
        ACE_INT64 transferred = 0;
        if(pClientNode->GetTransferInfo(nTransferID, transfer))
        {
            Convert(transfer, *lpTransfer);
            return TRUE;
        }
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_CancelFileTransfer(IN TTInstance* lpTTInstance,
                                           IN INT32 nTransferID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    return pClientNode->CancelFileTransfer(nTransferID);
}

TEAMTALKDLL_API INT32 TT_DoSubscribe(IN TTInstance* lpTTInstance,
                                     IN INT32 nUserID, 
                                     IN Subscriptions uSubscriptions)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoSubscribe(nUserID, uSubscriptions);
}

TEAMTALKDLL_API INT32 TT_DoUnsubscribe(IN TTInstance* lpTTInstance,
                                       IN INT32 nUserID, 
                                       IN Subscriptions uSubscriptions)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoUnsubscribe(nUserID, uSubscriptions);
}

TEAMTALKDLL_API void TT_GetErrorMessage(IN INT32 nError, OUT TTCHAR szErrorMsg[TT_STRLEN])
{
    ACE_TString err = teamtalk::GetErrorDescription(nError);
    ACE_OS::strsncpy(szErrorMsg, err.c_str(), TT_STRLEN);
}

TEAMTALKDLL_API TTBOOL TT_GetMessage(IN TTInstance* lpTTInstance, 
                                   OUT TTMessage* pMsg,
                                   IN const INT32* pnWaitMs)
{
    ClientInstance* pClient = GET_CLIENT(lpTTInstance);
    if(pClient && pMsg)
    {
        if(pnWaitMs && *pnWaitMs != -1)
        {
            ACE_Time_Value tv(*pnWaitMs/1000, (*pnWaitMs % 1000) * 1000);
            tv += ACE_OS::gettimeofday();
            return pClient->pEventHandler->GetMessage(*pMsg, &tv);
        }

        return pClient->pEventHandler->GetMessage(*pMsg, NULL);
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_PumpMessage(IN TTInstance* lpTTInstance,
                                      ClientEvent nEvent,
                                      INT32 nIdentifier)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, FALSE);

    switch(nEvent) {
    case CLIENTEVENT_USER_STATECHANGE :
        return pClientNode->StartUserTimer(USER_TIMER_UPDATE_USER, nIdentifier, nIdentifier,
                                           ACE_Time_Value(), ACE_Time_Value()) >= 0;
    default :
        return false;
    }
 }

TEAMTALKDLL_API INT32 TT_DoMakeChannel(IN TTInstance* lpTTInstance,
                                       IN const Channel* lpChannel)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    if(lpChannel)
    {
        teamtalk::ChannelProp prop;
        if(!Convert(*lpChannel, prop))
            return -1;

        return pClientNode->DoMakeChannel(prop);
    }
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoUpdateChannel(IN TTInstance* lpTTInstance,
                                         IN const Channel* lpChannel)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(lpChannel)
    {
        teamtalk::ChannelProp prop;
        if(!Convert(*lpChannel, prop))
            return -1;

        return pClientNode->DoUpdateChannel(prop);
    }
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoRemoveChannel(IN TTInstance* lpTTInstance,
                                         IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    return pClientNode->DoRemoveChannel(nChannelID);
}


TEAMTALKDLL_API INT32 TT_DoMoveUser(IN TTInstance* lpTTInstance,
                                    IN INT32 nUserID, 
                                    IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoMoveUser(nUserID, nChannelID);
}

TEAMTALKDLL_API INT32 TT_DoUpdateServer(IN TTInstance* lpTTInstance,
                                        IN const ServerProperties* lpServerProperties)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(lpServerProperties)
    {
        teamtalk::ServerInfo serverprop, tmp;
        // need to get host information to fill out new TCP/UDP ports
        if (pClientNode->GetServerInfo(tmp))
            serverprop.hostaddrs = tmp.hostaddrs;
        Convert(*lpServerProperties, serverprop);
        return pClientNode->DoUpdateServer(serverprop);
    }
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoListUserAccounts(IN TTInstance* lpTTInstance,
                                            IN INT32 nIndex,
                                            IN INT32 nCount)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    return pClientNode->DoListUserAccounts(nIndex, nCount);
}

TEAMTALKDLL_API INT32 TT_DoNewUserAccount(IN TTInstance* lpTTInstance,
                                          IN const UserAccount* lpUserAccount)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(!lpUserAccount)
        return -1;

    teamtalk::UserAccount intuser;
    Convert(*lpUserAccount, intuser);

    return pClientNode->DoNewUserAccount(intuser);
}

TEAMTALKDLL_API INT32 TT_DoDeleteUserAccount(IN TTInstance* lpTTInstance,
                                             IN const TTCHAR* szUsername)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    if(!szUsername)
        return -1;
    return pClientNode->DoDeleteUserAccount(szUsername);
}

TEAMTALKDLL_API INT32 TT_DoBanUser(IN TTInstance* lpTTInstance,
                                   IN INT32 nUserID,
                                   IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    teamtalk::BannedUser ban;
    ban.bantype = teamtalk::BANTYPE_DEFAULT;
    if(nChannelID)
        ban.bantype |= BANTYPE_CHANNEL;

    return pClientNode->DoBanUser(nUserID, ban);
}

TEAMTALKDLL_API INT32 TT_DoBanUserEx(IN TTInstance* lpTTInstance,
                                     IN INT32 nUserID,
                                     IN BanTypes uBanTypes)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    teamtalk::BannedUser ban;
    ban.bantype = teamtalk::BanTypes(uBanTypes);

    return pClientNode->DoBanUser(nUserID, ban);
}

TEAMTALKDLL_API INT32 TT_DoBan(IN TTInstance* lpTTInstance,
                               IN const BannedUser* lpBannedUser)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    teamtalk::BannedUser ban;
    if(lpBannedUser)
    {
        Convert(*lpBannedUser, ban);
        return pClientNode->DoBanUser(0, ban);
    }
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoBanIPAddress(IN TTInstance* lpTTInstance,
                                        IN const TTCHAR* szIPAddress,
                                        IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);

    if(!szIPAddress)
        return -1;

    teamtalk::BannedUser ban;
    ban.bantype = teamtalk::BANTYPE_DEFAULT;
    ban.ipaddr = szIPAddress;

    return pClientNode->DoBanUser(0, ban);
}

TEAMTALKDLL_API INT32 TT_DoUnBanUser(IN TTInstance* lpTTInstance,
                                     IN const TTCHAR* szIPAddress,
                                     IN INT32 nChannelID)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    teamtalk::BannedUser ban;
    ban.bantype = teamtalk::BANTYPE_DEFAULT;
    ban.ipaddr = szIPAddress;
    return pClientNode->DoUnBanUser(ban);
}

TEAMTALKDLL_API INT32 TT_DoUnBanUserEx(IN TTInstance* lpTTInstance,
                                       IN const BannedUser* lpBannedUser)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    teamtalk::BannedUser ban;
    if(lpBannedUser)
    {
        Convert(*lpBannedUser, ban);
        return pClientNode->DoUnBanUser(ban);
    }
    return -1;
}

TEAMTALKDLL_API INT32 TT_DoListBans(IN TTInstance* lpTTInstance,
                                    IN INT32 nChannelID,
                                    IN INT32 nIndex, IN INT32 nCount)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoListBans(nChannelID, nIndex, nCount);
}

TEAMTALKDLL_API INT32 TT_DoSaveConfig(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoSaveConfig();
}

TEAMTALKDLL_API INT32 TT_DoQueryServerStats(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoQueryServerStats();
}

TEAMTALKDLL_API INT32 TT_DoQuit(IN TTInstance* lpTTInstance)
{
    ClientNode* pClientNode;
    GET_CLIENTNODE_RET(pClientNode, lpTTInstance, -1);
    return pClientNode->DoQuit();
}

TEAMTALKDLL_API INT32 TT_DesktopInput_KeyTranslate(TTKeyTranslate nTranslate,
                                                   IN const DesktopInput* lpDesktopInputs,
                                                   OUT DesktopInput* lpTranslatedDesktopInputs,
                                                   IN INT32 nDesktopInputCount)
{
    int count = 0;
    for(int i=0;i<nDesktopInputCount;i++)
        TranslateDesktopInput(nTranslate, lpDesktopInputs[i],
                              lpTranslatedDesktopInputs[i])? count++ : count = count;
    return count;
}

TEAMTALKDLL_API INT32 TT_DesktopInput_Execute(IN const DesktopInput* lpDesktopInputs,
                                              IN INT32 nDesktopInputCount)
{
#if defined(ACE_WIN32)
    std::vector<INPUT> inputs;

    for(int i=0;i<nDesktopInputCount;i++)
    {
        INPUT input = {0};

        if(lpDesktopInputs[i].uMousePosX != TT_DESKTOPINPUT_MOUSEPOS_IGNORE &&
           lpDesktopInputs[i].uMousePosY != TT_DESKTOPINPUT_MOUSEPOS_IGNORE)
        {
            input.type = INPUT_MOUSE;
            input.mi.dwExtraInfo = GetMessageExtraInfo();
            input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

            int screen_x = GetSystemMetrics(SM_CXSCREEN);
            int screen_y = GetSystemMetrics(SM_CYSCREEN);
            input.mi.dx = (0xFFFF * lpDesktopInputs[i].uMousePosX) / screen_x;
            input.mi.dy = (0xFFFF * lpDesktopInputs[i].uMousePosY) / screen_y;

            switch(lpDesktopInputs[i].uKeyCode)
            {
            case TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN :
                if(lpDesktopInputs[i].uKeyState & DESKTOPKEYSTATE_DOWN)
                    input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
                if(lpDesktopInputs[i].uKeyState & DESKTOPKEYSTATE_UP)
                    input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
                break;
            case TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN :
                if(lpDesktopInputs[i].uKeyState & DESKTOPKEYSTATE_DOWN)
                    input.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
                if(lpDesktopInputs[i].uKeyState & DESKTOPKEYSTATE_UP)
                    input.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
                break;
            case TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN :
                if(lpDesktopInputs[i].uKeyState & DESKTOPKEYSTATE_DOWN)
                    input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
                if(lpDesktopInputs[i].uKeyState & DESKTOPKEYSTATE_UP)
                    input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
                break;
            default : break;
            }
            inputs.push_back(input);
        }
        else if(lpDesktopInputs[i].uKeyCode != TT_DESKTOPINPUT_KEYCODE_IGNORE &&
                lpDesktopInputs[i].uKeyState != DESKTOPKEYSTATE_NONE)
        {

            input.type = INPUT_KEYBOARD;
            input.ki.dwExtraInfo = GetMessageExtraInfo();
            //input.ki.wVk = lpDesktopInputs[i].uKeyCode;
            input.ki.wScan = lpDesktopInputs[i].uKeyCode;
            input.ki.dwFlags = KEYEVENTF_SCANCODE;
            if (lpDesktopInputs[i].uKeyCode & 0x100)
                input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;

            if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_UP)
                input.ki.dwFlags |= KEYEVENTF_KEYUP;

            inputs.push_back(input);
        }
    }
    if(inputs.size())
        return SendInput(UINT(inputs.size()), &inputs[0], sizeof(inputs[0])) != 0;
    return 0;

#elif defined(__APPLE__) && !TARGET_OS_IPHONE

    for(int i=0;i<nDesktopInputCount;i++)
    {
        if(lpDesktopInputs[i].uMousePosX != TT_DESKTOPINPUT_MOUSEPOS_IGNORE &&
           lpDesktopInputs[i].uMousePosY != TT_DESKTOPINPUT_MOUSEPOS_IGNORE)
        {
            CGEventRef event1;
            CGEventType event_type = kCGEventMouseMoved;

            switch(lpDesktopInputs[i].uKeyCode)
            {
            case TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN :
                if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_UP)
                    event_type = kCGEventLeftMouseUp;
                else if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_DOWN)
                    event_type = kCGEventLeftMouseDown;
                break;
            case TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN :
//                 if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_UP)
//                     event_type = kCGEventLeftMouseUp;
//                 else if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_DOWN)
//                     event_type = kCGEventLeftMouseDown;
                break;
            case TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN :
                if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_UP)
                    event_type = kCGEventRightMouseUp;
                else if(lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_DOWN)
                    event_type = kCGEventRightMouseDown;
                break;
            }
            
            event1 = CGEventCreateMouseEvent(NULL, event_type,
                                             CGPointMake(lpDesktopInputs[i].uMousePosX,
                                                         lpDesktopInputs[i].uMousePosY),
                                             kCGMouseButtonLeft);
            if(event1)
            {
                CGEventPost(kCGSessionEventTap, event1);
                CFRelease(event1);
            }
        }
        else if(lpDesktopInputs[i].uKeyCode != TT_DESKTOPINPUT_KEYCODE_IGNORE &&
                lpDesktopInputs[i].uKeyState != DESKTOPKEYSTATE_NONE)
        {
            CGEventRef event1;
            event1 = CGEventCreateKeyboardEvent(NULL, lpDesktopInputs[i].uKeyCode, 
                                                lpDesktopInputs[i].uKeyState == DESKTOPKEYSTATE_DOWN);
            if(event1)
            {
                CGEventPost(kCGSessionEventTap, event1);
                CFRelease(event1);
            }
        }
    }
#else
    //TODO: Linux, key translate
#endif
    return 0;
}

#if defined(WIN32)

TEAMTALKDLL_API INT32 TT_Mixer_GetMixerCount()
{
    return mixerGetCount();
}

TEAMTALKDLL_API TTBOOL TT_Mixer_GetMixerName(IN INT32 nMixerIndex, OUT TTCHAR szMixerName[TT_STRLEN])
{
    if(szMixerName)
    {
        mixerGetName(nMixerIndex, szMixerName);
        return TRUE;
    }
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveInName(IN INT32 nWaveDeviceID,
                                            OUT TTCHAR szMixerName[TT_STRLEN])
{
    return mixerGetWaveInName(nWaveDeviceID, szMixerName);
}

TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveOutName(IN INT32 nWaveDeviceID,
                                             OUT TTCHAR szMixerName[TT_STRLEN])
{
    return mixerGetWaveOutName(nWaveDeviceID, szMixerName);
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveOutMute(IN INT32 nWaveDeviceID, IN MixerControl nControl, TTBOOL bMute)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = bMute;
    switch(nControl)
    {
    case WAVEOUT_MASTER :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MASTER | MIXER_WAVEOUT_SET | MIXER_WAVEOUT_MUTE, val);
        break;
    case WAVEOUT_WAVE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_WAVE | MIXER_WAVEOUT_SET | MIXER_WAVEOUT_MUTE, val);
        break;
    case WAVEOUT_MICROPHONE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MICROPHONE | MIXER_WAVEOUT_SET | MIXER_WAVEOUT_MUTE, val);
        break;
    }
    return result;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveOutMute(IN INT32 nWaveDeviceID, IN MixerControl nControl)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    switch(nControl)
    {
    case WAVEOUT_MASTER :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MASTER | MIXER_WAVEOUT_GET | MIXER_WAVEOUT_MUTE, val);
        break;
    case WAVEOUT_WAVE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_WAVE | MIXER_WAVEOUT_GET | MIXER_WAVEOUT_MUTE, val);
        break;
    case WAVEOUT_MICROPHONE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MICROPHONE | MIXER_WAVEOUT_GET | MIXER_WAVEOUT_MUTE, val);
        break;
    }
    if(!result)
        return -1;

    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveOutVolume(IN INT32 nWaveDeviceID, IN MixerControl nControl, IN INT32 nVolume)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = nVolume;
    switch(nControl)
    {
    case WAVEOUT_MASTER :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MASTER | MIXER_WAVEOUT_SET | MIXER_WAVEOUT_VOLUME, val);
        break;
    case WAVEOUT_WAVE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_WAVE | MIXER_WAVEOUT_SET | MIXER_WAVEOUT_VOLUME, val);
        break;
    case WAVEOUT_MICROPHONE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MICROPHONE | MIXER_WAVEOUT_SET | MIXER_WAVEOUT_VOLUME, val);
        break;
    }
    return result;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveOutVolume(IN INT32 nWaveDeviceID, IN MixerControl nControl)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    switch(nControl)
    {
    case WAVEOUT_MASTER :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MASTER | MIXER_WAVEOUT_GET | MIXER_WAVEOUT_VOLUME, val);
        break;
    case WAVEOUT_WAVE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_WAVE | MIXER_WAVEOUT_GET | MIXER_WAVEOUT_VOLUME, val);
        break;
    case WAVEOUT_MICROPHONE :
        result = mixerWaveOut(nWaveDeviceID, MIXER_WAVEOUT_MICROPHONE | MIXER_WAVEOUT_GET | MIXER_WAVEOUT_VOLUME, val);
        break;
    }
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInSelected(IN INT32 nWaveDeviceID, IN MixerControl nControl)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = 1;
    switch(nControl)
    {
    case WAVEIN_WAVEOUT :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_WAVEOUT | MIXER_WAVEIN_SET | MIXER_WAVEIN_SELECTED, val);
        break;
    case WAVEIN_LINEIN :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_LINEIN | MIXER_WAVEIN_SET | MIXER_WAVEIN_SELECTED, val);
        break;
    case WAVEIN_MICROPHONE :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_SET | MIXER_WAVEIN_SELECTED, val);
        break;
    }
    return result;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInSelected(IN INT32 nWaveDeviceID, IN MixerControl nControl)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    switch(nControl)
    {
    case WAVEIN_WAVEOUT :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_WAVEOUT | MIXER_WAVEIN_GET | MIXER_WAVEIN_SELECTED, val);
        break;
    case WAVEIN_LINEIN :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_LINEIN | MIXER_WAVEIN_GET | MIXER_WAVEIN_SELECTED, val);
        break;
    case WAVEIN_MICROPHONE :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_GET | MIXER_WAVEIN_SELECTED, val);
        break;
    }
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInVolume(IN INT32 nWaveDeviceID, IN MixerControl nControl, IN INT32 nVolume)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = nVolume;
    switch(nControl)
    {
    case WAVEIN_WAVEOUT :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_WAVEOUT | MIXER_WAVEIN_SET | MIXER_WAVEIN_VOLUME, val);
        break;
    case WAVEIN_LINEIN :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_LINEIN | MIXER_WAVEIN_SET | MIXER_WAVEIN_VOLUME, val);
        break;
    case WAVEIN_MICROPHONE :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_SET | MIXER_WAVEIN_VOLUME, val);
        break;
    }
    return result;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInVolume(IN INT32 nWaveDeviceID, IN MixerControl nControl)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    switch(nControl)
    {
    case WAVEIN_WAVEOUT :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_WAVEOUT | MIXER_WAVEIN_GET | MIXER_WAVEIN_VOLUME, val);
        break;
    case WAVEIN_LINEIN :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_LINEIN | MIXER_WAVEIN_GET | MIXER_WAVEIN_VOLUME, val);
        break;
    case WAVEIN_MICROPHONE :
        result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_GET | MIXER_WAVEIN_VOLUME, val);
        break;
    }
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInBoost(IN INT32 nWaveDeviceID, IN TTBOOL bEnable)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = bEnable;
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_SET | MIXER_WAVEIN_BOOST, val);
    return result;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInBoost(IN INT32 nWaveDeviceID)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_GET | MIXER_WAVEIN_BOOST, val);
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInMute(IN INT32 nWaveDeviceID, IN TTBOOL bEnable)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = bEnable;
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_SET | MIXER_WAVEIN_MUTE, val);
    return result;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInMute(IN INT32 nWaveDeviceID)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_MICROPHONE | MIXER_WAVEIN_GET | MIXER_WAVEIN_MUTE, val);
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API INT32 TT_Mixer_GetWaveInControlCount(IN INT32 nWaveDeviceID)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_GETCOUNT, val);
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveInControlName(IN INT32 nWaveDeviceID, IN INT32 nControlIndex, OUT TTCHAR szDeviceName[TT_STRLEN])
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = nControlIndex;
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_BYINDEX | MIXER_WAVEIN_GET | MIXER_WAVEIN_NAME, val);
    ACE_OS::strsncpy(szDeviceName, val.name, TT_STRLEN);
    return result;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_SetWaveInControlSelected(IN INT32 nWaveDeviceID, IN INT32 nControlIndex)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = nControlIndex;
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_BYINDEX | MIXER_WAVEIN_SET | MIXER_WAVEIN_SELECTED, val);
    return result;
}

TEAMTALKDLL_API TTBOOL TT_Mixer_GetWaveInControlSelected(IN INT32 nWaveDeviceID, IN INT32 nControlIndex)
{
    BOOL result = FALSE;
    InOutValue val = {0};
    val.value = nControlIndex;
    result = mixerWaveIn(nWaveDeviceID, MIXER_WAVEIN_BYINDEX | MIXER_WAVEIN_GET | MIXER_WAVEIN_SELECTED, val);
    if(!result)
        return -1;
    return val.value;
}

TEAMTALKDLL_API TTBOOL TT_Firewall_IsEnabled()
{
    return WinFirewall(false).IsFirewallOn();
}

TEAMTALKDLL_API TTBOOL TT_Firewall_Enable(IN TTBOOL bEnable)
{
    return WinFirewall(WinFirewall::HasUAE()).EnableFirewall(bEnable);
}

TEAMTALKDLL_API TTBOOL TT_Firewall_AppExceptionExists(IN const TTCHAR* szExecutable)
{
    return WinFirewall(false).IsApplicationFirewalled(szExecutable);
}

TEAMTALKDLL_API TTBOOL TT_Firewall_AddAppException(IN const TTCHAR* szName, 
                                                 IN const TTCHAR* szExecutable)
{
    if(WinFirewall(WinFirewall::HasUAE()).AddException(szExecutable,
                                                       szName))
        return WinFirewall(false).IsApplicationFirewalled(szExecutable);
    return FALSE;
}

TEAMTALKDLL_API TTBOOL TT_Firewall_RemoveAppException(IN const TTCHAR* szExecutable)
{
    if(WinFirewall(false).IsApplicationFirewalled(szExecutable))
        return WinFirewall(WinFirewall::HasUAE()).RemoveException(szExecutable);
    return false;
}

#endif /* WIN32 */


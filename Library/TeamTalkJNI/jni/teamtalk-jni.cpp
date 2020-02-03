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

#include "ttconvert-jni.h"

#include <string.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

#include <jni.h>

extern "C" {

    JNIEXPORT jstring JNICALL Java_dk_bearware_TeamTalkBase_getVersion(JNIEnv* env,
                                                                       jclass)
    {
        const TTCHAR* ttv = TT_GetVersion();
        return NEW_JSTRING(env, ttv);
    }

    JNIEXPORT jlong JNICALL Java_dk_bearware_TeamTalkBase_initTeamTalkPoll(JNIEnv* env,
                                                                           jobject thiz)
    {
        return (jlong)TT_InitTeamTalkPoll();
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeTeamTalk(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance)
    {
        return TT_CloseTeamTalk(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMessage(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jobject pMsg,
                                                                        jint pnWaitMs)
    {
        THROW_NULLEX(env, pMsg, false);

        INT32 _pnWaitMs = pnWaitMs;
        TTMessage msg;
        TTBOOL b = TT_GetMessage(reinterpret_cast<TTInstance*>(lpTTInstance),
                                 &msg, &_pnWaitMs);
        if(b)
            setTTMessage(env, msg, pMsg);
        return b;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_pumpMessage(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jint nClientEvent,
                                                                         jint nIdentifier)
    {
        return TT_PumpMessage(reinterpret_cast<TTInstance*>(lpTTInstance),
                              (ClientEvent)nClientEvent, nIdentifier);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getFlags(JNIEnv* env,
                                                                  jobject thiz,
                                                                  jlong lpTTInstance)
    {
        return TT_GetFlags(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setLicenseInformation(JNIEnv* env,
                                                                                   jclass,
                                                                                   jstring szRegName,
                                                                                   jstring szRegKey)
    {
        THROW_NULLEX(env, szRegName, false);
        THROW_NULLEX(env, szRegKey, false);
        return TT_SetLicenseInformation(ttstr(env, szRegName), ttstr(env, szRegKey));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getDefaultSoundDevices(JNIEnv* env,
                                                                                    jclass,
                                                                                    jobject lpnInputDeviceID,
                                                                                    jobject lpnOutputDeviceID)
    {
        THROW_NULLEX(env, lpnInputDeviceID, false);
        THROW_NULLEX(env, lpnOutputDeviceID, false);

        int inputid, outputid;
        if(TT_GetDefaultSoundDevices(&inputid, &outputid))
        {
            setIntPtr(env, lpnInputDeviceID, inputid);
            setIntPtr(env, lpnOutputDeviceID, outputid);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getDefaultSoundDevicesEx(JNIEnv* env,
                                                                                      jclass,
                                                                                      jint nSndSystem,
                                                                                      jobject lpnInputDeviceID,
                                                                                      jobject lpnOutputDeviceID)
    {
        THROW_NULLEX(env, lpnInputDeviceID, false);
        THROW_NULLEX(env, lpnOutputDeviceID, false);

        int inputid, outputid;
        if(TT_GetDefaultSoundDevicesEx((SoundSystem)nSndSystem, &inputid, &outputid))
        {
            setIntPtr(env, lpnInputDeviceID, inputid);
            setIntPtr(env, lpnOutputDeviceID, outputid);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getSoundDevices(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jobjectArray lpSoundDevices,
                                                                             jobject lpnHowMany)
    {
        THROW_NULLEX(env, lpnHowMany, false);

        int howmany = 0;
        if(lpSoundDevices == NULL)
        {
            if(!TT_GetSoundDevices(NULL, &howmany))
                return false;
            setIntPtr(env, lpnHowMany, howmany);
            return true;
        }

        INT32 arr_size = (INT32)env->GetArrayLength(lpSoundDevices);
        std::vector<SoundDevice> devs((size_t)arr_size);
        if(arr_size && !TT_GetSoundDevices(&devs[0], &arr_size))
            return false;

        for(jsize i=0;i<arr_size;i++)
            env->SetObjectArrayElement(lpSoundDevices, i, newSoundDevice(env, devs[i]));

        setIntPtr(env, lpnHowMany, arr_size);

        return true;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_restartSoundSystem(JNIEnv* env,
                                                                                jclass)
    {
        return TT_RestartSoundSystem();
    }

    JNIEXPORT jlong JNICALL Java_dk_bearware_TeamTalkBase_startSoundLoopbackTest(JNIEnv* env,
                                                                                 jclass,
                                                                                 jint nInputDeviceID, 
                                                                                 jint nOutputDeviceID,
                                                                                 jint nSampleRate,
                                                                                 jint nChannels,
                                                                                 jboolean bDuplexMode,
                                                                                 jobject lpSpeexDSP)
    {
    
        TTSoundLoop* inst;
        if(lpSpeexDSP)
        {
            SpeexDSP spxdsp;
            ZERO_STRUCT(spxdsp);
            setSpeexDSP(env, spxdsp, lpSpeexDSP, J2N);
        
            inst = TT_StartSoundLoopbackTest(nInputDeviceID, nOutputDeviceID, 
                                             nSampleRate, nChannels, bDuplexMode, &spxdsp);
        }
        else
        {
            inst = TT_StartSoundLoopbackTest(nInputDeviceID, nOutputDeviceID, 
                                             nSampleRate, nChannels, bDuplexMode, NULL);
        }
        return reinterpret_cast<jlong>(inst);
    }
    
    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundLoopbackTest(JNIEnv* env,
                                                                                    jclass,
                                                                                    jlong lpTTSoundLoop)
    {
        return TT_CloseSoundLoopbackTest(reinterpret_cast<TTSoundLoop*>(lpTTSoundLoop));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundInputDevice(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jint nInputDeviceID)
    {
        return TT_InitSoundInputDevice(reinterpret_cast<TTInstance*>(lpTTInstance), nInputDeviceID);
                                   
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundOutputDevice(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jint nOutputDeviceID)
    {
        return TT_InitSoundOutputDevice(reinterpret_cast<TTInstance*>(lpTTInstance), nOutputDeviceID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundDuplexDevices(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint nInputDeviceID,
                                                                                    jint nOutputDeviceID)
    {
        return TT_InitSoundDuplexDevices(reinterpret_cast<TTInstance*>(lpTTInstance), nInputDeviceID, nOutputDeviceID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundInputDevice(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance)
    {
        return TT_CloseSoundInputDevice(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundOutputDevice(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance)
    {
        return TT_CloseSoundOutputDevice(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundDuplexDevices(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance)
    {
        return TT_CloseSoundDuplexDevices(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputLevel(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance)
    {
        return TT_GetSoundInputLevel(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundInputGainLevel(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint nLevel)
    {
        return TT_SetSoundInputGainLevel(reinterpret_cast<TTInstance*>(lpTTInstance),
                                         nLevel);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputGainLevel(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance)
    {
        return TT_GetSoundInputGainLevel(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundInputPreprocess(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jobject lpSpeexDSP)
    {
        THROW_NULLEX(env, lpSpeexDSP, false);

        SpeexDSP spxdsp;
        ZERO_STRUCT(spxdsp);
        setSpeexDSP(env, spxdsp, lpSpeexDSP, J2N);
        return TT_SetSoundInputPreprocess(reinterpret_cast<TTInstance*>(lpTTInstance), &spxdsp);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputPreprocess(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jobject lpSpeexDSP)
    {
        THROW_NULLEX(env, lpSpeexDSP, false);

        SpeexDSP spxdsp;
        ZERO_STRUCT(spxdsp);
        if(TT_GetSoundInputPreprocess(reinterpret_cast<TTInstance*>(lpTTInstance), &spxdsp))
        {
            setSpeexDSP(env, spxdsp, lpSpeexDSP, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundOutputVolume(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jint nVolume)
    {
        return TT_SetSoundOutputVolume(reinterpret_cast<TTInstance*>(lpTTInstance),
                                       nVolume);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getSoundOutputVolume(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance)
    {
        return TT_GetSoundOutputVolume(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundOutputMute(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance,
                                                                                jboolean bMuteAll)
    {
        return TT_SetSoundOutputMute(reinterpret_cast<TTInstance*>(lpTTInstance),
                                     bMuteAll);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableVoiceTransmission(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jboolean bEnable)
    {
        return TT_EnableVoiceTransmission(reinterpret_cast<TTInstance*>(lpTTInstance),
                                          bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enable3DSoundPositioning(JNIEnv* env,
                                                                                      jobject thiz,
                                                                                      jlong lpTTInstance,
                                                                                      jboolean bEnable)
    {
        return TT_Enable3DSoundPositioning(reinterpret_cast<TTInstance*>(lpTTInstance),
                                           bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_autoPositionUsers(JNIEnv* env,
                                                                               jobject thiz,
                                                                               jlong lpTTInstance)
    {
        return TT_AutoPositionUsers(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableAudioBlockEvent(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jint nUserID,
                                                                                   jint nStreamType,
                                                                                   jboolean bEnable)
    {
        return TT_EnableAudioBlockEvent(reinterpret_cast<TTInstance*>(lpTTInstance),
                                        nUserID, (StreamType)nStreamType, bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_insertAudioBlock(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance,
                                                                              jobject lpAudioBlock)
    {
        AudioBlock ab = {};
        auto byteArr = setAudioBlock(env, ab, lpAudioBlock, J2N);
        jboolean b = TT_InsertAudioBlock(reinterpret_cast<TTInstance*>(lpTTInstance), &ab);
        if (byteArr)
            env->ReleaseByteArrayElements(byteArr, reinterpret_cast<jbyte*>(ab.lpRawAudio), JNI_ABORT);
        return b;
    }
    
    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableVoiceActivation(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jboolean bEnable)
    {
        return TT_EnableVoiceActivation(reinterpret_cast<TTInstance*>(lpTTInstance),
                                        bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setVoiceActivationLevel(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jint nLevel)
    {
        return TT_SetVoiceActivationLevel(reinterpret_cast<TTInstance*>(lpTTInstance),
                                          nLevel);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getVoiceActivationLevel(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance)
    {
        return TT_GetVoiceActivationLevel(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setVoiceActivationStopDelay(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nDelayMSec)
    {
        return TT_SetVoiceActivationStopDelay(reinterpret_cast<TTInstance*>(lpTTInstance),
                                              nDelayMSec);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getVoiceActivationStopDelay(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance)
    {
        return TT_GetVoiceActivationStopDelay(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_startRecordingMuxedAudioFile(JNIEnv* env,
                                                                                          jobject thiz,
                                                                                          jlong lpTTInstance,
                                                                                          jobject lpAudioCodec,
                                                                                          jstring szAudioFileName,
                                                                                          jint uAFF)
    {
        THROW_NULLEX(env, lpAudioCodec, false);
        THROW_NULLEX(env, szAudioFileName, false);

        AudioCodec audcodec;
        ZERO_STRUCT(audcodec);
        setAudioCodec(env, audcodec, lpAudioCodec, J2N);
        
        return TT_StartRecordingMuxedAudioFile(reinterpret_cast<TTInstance*>(lpTTInstance),
                                               &audcodec, ttstr(env, szAudioFileName), (AudioFileFormat)uAFF);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_startRecordingMuxedAudioFileEx(JNIEnv* env,
                                                                                            jobject thiz,
                                                                                            jlong lpTTInstance,
                                                                                            jint nChannelID,
                                                                                            jstring szAudioFileName,
                                                                                            jint uAFF)
    {
        THROW_NULLEX(env, szAudioFileName, false);

        return TT_StartRecordingMuxedAudioFileEx(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                 nChannelID, ttstr(env, szAudioFileName), (AudioFileFormat)uAFF);
    }
    
    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopRecordingMuxedAudioFile(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance)
    {
        return TT_StopRecordingMuxedAudioFile(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopRecordingMuxedAudioFileEx(JNIEnv* env,
                                                                                           jobject thiz,
                                                                                           jlong lpTTInstance,
                                                                                           jint nChannelID)
    {
        return TT_StopRecordingMuxedAudioFileEx(reinterpret_cast<TTInstance*>(lpTTInstance), nChannelID);
    }
    
    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_startVideoCaptureTransmission(JNIEnv* env,
                                                                                           jobject thiz,
                                                                                           jlong lpTTInstance,
                                                                                           jobject lpVideoCodec)
    {
        THROW_NULLEX(env, lpVideoCodec, false);

        VideoCodec vidcodec;
        ZERO_STRUCT(vidcodec);
        setVideoCodec(env, vidcodec, lpVideoCodec, J2N);
        return TT_StartVideoCaptureTransmission(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                &vidcodec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopVideoCaptureTransmission(JNIEnv* env,
                                                                                          jobject thiz,
                                                                                          jlong lpTTInstance)
    {
        return TT_StopVideoCaptureTransmission(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getVideoCaptureDevices(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jobjectArray lpVideoDevices,
                                                                                    jobject lpnHowMany)
    {
        THROW_NULLEX(env, lpnHowMany, false);

        int howmany = 0;
        if(lpVideoDevices == NULL)
        {
            if(!TT_GetVideoCaptureDevices(NULL, &howmany))
                return false;
            setIntPtr(env, lpnHowMany, howmany);
            return true;
        }

        INT32 arr_size = env->GetArrayLength(lpVideoDevices);
        std::vector<VideoCaptureDevice> devs((size_t)arr_size);
        if(arr_size && !TT_GetVideoCaptureDevices(&devs[0], &arr_size))
            return false;

        for(jsize i=0;i<arr_size;i++)
            env->SetObjectArrayElement(lpVideoDevices, i, newVideoDevice(env, devs[i]));

        setIntPtr(env, lpnHowMany, arr_size);

        return true;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initVideoCaptureDevice(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jstring szDeviceID,
                                                                                   jobject lpVideoFormat)
    {
        VideoFormat fmt;
        ZERO_STRUCT(fmt);
        setVideoFormat(env, fmt, lpVideoFormat, J2N);
        
        return TT_InitVideoCaptureDevice(reinterpret_cast<TTInstance*>(lpTTInstance),
                                         ttstr(env, szDeviceID), &fmt);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeVideoCaptureDevice(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance)
    {
        return TT_CloseVideoCaptureDevice(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserVideoCaptureFrame(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nUserID)
    {
        VideoFrame* vidframe = TT_AcquireUserVideoCaptureFrame(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                               nUserID);
        if(!vidframe)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/VideoFrame");
        jobject vidframe_obj = newObject(env, cls);
        setVideoFrame(env, *vidframe, vidframe_obj);

        TT_ReleaseUserVideoCaptureFrame(reinterpret_cast<TTInstance*>(lpTTInstance), vidframe);
        return vidframe_obj;
    }

// ReleaseUserVideoCaptureFrame()

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_startStreamingMediaFileToChannel(JNIEnv* env,
                                                                                              jobject thiz,
                                                                                              jlong lpTTInstance,
                                                                                              jstring szMediaFilePath,
                                                                                              jobject lpVideoCodec)
    {
        THROW_NULLEX(env, szMediaFilePath, false);

        VideoCodec vidcodec = {};
        vidcodec.nCodec = NO_CODEC;
        if(lpVideoCodec)
            setVideoCodec(env, vidcodec, lpVideoCodec, J2N);
        return TT_StartStreamingMediaFileToChannel(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                   ttstr(env, szMediaFilePath), &vidcodec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_startStreamingMediaFileToChannelEx(JNIEnv* env,
                                                                                                jobject thiz,
                                                                                                jlong lpTTInstance,
                                                                                                jstring szMediaFilePath,
                                                                                                jobject lpMediaFilePlayback,
                                                                                                jobject lpVideoCodec)
    {
        THROW_NULLEX(env, szMediaFilePath, false);
        THROW_NULLEX(env, lpMediaFilePlayback, false);

        MediaFilePlayback mfp = {};
        if (lpMediaFilePlayback)
            setMediaFilePlayback(env, mfp, lpMediaFilePlayback, J2N);

        VideoCodec vidcodec = {};
        vidcodec.nCodec = NO_CODEC;
        if(lpVideoCodec)
            setVideoCodec(env, vidcodec, lpVideoCodec, J2N);
        return TT_StartStreamingMediaFileToChannelEx(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                     ttstr(env, szMediaFilePath), &mfp, &vidcodec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_updateStreamingMediaFileToChannel(JNIEnv* env,
                                                                                               jobject thiz,
                                                                                               jlong lpTTInstance,
                                                                                               jobject lpMediaFilePlayback,
                                                                                               jobject lpVideoCodec)
    {
        THROW_NULLEX(env, lpMediaFilePlayback, false);

        MediaFilePlayback mfp = {};
        if (lpMediaFilePlayback)
            setMediaFilePlayback(env, mfp, lpMediaFilePlayback, J2N);
        
        VideoCodec vidcodec = {};
        vidcodec.nCodec = NO_CODEC;
        if (lpVideoCodec)
            setVideoCodec(env, vidcodec, lpVideoCodec, J2N);
        
        return TT_UpdateStreamingMediaFileToChannel(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                    &mfp, &vidcodec);
    }
    
    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopStreamingMediaFileToChannel(JNIEnv* env,
                                                                                             jobject thiz,
                                                                                             jlong lpTTInstance)
    {
        return TT_StopStreamingMediaFileToChannel(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_initLocalPlayback(JNIEnv* env, jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jstring szMediaFilePath,
                                                                           jobject lpMediaFilePlayback) {
        THROW_NULLEX(env, szMediaFilePath, 0);
        THROW_NULLEX(env, lpMediaFilePlayback, 0);

        MediaFilePlayback playback = {};
        setMediaFilePlayback(env, playback, lpMediaFilePlayback, J2N);
            
        return TT_InitLocalPlayback(reinterpret_cast<TTInstance*>(lpTTInstance),
                                    ttstr(env, szMediaFilePath), &playback);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_updateLocalPlayback(JNIEnv* env, jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jint nPlaybackSessionID,
                                                                                 jobject lpMediaFilePlayback) {
        
        THROW_NULLEX(env, lpMediaFilePlayback, false);

        MediaFilePlayback playback = {};
        setMediaFilePlayback(env, playback, lpMediaFilePlayback, J2N);

        return TT_UpdateLocalPlayback(reinterpret_cast<TTInstance*>(lpTTInstance),
                                      nPlaybackSessionID, &playback);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopLocalPlayback(JNIEnv* env, jobject thiz,
                                                                               jlong lpTTInstance,
                                                                               jint nPlaybackSessionID) {
        return TT_StopLocalPlayback(reinterpret_cast<TTInstance*>(lpTTInstance),
                                    nPlaybackSessionID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMediaFileInfo(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jstring szMediaFilePath,
                                                                              jobject lpMediaFileInfo)
    {
        THROW_NULLEX(env, szMediaFilePath, false);
        THROW_NULLEX(env, lpMediaFileInfo, false);

        MediaFileInfo mfi;
        if(TT_GetMediaFileInfo(ttstr(env, szMediaFilePath), &mfi))
        {
            setMediaFileInfo(env, mfi, lpMediaFileInfo, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserMediaVideoFrame(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jint nUserID)
    {
        VideoFrame* vidframe = TT_AcquireUserMediaVideoFrame(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                             nUserID);

        if(!vidframe)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/VideoFrame");
        jobject vidframe_obj = newObject(env, cls);
        setVideoFrame(env, *vidframe, vidframe_obj);

        TT_ReleaseUserMediaVideoFrame(reinterpret_cast<TTInstance*>(lpTTInstance), vidframe);
        return vidframe_obj;

    }

/*
    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_releaseUserMediaVideoFrame(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jobject lpVideoFrame)
    {
        return NULL;
    }
*/
    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_sendDesktopWindow(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jobject lpDesktopWindow,
                                                                           jint nConvertBmpFormat)
    {
        DesktopWindow wnd;
        setDesktopWindow(env, wnd, lpDesktopWindow, J2N);
        
        jclass cls = env->GetObjectClass(lpDesktopWindow);
        jfieldID fid_frmbuf = env->GetFieldID(cls, "frameBuffer", "[B");
        assert(fid_frmbuf);

        jbyteArray buf = (jbyteArray)env->GetObjectField(lpDesktopWindow, fid_frmbuf);
        jbyte* bufptr = env->GetByteArrayElements(buf, 0);
        if(!bufptr)
            return -1;

        wnd.frameBuffer = bufptr;
        wnd.nFrameBufferSize = env->GetArrayLength(buf);
        
        jint ret = TT_SendDesktopWindow(reinterpret_cast<TTInstance*>(lpTTInstance),
                                        &wnd, (BitmapFormat)nConvertBmpFormat);
        
       env->ReleaseByteArrayElements(buf, bufptr, 0);
       return ret;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeDesktopWindow(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance)
    {
        return TT_CloseDesktopWindow(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

//TODO: Palette_GetColorTable

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_sendDesktopCursorPosition(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jint nPosX, 
                                                                                       jint nPosY)
    {
        return TT_SendDesktopCursorPosition(reinterpret_cast<TTInstance*>(lpTTInstance),
                                            (UINT16)nPosX, (UINT16)nPosY);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_sendDesktopInput(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance,
                                                                              jint nUserID,
                                                                              jobjectArray lpDesktopInputs)
    {
        THROW_NULLEX(env, lpDesktopInputs, false);

        jsize len = env->GetArrayLength(lpDesktopInputs);
        std::vector<DesktopInput> inputs(len);
        for(jsize i=0;i<len;i++)
            setDesktopInput(env, inputs[i], env->GetObjectArrayElement(lpDesktopInputs, i), J2N);
        return TT_SendDesktopInput(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID,
                                   &inputs[0], len);
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserDesktopWindow(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jint nUserID) {
        DesktopWindow* deskwnd = TT_AcquireUserDesktopWindow(reinterpret_cast<TTInstance*>(lpTTInstance), 
                                                             nUserID);
        if(!deskwnd)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/DesktopWindow");
        jobject deskwnd_obj = newObject(env, cls);
        setDesktopWindow(env, *deskwnd, deskwnd_obj, N2J);

        TT_ReleaseUserDesktopWindow(reinterpret_cast<TTInstance*>(lpTTInstance), deskwnd);
        return deskwnd_obj;
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserDesktopWindowEx(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jint nUserID,
                                                                                       jint nBitmapFormat) {
        DesktopWindow* deskwnd = TT_AcquireUserDesktopWindowEx(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                               nUserID, (BitmapFormat)nBitmapFormat);
        if(!deskwnd)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/DesktopWindow");
        jobject deskwnd_obj = newObject(env, cls);
        setDesktopWindow(env, *deskwnd, deskwnd_obj, N2J);

        TT_ReleaseUserDesktopWindow(reinterpret_cast<TTInstance*>(lpTTInstance), deskwnd);
        return deskwnd_obj;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_connect(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jstring szHostAddress,
                                                                     jint nTcpPort, 
                                                                     jint nUdpPort, 
                                                                     jint nLocalTcpPort, 
                                                                     jint nLocalUdpPort,
                                                                     jboolean bEncrypted)
    {
        THROW_NULLEX(env, szHostAddress, false);

        return TT_Connect(reinterpret_cast<TTInstance*>(lpTTInstance),
                          ttstr(env,szHostAddress), nTcpPort, nUdpPort, 
                          nLocalTcpPort, nLocalUdpPort, bEncrypted);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_connectSysID(JNIEnv* env,
                                                                          jobject thiz,
                                                                          jlong lpTTInstance,
                                                                          jstring szHostAddress,
                                                                          jint nTcpPort, 
                                                                          jint nUdpPort, 
                                                                          jint nLocalTcpPort, 
                                                                          jint nLocalUdpPort,
                                                                          jboolean bEncrypted,
                                                                          jstring szSystemID)
    {
        THROW_NULLEX(env, szHostAddress, false);
        THROW_NULLEX(env, szSystemID, false);

        return TT_ConnectSysID(reinterpret_cast<TTInstance*>(lpTTInstance),
                               ttstr(env,szHostAddress), nTcpPort, nUdpPort, 
                               nLocalTcpPort, nLocalUdpPort, bEncrypted, 
                               ttstr(env, szSystemID));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_connectEx(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jstring szHostAddress,
                                                                       jint nTcpPort,
                                                                       jint nUdpPort,
                                                                       jstring szBindIPAddr,
                                                                       jint nLocalTcpPort,
                                                                       jint nLocalUdpPort,
                                                                       jboolean bEncrypted)
    {
        THROW_NULLEX(env, szHostAddress, false);
        THROW_NULLEX(env, szBindIPAddr, false);

        return TT_ConnectEx(reinterpret_cast<TTInstance*>(lpTTInstance),
                            ttstr(env, szHostAddress), nTcpPort, nUdpPort,
                            ttstr(env, szBindIPAddr), nLocalTcpPort, nLocalUdpPort,
                            bEncrypted);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_disconnect(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance)

    {
        return TT_Disconnect(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_queryMaxPayload(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jint nUserID)
    {
        return TT_QueryMaxPayload(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getClientStatistics(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jobject lpClientStatistics)
    {
        THROW_NULLEX(env, lpClientStatistics, false);

        ClientStatistics stats;
        if(TT_GetClientStatistics(reinterpret_cast<TTInstance*>(lpTTInstance),
                                  &stats))
        {
            setClientStatistics(env, stats, lpClientStatistics);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setClientKeepAlive(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance,
                                                                                jobject lpClientKeepAlive)
    {
        THROW_NULLEX(env, lpClientKeepAlive, false);

        ClientKeepAlive ka;
        setClientKeepAlive(env, ka, lpClientKeepAlive, J2N);
        
        return TT_SetClientKeepAlive(reinterpret_cast<TTInstance*>(lpTTInstance), &ka);
    }
    
    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getClientKeepAlive(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance,
                                                                                jobject lpClientKeepAlive)
    {
        THROW_NULLEX(env, lpClientKeepAlive, false);

        ClientKeepAlive ka = {};
        if (TT_GetClientKeepAlive(reinterpret_cast<TTInstance*>(lpTTInstance), &ka)) {
            setClientKeepAlive(env, ka, lpClientKeepAlive, N2J);
            return true;
        }

        return false;
    }
    
    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doPing(JNIEnv* env,
                                                                jobject thiz,
                                                                jlong lpTTInstance)
    {
        return TT_DoPing(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doLogin(JNIEnv* env,
                                                                 jobject thiz,
                                                                 jlong lpTTInstance,
                                                                 jstring szNickname, 
                                                                 jstring szUsername,
                                                                 jstring szPassword)
    {
        THROW_NULLEX(env, szNickname, -1);
        THROW_NULLEX(env, szUsername, -1);
        THROW_NULLEX(env, szPassword, -1);

        return TT_DoLogin(reinterpret_cast<TTInstance*>(lpTTInstance), 
                          ttstr(env, szNickname), ttstr(env, szUsername), 
                          ttstr(env, szPassword));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doLoginEx(JNIEnv* env,
                                                                   jobject thiz,
                                                                   jlong lpTTInstance,
                                                                   jstring szNickname, 
                                                                   jstring szUsername,
                                                                   jstring szPassword,
                                                                   jstring szClientName)
    {
        THROW_NULLEX(env, szNickname, -1);
        THROW_NULLEX(env, szUsername, -1);
        THROW_NULLEX(env, szPassword, -1);
        THROW_NULLEX(env, szClientName, -1);

        return TT_DoLoginEx(reinterpret_cast<TTInstance*>(lpTTInstance), 
                            ttstr(env, szNickname), ttstr(env, szUsername), 
                            ttstr(env, szPassword), ttstr(env, szClientName));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doLogout(JNIEnv* env,
                                                                  jobject thiz,
                                                                  jlong lpTTInstance)
    {
        return TT_DoLogout(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doJoinChannel(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, -1);

        Channel chan = {};
        setChannel(env, chan, lpChannel, J2N);
        return TT_DoJoinChannel(reinterpret_cast<TTInstance*>(lpTTInstance), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doJoinChannelByID(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jint nChannelID, 
                                                                           jstring szPassword)
    {
        THROW_NULLEX(env, szPassword, -1);

        return TT_DoJoinChannelByID(reinterpret_cast<TTInstance*>(lpTTInstance),
                                    nChannelID, ttstr(env, szPassword));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doLeaveChannel(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance)
    {
        return TT_DoLeaveChannel(reinterpret_cast<TTInstance*>(lpTTInstance));
                             
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChangeNickname(JNIEnv* env,
                                                                          jobject thiz,
                                                                          jlong lpTTInstance,
                                                                          jstring szNewNick)
    {
        THROW_NULLEX(env, szNewNick, -1);

        return TT_DoChangeNickname(reinterpret_cast<TTInstance*>(lpTTInstance),
                                   ttstr(env, szNewNick));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChangeStatus(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jint nStatusMode,
                                                                        jstring szStatusMessage)
    {
        THROW_NULLEX(env, szStatusMessage, -1);

        return TT_DoChangeStatus(reinterpret_cast<TTInstance*>(lpTTInstance),
                                 nStatusMode, ttstr(env, szStatusMessage));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doTextMessage(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jobject lpTextMessage)
    {
        THROW_NULLEX(env, lpTextMessage, -1);

        TextMessage msg;
        setTextMessage(env, msg, lpTextMessage, J2N);
        return TT_DoTextMessage(reinterpret_cast<TTInstance*>(lpTTInstance), &msg);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChannelOp(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jint nChannelID,
                                                                     jboolean bMakeOperator)
    {
        return TT_DoChannelOp(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, nChannelID, bMakeOperator);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChannelOpEx(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jint nUserID,
                                                                       jint nChannelID,
                                                                       jstring szOpPassword,
                                                                       jboolean bMakeOperator)
    {
        THROW_NULLEX(env, szOpPassword, -1);

        return TT_DoChannelOpEx(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, 
                                nChannelID, ttstr(env, szOpPassword), bMakeOperator);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doKickUser(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nUserID,
                                                                    jint nChannelID)
    {
        return TT_DoKickUser(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, 
                             nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doSendFile(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nChannelID,
                                                                    jstring szLocalFilePath)
    {
        THROW_NULLEX(env, szLocalFilePath, -1);

        return TT_DoSendFile(reinterpret_cast<TTInstance*>(lpTTInstance), nChannelID,
                             ttstr(env, szLocalFilePath));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doRecvFile(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nChannelID,
                                                                    jint nFileID,
                                                                    jstring szLocalFilePath)
    {
        THROW_NULLEX(env, szLocalFilePath, -1);

        return TT_DoRecvFile(reinterpret_cast<TTInstance*>(lpTTInstance), nChannelID, nFileID,
                             ttstr(env, szLocalFilePath));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doDeleteFile(JNIEnv* env,
                                                                      jobject thiz,
                                                                      jlong lpTTInstance,
                                                                      jint nChannelID,
                                                                      jint nFileID)
    {
        return TT_DoDeleteFile(reinterpret_cast<TTInstance*>(lpTTInstance), nChannelID, nFileID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doSubscribe(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jint uSubscriptions)
    {
        return TT_DoSubscribe(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, uSubscriptions);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUnsubscribe(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jint nUserID,
                                                                       jint uSubscriptions)
    {
        return TT_DoUnsubscribe(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, uSubscriptions);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doMakeChannel(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, -1);

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TT_DoMakeChannel(reinterpret_cast<TTInstance*>(lpTTInstance), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUpdateChannel(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, -1);

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TT_DoUpdateChannel(reinterpret_cast<TTInstance*>(lpTTInstance), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doRemoveChannel(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jint nChannelID)
    {
        return TT_DoRemoveChannel(reinterpret_cast<TTInstance*>(lpTTInstance), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doMoveUser(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nUserID,
                                                                    jint nChannelID)
    {
        return TT_DoMoveUser(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUpdateServer(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jobject lpServerProperties)
    {
        THROW_NULLEX(env, lpServerProperties, -1);

        ServerProperties srvprop;
        setServerProperties(env, srvprop, lpServerProperties, J2N);
        return TT_DoUpdateServer(reinterpret_cast<TTInstance*>(lpTTInstance), &srvprop);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doListUserAccounts(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance,
                                                                            jint nIndex, 
                                                                            jint nCount)
    {
        return TT_DoListUserAccounts(reinterpret_cast<TTInstance*>(lpTTInstance), nIndex, nCount);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doNewUserAccount(JNIEnv* env,
                                                                          jobject thiz,
                                                                          jlong lpTTInstance,
                                                                          jobject lpUserAccount)
    {
        THROW_NULLEX(env, lpUserAccount, -1);

        UserAccount account;
        setUserAccount(env, account, lpUserAccount, J2N);
        return TT_DoNewUserAccount(reinterpret_cast<TTInstance*>(lpTTInstance), &account);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doDeleteUserAccount(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jstring szUsername)
    {
        THROW_NULLEX(env, szUsername, -1);

        return TT_DoDeleteUserAccount(reinterpret_cast<TTInstance*>(lpTTInstance), ttstr(env, szUsername));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBanUser(JNIEnv* env,
                                                                   jobject thiz,
                                                                   jlong lpTTInstance,
                                                                   jint nUserID,
                                                                   jint nChannelID)
    {
        return TT_DoBanUser(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBanUserEx(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jint uBanTypes)
    {
        return TT_DoBanUserEx(reinterpret_cast<TTInstance*>(lpTTInstance), nUserID, uBanTypes);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBan(JNIEnv* env,
                                                               jobject thiz,
                                                               jlong lpTTInstance,
                                                               jobject lpBannedUser)
    {
        BannedUser ban;
        setBannedUser(env, ban, lpBannedUser, J2N);
        return TT_DoBan(reinterpret_cast<TTInstance*>(lpTTInstance), &ban);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBanIPAddress(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jstring szIPAddress,
                                                                        jint nChannelID)
    {
        THROW_NULLEX(env, szIPAddress, -1);

        return TT_DoBanIPAddress(reinterpret_cast<TTInstance*>(lpTTInstance), 
                                 ttstr(env, szIPAddress), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUnBanUser(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jstring szIPAddress,
                                                                     jint nChannelID)
    {
        THROW_NULLEX(env, szIPAddress, -1);

        return TT_DoUnBanUser(reinterpret_cast<TTInstance*>(lpTTInstance), 
                              ttstr(env, szIPAddress), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUnBanUserEx(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jobject lpBannedUser)
    {
        THROW_NULLEX(env, lpBannedUser, -1);
        BannedUser ban;
        setBannedUser(env, ban, lpBannedUser, J2N);
        return TT_DoUnBanUserEx(reinterpret_cast<TTInstance*>(lpTTInstance), &ban);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doListBans(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nChannelID,
                                                                    jint nIndex,
                                                                    jint nCount)
    {
        return TT_DoListBans(reinterpret_cast<TTInstance*>(lpTTInstance), nChannelID,
                             nIndex, nCount);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doSaveConfig(JNIEnv* env,
                                                                      jobject thiz,
                                                                      jlong lpTTInstance)
    {
        return TT_DoSaveConfig(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doQueryServerStats(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance)
    {
        return TT_DoQueryServerStats(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doQuit(JNIEnv* env,
                                                                jobject thiz,
                                                                jlong lpTTInstance)
    {
        return TT_DoQuit(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getServerProperties(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jobject lpServerProperties)
    {
        THROW_NULLEX(env, lpServerProperties, false);

        ServerProperties srvprop;
        if(TT_GetServerProperties(reinterpret_cast<TTInstance*>(lpTTInstance),
                                  &srvprop))
        {
            setServerProperties(env, srvprop, lpServerProperties, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getServerUsers(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance,
                                                                            jobjectArray lpUsers,
                                                                            jobject lpnHowMany)
    {
        THROW_NULLEX(env, lpnHowMany, false);

        INT32 n_users = 0;
        if(!lpUsers)
        {
            if(TT_GetServerUsers(reinterpret_cast<TTInstance*>(lpTTInstance),
                                 NULL, &n_users))
            {
                setIntPtr(env, lpnHowMany, n_users);
                return true;
            }
            return false;
        }
    
        n_users = getIntPtr(env, lpnHowMany);
        std::vector<User> users(n_users);

        if(n_users>0 &&
           TT_GetServerUsers(reinterpret_cast<TTInstance*>(lpTTInstance), &users[0],
                             &n_users))
        {
            n_users = std::min(n_users, (INT32)getIntPtr(env, lpnHowMany));
            setIntPtr(env, lpnHowMany, n_users);

            for(jsize i=0;i<(size_t)n_users;i++)
            {
                jclass cls = env->FindClass("dk/bearware/User");
                jobject user_obj = newObject(env, cls);
                setUser(env, users[i], user_obj);
                env->SetObjectArrayElement(lpUsers, i, user_obj);
            }
            return true;
        }
        return false;
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getRootChannelID(JNIEnv* env,
                                                                          jobject thiz,
                                                                          jlong lpTTInstance)
    {
        return TT_GetRootChannelID(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getMyChannelID(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance)
    {
        return TT_GetMyChannelID(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getChannel(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jint nChannelID,
                                                                        jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, false);

        Channel chan;
        if(TT_GetChannel(reinterpret_cast<TTInstance*>(lpTTInstance),
                         nChannelID, &chan))
        {
            setChannel(env, chan, lpChannel, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jstring JNICALL Java_dk_bearware_TeamTalkBase_getChannelPath(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jint nChannelID)
    {
        TTCHAR channel[TT_STRLEN] = {};
        TT_GetChannelPath(reinterpret_cast<TTInstance*>(lpTTInstance),
                          nChannelID, channel);
        return NEW_JSTRING(env, channel);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getChannelIDFromPath(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance,
                                                                              jstring szChannelPath)
    {
        THROW_NULLEX(env, szChannelPath, -1);

        ttstr channel(env, szChannelPath);
        return TT_GetChannelIDFromPath(reinterpret_cast<TTInstance*>(lpTTInstance),
                                       channel);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getChannelUsers(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jint nChannelID,
                                                                             jobjectArray lpUsers,
                                                                             jobject lpnHowMany)
    {
        THROW_NULLEX(env, lpnHowMany, false);

        INT32 n_users = 0;
        if(!lpUsers)
        {
            if(TT_GetChannelUsers(reinterpret_cast<TTInstance*>(lpTTInstance), 
                                  nChannelID, NULL, &n_users))
            {
                setIntPtr(env, lpnHowMany, n_users);
                return true;
            }
            return false;
        }
    
        n_users = getIntPtr(env, lpnHowMany);
        std::vector<User> users(n_users);

        if(n_users>0 &&
           TT_GetChannelUsers(reinterpret_cast<TTInstance*>(lpTTInstance), 
                              nChannelID, &users[0], &n_users))
        {
            n_users = std::min(n_users, (INT32)getIntPtr(env, lpnHowMany));
            setIntPtr(env, lpnHowMany, n_users);
            for(jsize i=0;i<(size_t)n_users;i++)
            {
                jclass cls = env->FindClass("dk/bearware/User");
                jobject user_obj = newObject(env, cls);
                setUser(env, users[i], user_obj);
                env->SetObjectArrayElement(lpUsers, i, user_obj);
            }

            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getChannelFiles(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jint nChannelID,
                                                                             jobjectArray lpRemoteFiles,
                                                                             jobject lpnHowMany)
    {
        THROW_NULLEX(env, lpnHowMany, false);

        INT32 n_files = 0;
        if(!lpRemoteFiles)
        {
            if(TT_GetChannelFiles(reinterpret_cast<TTInstance*>(lpTTInstance), 
                                  nChannelID, NULL, &n_files))
            {
                setIntPtr(env, lpnHowMany, n_files);
                return true;
            }
            return false;
        }
    
        n_files = getIntPtr(env, lpnHowMany);
        std::vector<RemoteFile> files(n_files);

        if(n_files>0 &&
           TT_GetChannelFiles(reinterpret_cast<TTInstance*>(lpTTInstance), 
                              nChannelID, &files[0], &n_files))
        {
            n_files = std::min(n_files, (INT32)getIntPtr(env, lpnHowMany));
            if(n_files>0)
            {
                std::vector<jobject> jfiles(n_files);
                jclass cls = env->FindClass("dk/bearware/RemoteFile");
                for(jsize i=0;i<(size_t)n_files;i++)
                {
                    jobject file_obj = newObject(env, cls);
                    setRemoteFile(env, files[i], file_obj, N2J);
                    env->SetObjectArrayElement(lpRemoteFiles, i, file_obj);
                }
            }
            setIntPtr(env, lpnHowMany, n_files);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getChannelRemoteFile(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jint nChannelID,
                                                                                  jint nFileID,
                                                                                  jobject lpRemoteFile)
    {
        THROW_NULLEX(env, lpRemoteFile, false);

        RemoteFile finfo;
        if(TT_GetChannelFile(reinterpret_cast<TTInstance*>(lpTTInstance), 
                             nChannelID, nFileID, &finfo))
        {
            setRemoteFile(env, finfo, lpRemoteFile, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_isChannelOperator(JNIEnv* env,
                                                                               jobject thiz,
                                                                               jlong lpTTInstance,
                                                                               jint nUserID,
                                                                               jint nChannelID)
    {
        return TT_IsChannelOperator(reinterpret_cast<TTInstance*>(lpTTInstance), 
                                    nUserID, nChannelID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getServerChannels(JNIEnv* env,
                                                                               jobject thiz,
                                                                               jlong lpTTInstance,
                                                                               jobjectArray lpChannels,
                                                                               jobject lpnHowMany)
    {
        THROW_NULLEX(env, lpnHowMany, false);

        int n_channels = 0;
        if(!lpChannels)
        {
            if(TT_GetServerChannels(reinterpret_cast<TTInstance*>(lpTTInstance),
                                    NULL, &n_channels))
            {
                setIntPtr(env, lpnHowMany, n_channels);
                return true;
            }
            return false;
        }
    
        n_channels = getIntPtr(env, lpnHowMany);
        std::vector<Channel> channels(n_channels);

        if(n_channels>0 &&
           TT_GetServerChannels(reinterpret_cast<TTInstance*>(lpTTInstance), &channels[0],
                                &n_channels))
        {
            n_channels = std::min(n_channels, (INT32)getIntPtr(env, lpnHowMany));
            setIntPtr(env, lpnHowMany, n_channels);
            jclass cls = env->FindClass("dk/bearware/Channel");
            for(jsize i=0;i<jsize(n_channels);i++)
            {
                jobject chan_obj = newObject(env, cls);
                setChannel(env, channels[i], chan_obj, N2J);
                env->SetObjectArrayElement(lpChannels, i, chan_obj);
            }
            return true;
        }
        return false;
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getMyUserID(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance)
    {
        return TT_GetMyUserID(reinterpret_cast<TTInstance*>(lpTTInstance));
    }                                                                           

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMyUserAccount(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance,
                                                                              jobject lpUserAccount)
    {
        THROW_NULLEX(env, lpUserAccount, false);

        UserAccount account;
        if(TT_GetMyUserAccount(reinterpret_cast<TTInstance*>(lpTTInstance), &account))
        {
            setUserAccount(env, account, lpUserAccount, N2J);
            return true;
        }
        return false;
    }                                                                           

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMyUserType(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance)
    {
        return TT_GetMyUserType(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMyUserData(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance)
    {
        return TT_GetMyUserData(reinterpret_cast<TTInstance*>(lpTTInstance));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getUser(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jobject lpUser)
    {
        THROW_NULLEX(env, lpUser, false);

        User user;
        if(TT_GetUser(reinterpret_cast<TTInstance*>(lpTTInstance),
                      nUserID, &user))
        {
            setUser(env, user, lpUser);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getUserStatistics(JNIEnv* env,
                                                                               jobject thiz,
                                                                               jlong lpTTInstance,
                                                                               jint nUserID,
                                                                               jobject lpUserStatistics)
    {
        THROW_NULLEX(env, lpUserStatistics, false);

        UserStatistics stats;
        if(TT_GetUserStatistics(reinterpret_cast<TTInstance*>(lpTTInstance),
                                nUserID, &stats))
        {
            setUserStatistics(env, stats, lpUserStatistics);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getUserByUsername(JNIEnv* env,
                                                                               jobject thiz,
                                                                               jlong lpTTInstance,
                                                                               jstring szUsername,
                                                                               jobject lpUser)
    {
        THROW_NULLEX(env, lpUser, false);

        User user;
        if(TT_GetUserByUsername(reinterpret_cast<TTInstance*>(lpTTInstance),
                                ttstr(env, szUsername), &user))
        {
            setUser(env, user, lpUser);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserVolume(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jint nUserID,
                                                                           jint nStreamType,
                                                                           jint nVolume) 
    {
        return TT_SetUserVolume(reinterpret_cast<TTInstance*>(lpTTInstance),
                                nUserID, (StreamType)nStreamType, nVolume);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserMute(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jint nUserID,
                                                                         jint nStreamType,
                                                                         jboolean bMute)
    {
        return TT_SetUserMute(reinterpret_cast<TTInstance*>(lpTTInstance),
                              nUserID, (StreamType)nStreamType, bMute);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserStoppedPlaybackDelay(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nUserID,
                                                                                         jint nStreamType,
                                                                                         jint nDelayMSec)
    {
        return TT_SetUserStoppedPlaybackDelay(reinterpret_cast<TTInstance*>(lpTTInstance),
                                              nUserID, (StreamType)nStreamType, nDelayMSec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserPosition(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jint nUserID,
                                                                             jint nStreamType,
                                                                             jfloat x, 
                                                                             jfloat y, 
                                                                             jfloat z)
    {
        return TT_SetUserPosition(reinterpret_cast<TTInstance*>(lpTTInstance),
                                  nUserID, (StreamType)nStreamType, x, y, z);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserStereo(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jint nUserID,
                                                                           jint nStreamType,
                                                                           jboolean bLeftSpeaker,
                                                                           jboolean bRightSpeaker)
    {
        return TT_SetUserStereo(reinterpret_cast<TTInstance*>(lpTTInstance),
                                nUserID, (StreamType)nStreamType, bLeftSpeaker, bRightSpeaker);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserMediaStorageDir(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint nUserID,
                                                                                    jstring szFolderPath,
                                                                                    jstring szFileNameVars,
                                                                                    jint uAFF)
    {
        THROW_NULLEX(env, szFolderPath, false);
        THROW_NULLEX(env, szFileNameVars, false);

        return TT_SetUserMediaStorageDir(reinterpret_cast<TTInstance*>(lpTTInstance),
                                         nUserID, ttstr(env, szFolderPath), 
                                         ttstr(env, szFileNameVars),
                                         (AudioFileFormat)uAFF);
    }


    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserAudioStreamBufferSize(JNIEnv* env,
                                                                                          jobject thiz,
                                                                                          jlong lpTTInstance,
                                                                                          jint nUserID,
                                                                                          jint uStreamType,
                                                                                          jint nMSec)
    {
        return TT_SetUserAudioStreamBufferSize(reinterpret_cast<TTInstance*>(lpTTInstance),
                                               nUserID, (StreamType)uStreamType, nMSec);
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserAudioBlock(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jint nStreamType,
                                                                                  jint nUserID)
    {
        AudioBlock* audblock = TT_AcquireUserAudioBlock(reinterpret_cast<TTInstance*>(lpTTInstance),
                                                        (StreamType)nStreamType, nUserID);
        if(!audblock)
            return NULL;
        jclass cls = env->FindClass("dk/bearware/AudioBlock");
        jobject audblk_obj = newObject(env, cls);
        setAudioBlock(env, *audblock, audblk_obj, N2J);
        TT_ReleaseUserAudioBlock(reinterpret_cast<TTInstance*>(lpTTInstance), audblock);
        return audblk_obj;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getFileTransferInfo(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jint nTransferID,
                                                                                 jobject lpFileTransfer)
    {
        THROW_NULLEX(env, lpFileTransfer, false);

        FileTransfer filetx;
        if(TT_GetFileTransferInfo(reinterpret_cast<TTInstance*>(lpTTInstance),
                                  nTransferID, &filetx))
        {
            setFileTransfer(env, filetx, lpFileTransfer);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_cancelFileTransfer(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance,
                                                                                jint nTransferID)
    {
        return TT_CancelFileTransfer(reinterpret_cast<TTInstance*>(lpTTInstance),
                                     nTransferID);
    }

    JNIEXPORT jstring JNICALL Java_dk_bearware_TeamTalkBase_getErrorMessage(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jint nError)
    {
        TTCHAR szError[TT_STRLEN] = {};
        TT_GetErrorMessage(nError, szError);
        return NEW_JSTRING(env, szError);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_DBG_1SetSoundInputTone(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint uStreamTypes,
                                                                                    jint nFrequency) {
        return TT_DBG_SetSoundInputTone(reinterpret_cast<TTInstance*>(lpTTInstance),
                                        uStreamTypes, nFrequency);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_DBG_1WriteAudioFileTone(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jobject lpMediaFileInfo,
                                                                                     jint nFrequency)
    {
        MediaFileInfo mfi = {};
        setMediaFileInfo(env, mfi, lpMediaFileInfo, J2N);
        return TT_DBG_WriteAudioFileTone(&mfi, nFrequency);
    }
    
    JNIEXPORT jint JNICALL Java_dk_bearware_PlatformHelper_desktopInputKeyTranslate(JNIEnv* env,
                                                                                    jclass,
                                                                                    jint nTranslate,
                                                                                    jobjectArray lpDesktopInputs,
                                                                                    jobjectArray lpTranslatedDesktopInputs)
    {
        if (env->GetArrayLength(lpDesktopInputs) != env->GetArrayLength(lpTranslatedDesktopInputs))
            return -1;

        jsize len = env->GetArrayLength(lpDesktopInputs);
        std::vector<DesktopInput> inputs(len), outputs(len);
        for(jsize i=0;i<len;i++) {
            setDesktopInput(env, inputs[i], env->GetObjectArrayElement(lpDesktopInputs, i), J2N);
        }
        jint ret = TT_DesktopInput_KeyTranslate(TTKeyTranslate(nTranslate), &inputs[0], &outputs[0], len);
        for (jsize i=0;i<jsize(len);++i) {
            setDesktopInput(env, outputs[i], env->GetObjectArrayElement(lpTranslatedDesktopInputs, i), N2J);
        }
        return ret;
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_PlatformHelper_desktopInputExecute(JNIEnv* env,
                                                                               jclass,
                                                                               jobjectArray lpDesktopInputs)
    {
        jsize len = env->GetArrayLength(lpDesktopInputs);
        std::vector<DesktopInput> inputs(len);
        for(jsize i=0;i<len;i++) {
            setDesktopInput(env, inputs[i], env->GetObjectArrayElement(lpDesktopInputs, i), J2N);
        }
        return TT_DesktopInput_Execute(&inputs[0], len);
    }
    
//TODO: TT_HotKey_*
                                                                              
} //extern "C"

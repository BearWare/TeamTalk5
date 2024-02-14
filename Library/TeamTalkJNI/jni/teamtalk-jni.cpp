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
#include <mutex>
#include <map>

#include <jni.h>

std::mutex ttinstmutex;
std::map<jint, TTInstance*> ttinstances;

void AddTTInstance(JNIEnv* env, jobject thiz, TTInstance* ttinst)
{
    auto hash = hashCode(env, thiz);

    std::lock_guard<std::mutex> g(ttinstmutex);
    ttinstances[hash] = ttinst;
}

TTInstance* RemoveTTInstance(JNIEnv* env, jobject thiz)
{
    auto hash = hashCode(env, thiz);

    std::lock_guard<std::mutex> g(ttinstmutex);
    TTInstance* ttinst = ttinstances[hash];
    ttinstances.erase(hash);
    return ttinst;
}

TTInstance* GetTTInstance(JNIEnv* env, jobject thiz)
{
    auto hash = hashCode(env, thiz);
    std::lock_guard<std::mutex> g(ttinstmutex);
    return ttinstances[hash];
}

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
        TTInstance* inst = TT_InitTeamTalkPoll();
        AddTTInstance(env, thiz, inst);
        return reinterpret_cast<jlong>(inst);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeTeamTalk(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance)
    {
        return TT_CloseTeamTalk(RemoveTTInstance(env, thiz));
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
        TTBOOL b = TT_GetMessage(GetTTInstance(env, thiz),
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
        return TT_PumpMessage(GetTTInstance(env, thiz),
                              (ClientEvent)nClientEvent, nIdentifier);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getFlags(JNIEnv* env,
                                                                  jobject thiz,
                                                                  jlong lpTTInstance)
    {
        return TT_GetFlags(GetTTInstance(env, thiz));
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

    JNIEXPORT jlong JNICALL Java_dk_bearware_TeamTalkBase_startSoundLoopbackTestEx(JNIEnv* env,
                                                                                   jclass,
                                                                                   jint nInputDeviceID,
                                                                                   jint nOutputDeviceID,
                                                                                   jint nSampleRate,
                                                                                   jint nChannels,
                                                                                   jboolean bDuplexMode,
                                                                                   jobject lpAudioPreprocessor,
                                                                                   jobject lpSoundDeviceEffects)
    {

        TTSoundLoop* inst;
        AudioPreprocessor preprocessor = {};
        SoundDeviceEffects effects = {};

        if (lpAudioPreprocessor)
            setAudioPreprocessor(env, preprocessor, lpAudioPreprocessor, J2N);
        if (lpSoundDeviceEffects)
            setSoundDeviceEffects(env, effects, lpSoundDeviceEffects, J2N);

        inst = TT_StartSoundLoopbackTestEx(nInputDeviceID, nOutputDeviceID,
                                           nSampleRate, nChannels, bDuplexMode,
                                           (lpAudioPreprocessor? &preprocessor : nullptr),
                                           (lpSoundDeviceEffects? &effects : nullptr));
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
        return TT_InitSoundInputDevice(GetTTInstance(env, thiz), nInputDeviceID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundInputSharedDevice(JNIEnv* env,
                                                                                        jclass,
                                                                                        jint nSampleRate,
                                                                                        jint nChannels,
                                                                                        jint nFrameSize)
    {
        return TT_InitSoundInputSharedDevice(nSampleRate, nChannels, nFrameSize);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundOutputDevice(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jint nOutputDeviceID)
    {
        return TT_InitSoundOutputDevice(GetTTInstance(env, thiz), nOutputDeviceID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundOutputSharedDevice(JNIEnv* env,
                                                                                        jclass,
                                                                                        jint nSampleRate,
                                                                                        jint nChannels,
                                                                                        jint nFrameSize)
    {
        return TT_InitSoundOutputSharedDevice(nSampleRate, nChannels, nFrameSize);
    }


    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_initSoundDuplexDevices(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint nInputDeviceID,
                                                                                    jint nOutputDeviceID)
    {
        return TT_InitSoundDuplexDevices(GetTTInstance(env, thiz), nInputDeviceID, nOutputDeviceID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundInputDevice(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance)
    {
        return TT_CloseSoundInputDevice(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundOutputDevice(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance)
    {
        return TT_CloseSoundOutputDevice(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeSoundDuplexDevices(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance)
    {
        return TT_CloseSoundDuplexDevices(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundDeviceEffects(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jobject lpSoundDeviceEffects)
    {
        SoundDeviceEffects effects = {};
        setSoundDeviceEffects(env, effects, lpSoundDeviceEffects, J2N);

        return TT_SetSoundDeviceEffects(GetTTInstance(env, thiz), &effects);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getSoundDeviceEffects(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jobject lpSoundDeviceEffects)
    {
        SoundDeviceEffects effects = {};
        if (TT_GetSoundDeviceEffects(GetTTInstance(env, thiz), &effects))
        {
            setSoundDeviceEffects(env, effects, lpSoundDeviceEffects, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputLevel(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance)
    {
        return TT_GetSoundInputLevel(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundInputGainLevel(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint nLevel)
    {
        return TT_SetSoundInputGainLevel(GetTTInstance(env, thiz),
                                         nLevel);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputGainLevel(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance)
    {
        return TT_GetSoundInputGainLevel(GetTTInstance(env, thiz));
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
        return TT_SetSoundInputPreprocess(GetTTInstance(env, thiz), &spxdsp);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputPreprocess(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jobject lpSpeexDSP)
    {
        THROW_NULLEX(env, lpSpeexDSP, false);

        SpeexDSP spxdsp;
        ZERO_STRUCT(spxdsp);
        if(TT_GetSoundInputPreprocess(GetTTInstance(env, thiz), &spxdsp))
        {
            setSpeexDSP(env, spxdsp, lpSpeexDSP, N2J);
            return true;
        }
        return false;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundInputPreprocessEx(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jobject lpAudioPreprocessor)
    {
        THROW_NULLEX(env, lpAudioPreprocessor, false);

        AudioPreprocessor preprocessor = {};
        setAudioPreprocessor(env, preprocessor, lpAudioPreprocessor, J2N);
        return TT_SetSoundInputPreprocessEx(GetTTInstance(env, thiz), &preprocessor);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getSoundInputPreprocessEx(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jobject lpAudioPreprocessor)
    {
        THROW_NULLEX(env, lpAudioPreprocessor, false);

        AudioPreprocessor preprocess = {};
        if (TT_GetSoundInputPreprocessEx(GetTTInstance(env, thiz), &preprocess))
        {
            setAudioPreprocessor(env, preprocess, lpAudioPreprocessor, N2J);
            return true;
        }
        return false;
    }


    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundOutputVolume(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jint nVolume)
    {
        return TT_SetSoundOutputVolume(GetTTInstance(env, thiz),
                                       nVolume);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getSoundOutputVolume(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance)
    {
        return TT_GetSoundOutputVolume(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setSoundOutputMute(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance,
                                                                                jboolean bMuteAll)
    {
        return TT_SetSoundOutputMute(GetTTInstance(env, thiz),
                                     bMuteAll);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableVoiceTransmission(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jboolean bEnable)
    {
        return TT_EnableVoiceTransmission(GetTTInstance(env, thiz),
                                          bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enable3DSoundPositioning(JNIEnv* env,
                                                                                      jobject thiz,
                                                                                      jlong lpTTInstance,
                                                                                      jboolean bEnable)
    {
        return TT_Enable3DSoundPositioning(GetTTInstance(env, thiz),
                                           bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_autoPositionUsers(JNIEnv* env,
                                                                               jobject thiz,
                                                                               jlong lpTTInstance)
    {
        return TT_AutoPositionUsers(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableAudioBlockEvent(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jint nUserID,
                                                                                   jint nStreamType,
                                                                                   jboolean bEnable)
    {
        return TT_EnableAudioBlockEvent(GetTTInstance(env, thiz),
                                        nUserID, (StreamType)nStreamType, bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableAudioBlockEventEx(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jint nUserID,
                                                                                     jint nStreamType,
                                                                                     jobject lpAudioFormat,
                                                                                     jboolean bEnable)
    {
        AudioFormat fmt = {};
        if (lpAudioFormat)
            setAudioFormat(env, fmt, lpAudioFormat, J2N);

        return TT_EnableAudioBlockEventEx(GetTTInstance(env, thiz),
                                          nUserID, (StreamType)nStreamType, lpAudioFormat? &fmt : nullptr, bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_insertAudioBlock(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance,
                                                                              jobject lpAudioBlock)
    {
        AudioBlock ab = {};
        auto byteArr = setAudioBlock(env, ab, lpAudioBlock, J2N);
        jboolean b = TT_InsertAudioBlock(GetTTInstance(env, thiz), &ab);
        if (byteArr)
            env->ReleaseByteArrayElements(byteArr, reinterpret_cast<jbyte*>(ab.lpRawAudio), JNI_ABORT);
        return b;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_enableVoiceActivation(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong lpTTInstance,
                                                                                   jboolean bEnable)
    {
        return TT_EnableVoiceActivation(GetTTInstance(env, thiz),
                                        bEnable);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setVoiceActivationLevel(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jint nLevel)
    {
        return TT_SetVoiceActivationLevel(GetTTInstance(env, thiz),
                                          nLevel);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getVoiceActivationLevel(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance)
    {
        return TT_GetVoiceActivationLevel(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setVoiceActivationStopDelay(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nDelayMSec)
    {
        return TT_SetVoiceActivationStopDelay(GetTTInstance(env, thiz),
                                              nDelayMSec);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getVoiceActivationStopDelay(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance)
    {
        return TT_GetVoiceActivationStopDelay(GetTTInstance(env, thiz));
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

        AudioCodec audcodec = {};
        setAudioCodec(env, audcodec, lpAudioCodec, J2N);

        return TT_StartRecordingMuxedAudioFile(GetTTInstance(env, thiz),
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

        return TT_StartRecordingMuxedAudioFileEx(GetTTInstance(env, thiz),
                                                 nChannelID, ttstr(env, szAudioFileName), (AudioFileFormat)uAFF);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_startRecordingMuxedStreams(JNIEnv* env,
                                                                                        jobject thiz,
                                                                                        jlong lpTTInstance,
                                                                                        jint uStreamTypes,
                                                                                        jobject lpAudioCodec,
                                                                                        jstring szAudioFileName,
                                                                                        jint uAFF)
    {
        THROW_NULLEX(env, lpAudioCodec, false);
        THROW_NULLEX(env, szAudioFileName, false);

        AudioCodec audcodec = {};
        setAudioCodec(env, audcodec, lpAudioCodec, J2N);
        
        return TT_StartRecordingMuxedStreams(GetTTInstance(env, thiz),
                                             uStreamTypes, &audcodec,
                                             ttstr(env, szAudioFileName), (AudioFileFormat)uAFF);
    }


    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopRecordingMuxedAudioFile(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance)
    {
        return TT_StopRecordingMuxedAudioFile(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopRecordingMuxedAudioFileEx(JNIEnv* env,
                                                                                           jobject thiz,
                                                                                           jlong lpTTInstance,
                                                                                           jint nChannelID)
    {
        return TT_StopRecordingMuxedAudioFileEx(GetTTInstance(env, thiz), nChannelID);
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
        return TT_StartVideoCaptureTransmission(GetTTInstance(env, thiz),
                                                &vidcodec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopVideoCaptureTransmission(JNIEnv* env,
                                                                                          jobject thiz,
                                                                                          jlong lpTTInstance)
    {
        return TT_StopVideoCaptureTransmission(GetTTInstance(env, thiz));
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

        return TT_InitVideoCaptureDevice(GetTTInstance(env, thiz),
                                         ttstr(env, szDeviceID), &fmt);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeVideoCaptureDevice(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance)
    {
        return TT_CloseVideoCaptureDevice(GetTTInstance(env, thiz));
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserVideoCaptureFrame(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nUserID)
    {
        VideoFrame* vidframe = TT_AcquireUserVideoCaptureFrame(GetTTInstance(env, thiz),
                                                               nUserID);
        if(!vidframe)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/VideoFrame");
        jobject vidframe_obj = newObject(env, cls);
        setVideoFrame(env, *vidframe, vidframe_obj);

        TT_ReleaseUserVideoCaptureFrame(GetTTInstance(env, thiz), vidframe);
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
        return TT_StartStreamingMediaFileToChannel(GetTTInstance(env, thiz),
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
        return TT_StartStreamingMediaFileToChannelEx(GetTTInstance(env, thiz),
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

        return TT_UpdateStreamingMediaFileToChannel(GetTTInstance(env, thiz),
                                                    &mfp, &vidcodec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopStreamingMediaFileToChannel(JNIEnv* env,
                                                                                             jobject thiz,
                                                                                             jlong lpTTInstance)
    {
        return TT_StopStreamingMediaFileToChannel(GetTTInstance(env, thiz));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_initLocalPlayback(JNIEnv* env, jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jstring szMediaFilePath,
                                                                           jobject lpMediaFilePlayback) {
        THROW_NULLEX(env, szMediaFilePath, 0);
        THROW_NULLEX(env, lpMediaFilePlayback, 0);

        MediaFilePlayback playback = {};
        setMediaFilePlayback(env, playback, lpMediaFilePlayback, J2N);

        return TT_InitLocalPlayback(GetTTInstance(env, thiz),
                                    ttstr(env, szMediaFilePath), &playback);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_updateLocalPlayback(JNIEnv* env, jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jint nPlaybackSessionID,
                                                                                 jobject lpMediaFilePlayback) {

        THROW_NULLEX(env, lpMediaFilePlayback, false);

        MediaFilePlayback playback = {};
        setMediaFilePlayback(env, playback, lpMediaFilePlayback, J2N);

        return TT_UpdateLocalPlayback(GetTTInstance(env, thiz),
                                      nPlaybackSessionID, &playback);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_stopLocalPlayback(JNIEnv* env, jobject thiz,
                                                                               jlong lpTTInstance,
                                                                               jint nPlaybackSessionID) {
        return TT_StopLocalPlayback(GetTTInstance(env, thiz),
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
        VideoFrame* vidframe = TT_AcquireUserMediaVideoFrame(GetTTInstance(env, thiz),
                                                             nUserID);

        if(!vidframe)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/VideoFrame");
        jobject vidframe_obj = newObject(env, cls);
        setVideoFrame(env, *vidframe, vidframe_obj);

        TT_ReleaseUserMediaVideoFrame(GetTTInstance(env, thiz), vidframe);
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

        jint ret = TT_SendDesktopWindow(GetTTInstance(env, thiz),
                                        &wnd, (BitmapFormat)nConvertBmpFormat);

       env->ReleaseByteArrayElements(buf, bufptr, 0);
       return ret;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_closeDesktopWindow(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance)
    {
        return TT_CloseDesktopWindow(GetTTInstance(env, thiz));
    }

//TODO: Palette_GetColorTable

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_sendDesktopCursorPosition(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jint nPosX,
                                                                                       jint nPosY)
    {
        return TT_SendDesktopCursorPosition(GetTTInstance(env, thiz),
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
        return TT_SendDesktopInput(GetTTInstance(env, thiz), nUserID,
                                   &inputs[0], len);
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserDesktopWindow(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong lpTTInstance,
                                                                                     jint nUserID) {
        DesktopWindow* deskwnd = TT_AcquireUserDesktopWindow(GetTTInstance(env, thiz),
                                                             nUserID);
        if(!deskwnd)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/DesktopWindow");
        jobject deskwnd_obj = newObject(env, cls);
        setDesktopWindow(env, *deskwnd, deskwnd_obj, N2J);

        TT_ReleaseUserDesktopWindow(GetTTInstance(env, thiz), deskwnd);
        return deskwnd_obj;
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserDesktopWindowEx(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong lpTTInstance,
                                                                                       jint nUserID,
                                                                                       jint nBitmapFormat) {
        DesktopWindow* deskwnd = TT_AcquireUserDesktopWindowEx(GetTTInstance(env, thiz),
                                                               nUserID, (BitmapFormat)nBitmapFormat);
        if(!deskwnd)
            return NULL;

        jclass cls = env->FindClass("dk/bearware/DesktopWindow");
        jobject deskwnd_obj = newObject(env, cls);
        setDesktopWindow(env, *deskwnd, deskwnd_obj, N2J);

        TT_ReleaseUserDesktopWindow(GetTTInstance(env, thiz), deskwnd);
        return deskwnd_obj;
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setEncryptionContext(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jobject lpEncryptionContext) {
        THROW_NULLEX(env, lpEncryptionContext, false);

        EncryptionContext context = {};
        setEncryptionContext(env, context, lpEncryptionContext, J2N);

        return TT_SetEncryptionContext(GetTTInstance(env, thiz),
                                       &context);
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

        return TT_Connect(GetTTInstance(env, thiz),
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

        return TT_ConnectSysID(GetTTInstance(env, thiz),
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

        return TT_ConnectEx(GetTTInstance(env, thiz),
                            ttstr(env, szHostAddress), nTcpPort, nUdpPort,
                            ttstr(env, szBindIPAddr), nLocalTcpPort, nLocalUdpPort,
                            bEncrypted);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_disconnect(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance)

    {
        return TT_Disconnect(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_queryMaxPayload(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jint nUserID)
    {
        return TT_QueryMaxPayload(GetTTInstance(env, thiz), nUserID);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getClientStatistics(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jobject lpClientStatistics)
    {
        THROW_NULLEX(env, lpClientStatistics, false);

        ClientStatistics stats;
        if(TT_GetClientStatistics(GetTTInstance(env, thiz),
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

        return TT_SetClientKeepAlive(GetTTInstance(env, thiz), &ka);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getClientKeepAlive(JNIEnv* env,
                                                                                jobject thiz,
                                                                                jlong lpTTInstance,
                                                                                jobject lpClientKeepAlive)
    {
        THROW_NULLEX(env, lpClientKeepAlive, false);

        ClientKeepAlive ka = {};
        if (TT_GetClientKeepAlive(GetTTInstance(env, thiz), &ka)) {
            setClientKeepAlive(env, ka, lpClientKeepAlive, N2J);
            return true;
        }

        return false;
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doPing(JNIEnv* env,
                                                                jobject thiz,
                                                                jlong lpTTInstance)
    {
        return TT_DoPing(GetTTInstance(env, thiz));
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

        return TT_DoLogin(GetTTInstance(env, thiz),
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

        return TT_DoLoginEx(GetTTInstance(env, thiz),
                            ttstr(env, szNickname), ttstr(env, szUsername),
                            ttstr(env, szPassword), ttstr(env, szClientName));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doLogout(JNIEnv* env,
                                                                  jobject thiz,
                                                                  jlong lpTTInstance)
    {
        return TT_DoLogout(GetTTInstance(env, thiz));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doJoinChannel(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, -1);

        Channel chan = {};
        setChannel(env, chan, lpChannel, J2N);
        return TT_DoJoinChannel(GetTTInstance(env, thiz), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doJoinChannelByID(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance,
                                                                           jint nChannelID,
                                                                           jstring szPassword)
    {
        THROW_NULLEX(env, szPassword, -1);

        return TT_DoJoinChannelByID(GetTTInstance(env, thiz),
                                    nChannelID, ttstr(env, szPassword));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doLeaveChannel(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance)
    {
        return TT_DoLeaveChannel(GetTTInstance(env, thiz));

    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChangeNickname(JNIEnv* env,
                                                                          jobject thiz,
                                                                          jlong lpTTInstance,
                                                                          jstring szNewNick)
    {
        THROW_NULLEX(env, szNewNick, -1);

        return TT_DoChangeNickname(GetTTInstance(env, thiz),
                                   ttstr(env, szNewNick));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChangeStatus(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jint nStatusMode,
                                                                        jstring szStatusMessage)
    {
        THROW_NULLEX(env, szStatusMessage, -1);

        return TT_DoChangeStatus(GetTTInstance(env, thiz),
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
        return TT_DoTextMessage(GetTTInstance(env, thiz), &msg);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doChannelOp(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jint nChannelID,
                                                                     jboolean bMakeOperator)
    {
        return TT_DoChannelOp(GetTTInstance(env, thiz), nUserID, nChannelID, bMakeOperator);
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

        return TT_DoChannelOpEx(GetTTInstance(env, thiz), nUserID,
                                nChannelID, ttstr(env, szOpPassword), bMakeOperator);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doKickUser(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nUserID,
                                                                    jint nChannelID)
    {
        return TT_DoKickUser(GetTTInstance(env, thiz), nUserID,
                             nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doSendFile(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nChannelID,
                                                                    jstring szLocalFilePath)
    {
        THROW_NULLEX(env, szLocalFilePath, -1);

        return TT_DoSendFile(GetTTInstance(env, thiz), nChannelID,
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

        return TT_DoRecvFile(GetTTInstance(env, thiz), nChannelID, nFileID,
                             ttstr(env, szLocalFilePath));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doDeleteFile(JNIEnv* env,
                                                                      jobject thiz,
                                                                      jlong lpTTInstance,
                                                                      jint nChannelID,
                                                                      jint nFileID)
    {
        return TT_DoDeleteFile(GetTTInstance(env, thiz), nChannelID, nFileID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doSubscribe(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jint uSubscriptions)
    {
        return TT_DoSubscribe(GetTTInstance(env, thiz), nUserID, uSubscriptions);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUnsubscribe(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jint nUserID,
                                                                       jint uSubscriptions)
    {
        return TT_DoUnsubscribe(GetTTInstance(env, thiz), nUserID, uSubscriptions);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doMakeChannel(JNIEnv* env,
                                                                       jobject thiz,
                                                                       jlong lpTTInstance,
                                                                       jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, -1);

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TT_DoMakeChannel(GetTTInstance(env, thiz), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUpdateChannel(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, -1);

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TT_DoUpdateChannel(GetTTInstance(env, thiz), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doRemoveChannel(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jint nChannelID)
    {
        return TT_DoRemoveChannel(GetTTInstance(env, thiz), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doMoveUser(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nUserID,
                                                                    jint nChannelID)
    {
        return TT_DoMoveUser(GetTTInstance(env, thiz), nUserID, nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUpdateServer(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jobject lpServerProperties)
    {
        THROW_NULLEX(env, lpServerProperties, -1);

        ServerProperties srvprop;
        setServerProperties(env, srvprop, lpServerProperties, J2N);
        return TT_DoUpdateServer(GetTTInstance(env, thiz), &srvprop);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doListUserAccounts(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance,
                                                                            jint nIndex,
                                                                            jint nCount)
    {
        return TT_DoListUserAccounts(GetTTInstance(env, thiz), nIndex, nCount);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doNewUserAccount(JNIEnv* env,
                                                                          jobject thiz,
                                                                          jlong lpTTInstance,
                                                                          jobject lpUserAccount)
    {
        THROW_NULLEX(env, lpUserAccount, -1);

        UserAccount account;
        setUserAccount(env, account, lpUserAccount, J2N);
        return TT_DoNewUserAccount(GetTTInstance(env, thiz), &account);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doDeleteUserAccount(JNIEnv* env,
                                                                             jobject thiz,
                                                                             jlong lpTTInstance,
                                                                             jstring szUsername)
    {
        THROW_NULLEX(env, szUsername, -1);

        return TT_DoDeleteUserAccount(GetTTInstance(env, thiz), ttstr(env, szUsername));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBanUser(JNIEnv* env,
                                                                   jobject thiz,
                                                                   jlong lpTTInstance,
                                                                   jint nUserID,
                                                                   jint nChannelID)
    {
        return TT_DoBanUser(GetTTInstance(env, thiz), nUserID, nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBanUserEx(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jint uBanTypes)
    {
        return TT_DoBanUserEx(GetTTInstance(env, thiz), nUserID, uBanTypes);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBan(JNIEnv* env,
                                                               jobject thiz,
                                                               jlong lpTTInstance,
                                                               jobject lpBannedUser)
    {
        BannedUser ban;
        setBannedUser(env, ban, lpBannedUser, J2N);
        return TT_DoBan(GetTTInstance(env, thiz), &ban);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doBanIPAddress(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jstring szIPAddress,
                                                                        jint nChannelID)
    {
        THROW_NULLEX(env, szIPAddress, -1);

        return TT_DoBanIPAddress(GetTTInstance(env, thiz),
                                 ttstr(env, szIPAddress), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doUnBanUser(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jstring szIPAddress,
                                                                     jint nChannelID)
    {
        THROW_NULLEX(env, szIPAddress, -1);

        return TT_DoUnBanUser(GetTTInstance(env, thiz),
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
        return TT_DoUnBanUserEx(GetTTInstance(env, thiz), &ban);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doListBans(JNIEnv* env,
                                                                    jobject thiz,
                                                                    jlong lpTTInstance,
                                                                    jint nChannelID,
                                                                    jint nIndex,
                                                                    jint nCount)
    {
        return TT_DoListBans(GetTTInstance(env, thiz), nChannelID,
                             nIndex, nCount);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doSaveConfig(JNIEnv* env,
                                                                      jobject thiz,
                                                                      jlong lpTTInstance)
    {
        return TT_DoSaveConfig(GetTTInstance(env, thiz));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doQueryServerStats(JNIEnv* env,
                                                                            jobject thiz,
                                                                            jlong lpTTInstance)
    {
        return TT_DoQueryServerStats(GetTTInstance(env, thiz));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_doQuit(JNIEnv* env,
                                                                jobject thiz,
                                                                jlong lpTTInstance)
    {
        return TT_DoQuit(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getServerProperties(JNIEnv* env,
                                                                                 jobject thiz,
                                                                                 jlong lpTTInstance,
                                                                                 jobject lpServerProperties)
    {
        THROW_NULLEX(env, lpServerProperties, false);

        ServerProperties srvprop;
        if(TT_GetServerProperties(GetTTInstance(env, thiz),
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
            if(TT_GetServerUsers(GetTTInstance(env, thiz),
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
           TT_GetServerUsers(GetTTInstance(env, thiz), &users[0],
                             &n_users))
        {
            n_users = std::min(n_users, (INT32)getIntPtr(env, lpnHowMany));
            setIntPtr(env, lpnHowMany, n_users);

            for(jsize i=0; i < jsize(n_users); i++)
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
        return TT_GetRootChannelID(GetTTInstance(env, thiz));
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkBase_getMyChannelID(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance)
    {
        return TT_GetMyChannelID(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getChannel(JNIEnv* env,
                                                                        jobject thiz,
                                                                        jlong lpTTInstance,
                                                                        jint nChannelID,
                                                                        jobject lpChannel)
    {
        THROW_NULLEX(env, lpChannel, false);

        Channel chan;
        if(TT_GetChannel(GetTTInstance(env, thiz),
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
        TT_GetChannelPath(GetTTInstance(env, thiz),
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
        return TT_GetChannelIDFromPath(GetTTInstance(env, thiz),
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
            if(TT_GetChannelUsers(GetTTInstance(env, thiz),
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
           TT_GetChannelUsers(GetTTInstance(env, thiz),
                              nChannelID, &users[0], &n_users))
        {
            n_users = std::min(n_users, (INT32)getIntPtr(env, lpnHowMany));
            setIntPtr(env, lpnHowMany, n_users);
            for(jsize i=0; i < jsize(n_users); i++)
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
            if(TT_GetChannelFiles(GetTTInstance(env, thiz),
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
           TT_GetChannelFiles(GetTTInstance(env, thiz),
                              nChannelID, &files[0], &n_files))
        {
            n_files = std::min(n_files, (INT32)getIntPtr(env, lpnHowMany));
            if(n_files>0)
            {
                std::vector<jobject> jfiles(n_files);
                jclass cls = env->FindClass("dk/bearware/RemoteFile");
                for(jsize i=0; i < jsize(n_files); i++)
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
        if(TT_GetChannelFile(GetTTInstance(env, thiz),
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
        return TT_IsChannelOperator(GetTTInstance(env, thiz),
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
            if(TT_GetServerChannels(GetTTInstance(env, thiz),
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
           TT_GetServerChannels(GetTTInstance(env, thiz), &channels[0],
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
        return TT_GetMyUserID(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMyUserAccount(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong lpTTInstance,
                                                                              jobject lpUserAccount)
    {
        THROW_NULLEX(env, lpUserAccount, false);

        UserAccount account;
        if(TT_GetMyUserAccount(GetTTInstance(env, thiz), &account))
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
        return TT_GetMyUserType(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getMyUserData(JNIEnv* env,
                                                                           jobject thiz,
                                                                           jlong lpTTInstance)
    {
        return TT_GetMyUserData(GetTTInstance(env, thiz));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getUser(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jlong lpTTInstance,
                                                                     jint nUserID,
                                                                     jobject lpUser)
    {
        THROW_NULLEX(env, lpUser, false);

        User user;
        if(TT_GetUser(GetTTInstance(env, thiz),
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
        if(TT_GetUserStatistics(GetTTInstance(env, thiz),
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
        if(TT_GetUserByUsername(GetTTInstance(env, thiz),
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
        return TT_SetUserVolume(GetTTInstance(env, thiz),
                                nUserID, (StreamType)nStreamType, nVolume);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserMute(JNIEnv* env,
                                                                         jobject thiz,
                                                                         jlong lpTTInstance,
                                                                         jint nUserID,
                                                                         jint nStreamType,
                                                                         jboolean bMute)
    {
        return TT_SetUserMute(GetTTInstance(env, thiz),
                              nUserID, (StreamType)nStreamType, bMute);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserStoppedPlaybackDelay(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nUserID,
                                                                                         jint nStreamType,
                                                                                         jint nDelayMSec)
    {
        return TT_SetUserStoppedPlaybackDelay(GetTTInstance(env, thiz),
                                              nUserID, (StreamType)nStreamType, nDelayMSec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserJitterControl(JNIEnv* env,
                                                                                         jobject thiz,
                                                                                         jlong lpTTInstance,
                                                                                         jint nUserID,
                                                                                         jint nStreamType,
                                                                                         jobject lpJitterConfig)
    {

        JitterConfig config = {};

        if (lpJitterConfig)
            setJitterConfig(env, config, lpJitterConfig);

        return TT_SetUserJitterControl(GetTTInstance(env, thiz), nUserID, (StreamType)nStreamType,
                                        (lpJitterConfig ? &config : nullptr));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_getUserJitterControl(JNIEnv* env,
                                                                                        jobject thiz,
                                                                                        jlong lpTTInstance,
                                                                                        jint nUserID,
                                                                                        jint nStreamType,
                                                                                        jobject lpJitterConfig)
    {
        THROW_NULLEX(env, lpJitterConfig, false);

        JitterConfig config = {};

        if (TT_GetUserJitterControl(GetTTInstance(env, thiz), nUserID, (StreamType)nStreamType,
            (lpJitterConfig ? &config : nullptr)))
        {
            setJitterConfig(env, lpJitterConfig, config);
            return true;
        }
        return false;
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
        return TT_SetUserPosition(GetTTInstance(env, thiz),
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
        return TT_SetUserStereo(GetTTInstance(env, thiz),
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

        return TT_SetUserMediaStorageDir(GetTTInstance(env, thiz),
                                         nUserID, ttstr(env, szFolderPath),
                                         ttstr(env, szFileNameVars),
                                         (AudioFileFormat)uAFF);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserMediaStorageDirEx(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jlong lpTTInstance,
                                                                                    jint nUserID,
                                                                                    jstring szFolderPath,
                                                                                    jstring szFileNameVars,
                                                                                    jint uAFF,
                                                                                    jint nStopRecordingExtraDelayMSec)
    {
        THROW_NULLEX(env, szFolderPath, false);
        THROW_NULLEX(env, szFileNameVars, false);

        return TT_SetUserMediaStorageDirEx(GetTTInstance(env, thiz),
                                         nUserID, ttstr(env, szFolderPath),
                                         ttstr(env, szFileNameVars),
                                         (AudioFileFormat)uAFF,
                                         nStopRecordingExtraDelayMSec);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkBase_setUserAudioStreamBufferSize(JNIEnv* env,
                                                                                          jobject thiz,
                                                                                          jlong lpTTInstance,
                                                                                          jint nUserID,
                                                                                          jint uStreamType,
                                                                                          jint nMSec)
    {
        return TT_SetUserAudioStreamBufferSize(GetTTInstance(env, thiz),
                                               nUserID, (StreamType)uStreamType, nMSec);
    }

    JNIEXPORT jobject JNICALL Java_dk_bearware_TeamTalkBase_acquireUserAudioBlock(JNIEnv* env,
                                                                                  jobject thiz,
                                                                                  jlong lpTTInstance,
                                                                                  jint nStreamType,
                                                                                  jint nUserID)
    {
        AudioBlock* audblock = TT_AcquireUserAudioBlock(GetTTInstance(env, thiz),
                                                        (StreamType)nStreamType, nUserID);
        if(!audblock)
            return NULL;
        jclass cls = env->FindClass("dk/bearware/AudioBlock");
        jobject audblk_obj = newObject(env, cls);
        setAudioBlock(env, *audblock, audblk_obj, N2J);
        TT_ReleaseUserAudioBlock(GetTTInstance(env, thiz), audblock);
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
        if(TT_GetFileTransferInfo(GetTTInstance(env, thiz),
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
        return TT_CancelFileTransfer(GetTTInstance(env, thiz),
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
        return TT_DBG_SetSoundInputTone(GetTTInstance(env, thiz),
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

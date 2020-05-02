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
#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>
using namespace std;

#define TRANSMITUSER_ARRAY_SIZE 2

#if defined(WIN32)
const jint* TO_JINT_ARRAY(const INT32* ttints, jint* jints, INT32 N)
{
    for(int ii=0;ii<N;ii++)jints[ii] = (jint)ttints[ii];
    return jints;
}
#endif

const INT32* TO_INT32_ARRAY(const jint* jints, INT32* ttints, jsize N)
{
    for(int ii=0;ii<N;ii++)ttints[ii] = (INT32)jints[ii];
    return ttints;
}

jobject newObject(JNIEnv* env, jclass cls_obj)
{
    jmethodID midInit = env->GetMethodID(cls_obj, "<init>", "()V");
    assert(midInit);
    jobject newObj = env->NewObject(cls_obj, midInit);
    assert(newObj);
    return newObj;
}

jobject newSoundDevice(JNIEnv* env, const SoundDevice& dev)
{
    jclass cls_snddev = env->FindClass("dk/bearware/SoundDevice");

    jobject newObj = newObject(env, cls_snddev);

    jfieldID fid_devid = env->GetFieldID(cls_snddev, "nDeviceID", "I");
    assert(fid_devid);
    jfieldID fid_sndsys = env->GetFieldID(cls_snddev, "nSoundSystem", "I");
    assert(fid_sndsys);
    jfieldID fid_devname = env->GetFieldID(cls_snddev, "szDeviceName", "Ljava/lang/String;");
    assert(fid_devname);
    jfieldID fid_devstr = env->GetFieldID(cls_snddev, "szDeviceID", "Ljava/lang/String;");
    assert(fid_devstr);
    jfieldID fid_wave = env->GetFieldID(cls_snddev, "nWaveDeviceID", "I");
    assert(fid_wave);
    jfieldID fid_3d = env->GetFieldID(cls_snddev, "bSupports3D", "Z");
    assert(fid_3d);
    jfieldID fid_inchan = env->GetFieldID(cls_snddev, "nMaxInputChannels", "I");
    assert(fid_inchan);
    jfieldID fid_outchan = env->GetFieldID(cls_snddev, "nMaxOutputChannels", "I");
    assert(fid_outchan);
    jfieldID fid_insr = env->GetFieldID(cls_snddev, "inputSampleRates", "[I");
    assert(fid_insr);
    jfieldID fid_outsr = env->GetFieldID(cls_snddev, "outputSampleRates", "[I");
    assert(fid_outsr);
    jfieldID fid_defsr = env->GetFieldID(cls_snddev, "nDefaultSampleRate", "I");
    assert(fid_defsr);
    jfieldID fid_sdf = env->GetFieldID(cls_snddev, "uSoundDeviceFeatures", "I");
    assert(fid_sdf);

    env->SetIntField(newObj, fid_devid, dev.nDeviceID);
    env->SetIntField(newObj, fid_sndsys, dev.nSoundSystem);
    env->SetObjectField(newObj, fid_devname, NEW_JSTRING(env, dev.szDeviceName));
    env->SetObjectField(newObj, fid_devstr, NEW_JSTRING(env, dev.szDeviceID));
    env->SetIntField(newObj, fid_wave, dev.nWaveDeviceID);
    env->SetBooleanField(newObj, fid_3d, dev.bSupports3D);
    env->SetIntField(newObj, fid_inchan, dev.nMaxInputChannels);
    env->SetIntField(newObj, fid_outchan, dev.nMaxOutputChannels);
    jintArray arr_insr = env->NewIntArray(TT_SAMPLERATES_MAX);
    jint tmp[TT_SAMPLERATES_MAX];
    env->SetIntArrayRegion(arr_insr, 0, TT_SAMPLERATES_MAX, TO_JINT_ARRAY(dev.inputSampleRates, tmp, TT_SAMPLERATES_MAX));
    env->SetObjectField(newObj, fid_insr, arr_insr);
    jintArray arr_outsr = env->NewIntArray(TT_SAMPLERATES_MAX);
    env->SetIntArrayRegion(arr_outsr, 0, TT_SAMPLERATES_MAX, TO_JINT_ARRAY(dev.outputSampleRates, tmp, TT_SAMPLERATES_MAX));
    env->SetObjectField(newObj, fid_outsr, arr_outsr);
    env->SetIntField(newObj, fid_defsr, dev.nDefaultSampleRate);
    env->SetIntField(newObj, fid_sdf, dev.uSoundDeviceFeatures);

    return newObj;
}

jobject newVideoDevice(JNIEnv* env, VideoCaptureDevice& dev)
{
    jclass cls_viddev = env->FindClass("dk/bearware/VideoCaptureDevice");

    jobject newObj = newObject(env, cls_viddev);

    jfieldID fid_devid = env->GetFieldID(cls_viddev, "szDeviceID", "Ljava/lang/String;");
    jfieldID fid_name = env->GetFieldID(cls_viddev, "szDeviceName", "Ljava/lang/String;");
    jfieldID fid_api = env->GetFieldID(cls_viddev, "szCaptureAPI", "Ljava/lang/String;");
    jfieldID fid_fmts = env->GetFieldID(cls_viddev, "videoFormats", "[Ldk/bearware/VideoFormat;");

    assert(fid_devid);
    assert(fid_name);
    assert(fid_api);
    assert(fid_fmts);

    env->SetObjectField(newObj, fid_devid, NEW_JSTRING(env, dev.szDeviceID));
    env->SetObjectField(newObj, fid_name, NEW_JSTRING(env, dev.szDeviceName));
    env->SetObjectField(newObj, fid_api, NEW_JSTRING(env, dev.szCaptureAPI));
    jclass cls_vidfmt = env->FindClass("dk/bearware/VideoFormat");
    jobjectArray buf = env->NewObjectArray(dev.nVideoFormatsCount, cls_vidfmt, NULL);
    env->SetObjectField(newObj, fid_fmts, buf);

    for(int i=0;i<dev.nVideoFormatsCount;i++)
    {
        jobject vidfmt_obj = newObject(env, cls_vidfmt);
        setVideoFormat(env, dev.videoFormats[i], vidfmt_obj, N2J);
        env->SetObjectArrayElement(buf, i, vidfmt_obj);
    }
    return newObj;
}

jobject newChannel(JNIEnv* env, const Channel* lpChannel) {
    jclass cls = env->FindClass("dk/bearware/Channel");
    assert(cls);
    jobject channel_obj = NULL;
    if(lpChannel) {
        channel_obj = newObject(env, cls);
        assert(channel_obj);
        setChannel(env, const_cast<Channel&>(*lpChannel), channel_obj, N2J);
    }
    return channel_obj;
}

jobject newUser(JNIEnv* env, const User* lpUser) {
    jclass cls = env->FindClass("dk/bearware/User");
    assert(cls);
    jobject user_obj = NULL;
    if(lpUser) {
        user_obj = newObject(env, cls);
        assert(user_obj);
        setUser(env, const_cast<User&>(*lpUser), user_obj);
    }
    return user_obj;
}

jobject newClientErrorMsg(JNIEnv* env, const ClientErrorMsg* lpClientErrorMsg) {

    jclass cls = env->FindClass("dk/bearware/ClientErrorMsg");
    jobject errmsg_obj = NULL;

    if(lpClientErrorMsg) {
        errmsg_obj = newObject(env, cls);
        assert(errmsg_obj);
        setClientErrorMsg(env, const_cast<ClientErrorMsg&>(*lpClientErrorMsg), errmsg_obj, N2J);
    }
    return errmsg_obj;
}

jobject newUserAccount(JNIEnv* env, const UserAccount* lpUserAccount) {
    jclass cls = env->FindClass("dk/bearware/UserAccount");
    jobject ua_obj = NULL;

    if(lpUserAccount) {
        ua_obj = newObject(env, cls);
        assert(ua_obj);
        setUserAccount(env, const_cast<UserAccount&>(*lpUserAccount), ua_obj, N2J);
    }
    return ua_obj;
}

jobject newTextMessage(JNIEnv* env, const TextMessage* lpTextMessage) {
    jclass cls = env->FindClass("dk/bearware/TextMessage");
    jobject tm_obj = NULL;

    if(lpTextMessage) {
        tm_obj = newObject(env, cls);
        assert(tm_obj);
        setTextMessage(env, const_cast<TextMessage&>(*lpTextMessage), tm_obj, N2J);
    }
    return tm_obj;
}

jobject newRemoteFile(JNIEnv* env, const RemoteFile* lpRemoteFile) {
    jclass cls = env->FindClass("dk/bearware/RemoteFile");
    jobject rf_obj = NULL;

    if(lpRemoteFile) {
        rf_obj = newObject(env, cls);
        assert(rf_obj);
        setRemoteFile(env, const_cast<RemoteFile&>(*lpRemoteFile), rf_obj, N2J);
    }
    return rf_obj;

}

jobject newServerProperties(JNIEnv* env, const ServerProperties* lpServerProperties) {
    jclass cls = env->FindClass("dk/bearware/ServerProperties");
    jobject sp_obj = NULL;

    if(lpServerProperties) {
        sp_obj = newObject(env, cls);
        assert(sp_obj);
        setServerProperties(env, const_cast<ServerProperties&>(*lpServerProperties), sp_obj, N2J);
    }
    return sp_obj;
}

jobject newAbusePrevention(JNIEnv* env, const AbusePrevention* lpAbusePrevent) {
    jclass cls = env->FindClass("dk/bearware/AbusePrevention");
    jobject ap_obj = NULL;
    if(lpAbusePrevent) {
        ap_obj = newObject(env, cls);
        assert(ap_obj);
        setAbusePrevention(env, const_cast<AbusePrevention&>(*lpAbusePrevent), ap_obj, N2J);
    }
    return ap_obj;
}

void setChannel(JNIEnv* env, Channel& chan, jobject lpChannel, JConvert conv)
{
    jclass cls_chan = env->GetObjectClass(lpChannel);
    jfieldID fid_parentid = env->GetFieldID(cls_chan, "nParentID", "I");
    jfieldID fid_chanid = env->GetFieldID(cls_chan, "nChannelID", "I");
    jfieldID fid_name = env->GetFieldID(cls_chan, "szName", "Ljava/lang/String;");
    jfieldID fid_topic = env->GetFieldID(cls_chan, "szTopic", "Ljava/lang/String;");
    jfieldID fid_passwd = env->GetFieldID(cls_chan, "szPassword", "Ljava/lang/String;");
    jfieldID fid_prot = env->GetFieldID(cls_chan, "bPassword", "Z");
    jfieldID fid_chantype = env->GetFieldID(cls_chan, "uChannelType", "I");
    jfieldID fid_userdata = env->GetFieldID(cls_chan, "nUserData", "I");
    jfieldID fid_quota = env->GetFieldID(cls_chan, "nDiskQuota", "J");
    jfieldID fid_oppasswd = env->GetFieldID(cls_chan, "szOpPassword", "Ljava/lang/String;");
    jfieldID fid_maxusers = env->GetFieldID(cls_chan, "nMaxUsers", "I");
    jfieldID fid_codec = env->GetFieldID(cls_chan, "audiocodec", "Ldk/bearware/AudioCodec;");
    jfieldID fid_audcfg = env->GetFieldID(cls_chan, "audiocfg", "Ldk/bearware/AudioConfig;");
    jfieldID fid_txusers = env->GetFieldID(cls_chan, "transmitUsers", "[[I");
    jfieldID fid_queueusers = env->GetFieldID(cls_chan, "transmitUsersQueue", "[I");

    assert(fid_parentid);
    assert(fid_chanid);
    assert(fid_name);
    assert(fid_topic);
    assert(fid_passwd);
    assert(fid_prot);
    assert(fid_codec);
    assert(fid_audcfg);
    assert(fid_chantype);
    assert(fid_userdata);
    assert(fid_quota);
    assert(fid_oppasswd);
    assert(fid_maxusers);
    assert(fid_txusers);
    assert(fid_queueusers);

    if(conv == N2J)
    {
        env->SetIntField(lpChannel, fid_parentid, chan.nParentID);
        env->SetIntField(lpChannel, fid_chanid, chan.nChannelID);
        env->SetObjectField(lpChannel, fid_name, NEW_JSTRING(env, chan.szName));
        env->SetObjectField(lpChannel, fid_topic, NEW_JSTRING(env, chan.szTopic));
        env->SetObjectField(lpChannel, fid_passwd, NEW_JSTRING(env, chan.szPassword));
        env->SetBooleanField(lpChannel, fid_prot, chan.bPassword);
        env->SetIntField(lpChannel, fid_chantype, chan.uChannelType);
        env->SetIntField(lpChannel, fid_userdata, chan.nUserData);
        env->SetLongField(lpChannel, fid_quota, chan.nDiskQuota);
        env->SetObjectField(lpChannel, fid_oppasswd, NEW_JSTRING(env, chan.szOpPassword));
        env->SetIntField(lpChannel, fid_maxusers, chan.nMaxUsers);

        jclass cls_codec = env->FindClass("dk/bearware/AudioCodec");
        jobject newObj = newObject(env, cls_codec);
        env->SetObjectField(lpChannel, fid_codec, newObj);

        jclass cls_audcfg = env->FindClass("dk/bearware/AudioConfig");
        newObj = newObject(env, cls_audcfg);
        env->SetObjectField(lpChannel, fid_audcfg, newObj);

        jintArray intArr = env->NewIntArray(TT_TRANSMITQUEUE_MAX);
        jobjectArray outer = env->NewObjectArray(TT_TRANSMITQUEUE_MAX, env->FindClass("[I"), intArr);
        for (int i=0;i<TT_TRANSMITQUEUE_MAX;++i) {
            intArr = env->NewIntArray(TRANSMITUSER_ARRAY_SIZE);
            jint tmp[TRANSMITUSER_ARRAY_SIZE];
            env->SetIntArrayRegion(intArr, 0, TRANSMITUSER_ARRAY_SIZE, TO_JINT_ARRAY(chan.transmitUsers[i], tmp, TRANSMITUSER_ARRAY_SIZE));
            env->SetObjectArrayElement(outer, i, intArr);
            env->DeleteLocalRef(intArr);
        }
        env->SetObjectField(lpChannel, fid_txusers, outer);
        
        intArr = env->NewIntArray(TT_TRANSMITQUEUE_MAX);
        jint tmp[TT_TRANSMITQUEUE_MAX] = {};
        env->SetIntArrayRegion(intArr, 0, TT_TRANSMITQUEUE_MAX, TO_JINT_ARRAY(chan.transmitUsersQueue, tmp, TT_TRANSMITQUEUE_MAX));
        env->SetObjectField(lpChannel, fid_queueusers, intArr);
    }
    else
    {
        chan.nParentID = env->GetIntField(lpChannel, fid_parentid);
        chan.nChannelID = env->GetIntField(lpChannel, fid_chanid);
        TT_STRCPY(chan.szName, ttstr(env, (jstring)env->GetObjectField(lpChannel, fid_name)));
        TT_STRCPY(chan.szTopic, ttstr(env, (jstring)env->GetObjectField(lpChannel, fid_topic)));
        TT_STRCPY(chan.szPassword, ttstr(env, (jstring)env->GetObjectField(lpChannel, fid_passwd)));
        chan.uChannelType = env->GetIntField(lpChannel, fid_chantype);
        chan.nUserData = env->GetIntField(lpChannel, fid_userdata);
        chan.nDiskQuota = env->GetLongField(lpChannel, fid_quota);
        TT_STRCPY(chan.szOpPassword, ttstr(env, (jstring)env->GetObjectField(lpChannel, fid_oppasswd)));
        chan.nMaxUsers = env->GetIntField(lpChannel, fid_maxusers);
        memset(chan.transmitUsers, 0, sizeof(chan.transmitUsers));
        jobjectArray outer = jobjectArray(env->GetObjectField(lpChannel, fid_txusers));
        for (int i=0;i<TT_TRANSMITQUEUE_MAX;++i) {
            jintArray intArr = jintArray(env->GetObjectArrayElement(outer, i));
            jint tmp[TRANSMITUSER_ARRAY_SIZE];
            env->GetIntArrayRegion(intArr, 0, TRANSMITUSER_ARRAY_SIZE, tmp);
            TO_INT32_ARRAY(tmp, chan.transmitUsers[i], TRANSMITUSER_ARRAY_SIZE);
            env->DeleteLocalRef(intArr);
        }
        jintArray intArr = (jintArray)env->GetObjectField(lpChannel, fid_queueusers);
        jint tmp[TT_TRANSMITQUEUE_MAX] = {};
        env->GetIntArrayRegion(intArr, 0, TT_TRANSMITQUEUE_MAX, tmp);
        TO_INT32_ARRAY(tmp, chan.transmitUsersQueue, TT_TRANSMITQUEUE_MAX);
    }

    setAudioCodec(env, chan.audiocodec, env->GetObjectField(lpChannel, fid_codec), conv);
    setAudioConfig(env, chan.audiocfg, env->GetObjectField(lpChannel, fid_audcfg), conv);
}

void setUser(JNIEnv* env, const User& user, jobject lpUser)
{
    jclass cls_user = env->GetObjectClass(lpUser);
    jfieldID fid_userid = env->GetFieldID(cls_user, "nUserID", "I");
    jfieldID fid_username = env->GetFieldID(cls_user, "szUsername", "Ljava/lang/String;");
    jfieldID fid_userdata = env->GetFieldID(cls_user, "nUserData", "I");
    jfieldID fid_usertype = env->GetFieldID(cls_user, "uUserType", "I");
    jfieldID fid_ipaddr = env->GetFieldID(cls_user, "szIPAddress", "Ljava/lang/String;");
    jfieldID fid_version = env->GetFieldID(cls_user, "uVersion", "I");
    jfieldID fid_chanid = env->GetFieldID(cls_user, "nChannelID", "I");
    jfieldID fid_lsub = env->GetFieldID(cls_user, "uLocalSubscriptions", "I");
    jfieldID fid_psub = env->GetFieldID(cls_user, "uPeerSubscriptions", "I");
    jfieldID fid_nickname = env->GetFieldID(cls_user, "szNickname", "Ljava/lang/String;");
    jfieldID fid_stmode = env->GetFieldID(cls_user, "nStatusMode", "I");
    jfieldID fid_stmsg = env->GetFieldID(cls_user, "szStatusMsg", "Ljava/lang/String;");
    jfieldID fid_state = env->GetFieldID(cls_user, "uUserState", "I");
    jfieldID fid_folder = env->GetFieldID(cls_user, "szMediaStorageDir", "Ljava/lang/String;");
    jfieldID fid_volvoice = env->GetFieldID(cls_user, "nVolumeVoice", "I");
    jfieldID fid_volmf = env->GetFieldID(cls_user, "nVolumeMediaFile", "I");
    jfieldID fid_stopvoice = env->GetFieldID(cls_user, "nStoppedDelayVoice", "I");
    jfieldID fid_stopmf = env->GetFieldID(cls_user, "nStoppedDelayMediaFile", "I");
    jfieldID fid_pbvoice = env->GetFieldID(cls_user, "stereoPlaybackVoice", "[Z");
    jfieldID fid_pbmf = env->GetFieldID(cls_user, "stereoPlaybackMediaFile", "[Z");
    jfieldID fid_mfbuf = env->GetFieldID(cls_user, "nBufferMSecMediaFile", "I");
    jfieldID fid_vbuf = env->GetFieldID(cls_user, "nBufferMSecVoice", "I");
    jfieldID fid_cltname = env->GetFieldID(cls_user, "szClientName", "Ljava/lang/String;");

    assert(fid_userid);
    assert(fid_username);
    assert(fid_userdata);
    assert(fid_usertype);
    assert(fid_ipaddr);
    assert(fid_version);
    assert(fid_chanid);
    assert(fid_lsub);
    assert(fid_psub);
    assert(fid_nickname);
    assert(fid_stmode);
    assert(fid_stmsg);
    assert(fid_state);
    assert(fid_folder);
    assert(fid_volvoice);
    assert(fid_volmf);
    assert(fid_stopvoice);
    assert(fid_stopmf);
    assert(fid_pbvoice);
    assert(fid_pbmf);
    assert(fid_mfbuf);
    assert(fid_vbuf);
    assert(fid_cltname);
    
    env->SetIntField(lpUser, fid_userid, user.nUserID);
    env->SetObjectField(lpUser, fid_username, NEW_JSTRING(env, user.szUsername));
    env->SetIntField(lpUser, fid_userdata, user.nUserData);
    env->SetIntField(lpUser, fid_usertype, user.uUserType);
    env->SetObjectField(lpUser, fid_ipaddr, NEW_JSTRING(env, user.szIPAddress));
    env->SetIntField(lpUser, fid_version, user.uVersion);
    env->SetIntField(lpUser, fid_chanid, user.nChannelID);
    env->SetIntField(lpUser, fid_lsub, user.uLocalSubscriptions);
    env->SetIntField(lpUser, fid_psub, user.uPeerSubscriptions);
    env->SetObjectField(lpUser, fid_nickname, NEW_JSTRING(env, user.szNickname));
    env->SetIntField(lpUser, fid_stmode, user.nStatusMode);
    env->SetObjectField(lpUser, fid_stmsg, NEW_JSTRING(env, user.szStatusMsg));
    env->SetIntField(lpUser, fid_state, user.uUserState);
    env->SetObjectField(lpUser, fid_folder, NEW_JSTRING(env, user.szMediaStorageDir));
    env->SetIntField(lpUser, fid_volvoice, user.nVolumeVoice);
    env->SetIntField(lpUser, fid_volmf, user.nVolumeMediaFile);
    env->SetIntField(lpUser, fid_stopvoice, user.nStoppedDelayVoice);
    env->SetIntField(lpUser, fid_stopmf, user.nStoppedDelayMediaFile);
    jbooleanArray boolArray = env->NewBooleanArray(2);
    jboolean tmp[2];
    tmp[0] = user.stereoPlaybackVoice[0] != 0;
    tmp[1] = user.stereoPlaybackVoice[1] != 0;
    env->SetBooleanArrayRegion(boolArray, 0, 2, tmp);
    env->SetObjectField(lpUser, fid_pbvoice, boolArray);
    boolArray = env->NewBooleanArray(2);
    tmp[0] = user.stereoPlaybackMediaFile[0] != 0;
    tmp[1] = user.stereoPlaybackMediaFile[1] != 0;
    env->SetBooleanArrayRegion(boolArray, 0, 2, tmp);
    env->SetObjectField(lpUser, fid_pbmf, boolArray);
    env->SetIntField(lpUser, fid_mfbuf, user.nBufferMSecMediaFile);
    env->SetIntField(lpUser, fid_vbuf, user.nBufferMSecVoice);
    env->SetObjectField(lpUser, fid_cltname, NEW_JSTRING(env, user.szClientName));
}

void setTTMessage(JNIEnv* env, TTMessage& msg, jobject pMsg)
{
    jclass ttmsg_class = env->GetObjectClass(pMsg);
    jfieldID fid_event = env->GetFieldID(ttmsg_class, "nClientEvent", "I");
    jfieldID fid_src = env->GetFieldID(ttmsg_class, "nSource", "I");
    jfieldID fid_type = env->GetFieldID(ttmsg_class, "ttType", "I");

    assert(fid_event);
    assert(fid_src);
    assert(fid_type);

    env->SetIntField(pMsg, fid_event, msg.nClientEvent);
    env->SetIntField(pMsg, fid_src, msg.nSource);
    env->SetIntField(pMsg, fid_type, msg.ttType);

    jfieldID fid_channel = env->GetFieldID(ttmsg_class, "channel", "Ldk/bearware/Channel;");
    jfieldID fid_cemsg = env->GetFieldID(ttmsg_class, "clienterrormsg", "Ldk/bearware/ClientErrorMsg;");
    jfieldID fid_deskinput = env->GetFieldID(ttmsg_class, "desktopinput", "Ldk/bearware/DesktopInput;");
    jfieldID fid_ftx = env->GetFieldID(ttmsg_class, "filetransfer", "Ldk/bearware/FileTransfer;");
    jfieldID fid_mfi = env->GetFieldID(ttmsg_class, "mediafileinfo", "Ldk/bearware/MediaFileInfo;");
    jfieldID fid_rfile = env->GetFieldID(ttmsg_class, "remotefile", "Ldk/bearware/RemoteFile;");
    jfieldID fid_srvp = env->GetFieldID(ttmsg_class, "serverproperties", "Ldk/bearware/ServerProperties;");
    jfieldID fid_srvs = env->GetFieldID(ttmsg_class, "serverstatistics", "Ldk/bearware/ServerStatistics;");
    jfieldID fid_txtmsg = env->GetFieldID(ttmsg_class, "textmessage", "Ldk/bearware/TextMessage;");
    jfieldID fid_usr = env->GetFieldID(ttmsg_class, "user", "Ldk/bearware/User;");
    jfieldID fid_acc = env->GetFieldID(ttmsg_class, "useraccount", "Ldk/bearware/UserAccount;");
    jfieldID fid_ban = env->GetFieldID(ttmsg_class, "banneduser", "Ldk/bearware/BannedUser;");
    jfieldID fid_act = env->GetFieldID(ttmsg_class, "bActive", "Z");
    jfieldID fid_bremain = env->GetFieldID(ttmsg_class, "nBytesRemain", "I");
    jfieldID fid_streamid = env->GetFieldID(ttmsg_class, "nStreamID", "I");
    jfieldID fid_payload = env->GetFieldID(ttmsg_class, "nPayloadSize", "I");
    jfieldID fid_st = env->GetFieldID(ttmsg_class, "nStreamType", "I");
    jfieldID fid_aip = env->GetFieldID(ttmsg_class, "audioinputprogress", "Ldk/bearware/AudioInputProgress;");

    assert(fid_channel);
    assert(fid_cemsg);
    assert(fid_deskinput);
    assert(fid_ftx);
    assert(fid_mfi);
    assert(fid_rfile);
    assert(fid_srvp);
    assert(fid_srvs);
    assert(fid_txtmsg);
    assert(fid_usr);
    assert(fid_acc);
    assert(fid_ban);
    assert(fid_act);
    assert(fid_bremain);
    assert(fid_streamid);
    assert(fid_payload);
    assert(fid_st);
    assert(fid_aip);

    switch(msg.ttType)
    {
    case __CHANNEL :
    {
        jclass cls_obj = env->FindClass("dk/bearware/Channel");
        jobject newObj = newObject(env, cls_obj);
        setChannel(env, msg.channel, newObj, N2J);
        env->SetObjectField(pMsg, fid_channel, newObj);
    }
    break;
    case __CLIENTERRORMSG :
    {
        jclass cls_obj = env->FindClass("dk/bearware/ClientErrorMsg");
        jobject newObj = newObject(env, cls_obj);
        setClientErrorMsg(env, msg.clienterrormsg, newObj, N2J);
        env->SetObjectField(pMsg, fid_cemsg, newObj);
    }
    break;
    case __DESKTOPINPUT :
    {
        jclass cls_obj = env->FindClass("dk/bearware/DesktopInput");
        jobject newObj = newObject(env, cls_obj);
        setDesktopInput(env, msg.desktopinput, newObj, N2J);
        env->SetObjectField(pMsg, fid_deskinput, newObj);
    }
    break;
    case __FILETRANSFER :
    {
        jclass cls_obj = env->FindClass("dk/bearware/FileTransfer");
        jobject newObj = newObject(env, cls_obj);
        setFileTransfer(env, msg.filetransfer, newObj);
        env->SetObjectField(pMsg, fid_ftx, newObj);
    }
    break;
    case __MEDIAFILEINFO :
    {
        jclass cls_obj = env->FindClass("dk/bearware/MediaFileInfo");
        jobject newObj = newObject(env, cls_obj);
        setMediaFileInfo(env, msg.mediafileinfo, newObj, N2J);
        env->SetObjectField(pMsg, fid_mfi, newObj);
    }
    break;
    case __REMOTEFILE :
    {
        jclass cls_obj = env->FindClass("dk/bearware/RemoteFile");
        jobject newObj = newObject(env, cls_obj);
        setRemoteFile(env, msg.remotefile, newObj, N2J);
        env->SetObjectField(pMsg, fid_rfile, newObj);
    }
    break;
    case __SERVERPROPERTIES :
    {
        jclass cls_obj = env->FindClass("dk/bearware/ServerProperties");
        jobject newObj = newObject(env, cls_obj);
        setServerProperties(env, msg.serverproperties, newObj, N2J);
        env->SetObjectField(pMsg, fid_srvp, newObj);
    }
    break;
    case __SERVERSTATISTICS :
    {
        jclass cls_obj = env->FindClass("dk/bearware/ServerStatistics");
        jobject newObj = newObject(env, cls_obj);
        setServerStatistics(env, msg.serverstatistics, newObj, N2J);
        env->SetObjectField(pMsg, fid_srvs, newObj);
    }
    break;
    case __TEXTMESSAGE :
    {
        jclass cls_obj = env->FindClass("dk/bearware/TextMessage");
        jobject newObj = newObject(env, cls_obj);
        setTextMessage(env, msg.textmessage, newObj, N2J);
        env->SetObjectField(pMsg, fid_txtmsg, newObj);
    }
    break;
    case __USER :
    {
        jclass cls_obj = env->FindClass("dk/bearware/User");
        jobject newObj = newObject(env, cls_obj);
        setUser(env, msg.user, newObj);
        env->SetObjectField(pMsg, fid_usr, newObj);
    }
    break;
    case __USERACCOUNT :
    {
        jclass cls_obj = env->FindClass("dk/bearware/UserAccount");
        jobject newObj = newObject(env, cls_obj);
        setUserAccount(env, msg.useraccount, newObj, N2J);
        env->SetObjectField(pMsg, fid_acc, newObj);
    }
    break;
    case __BANNEDUSER :
    {
        jclass cls_obj = env->FindClass("dk/bearware/BannedUser");
        jobject newObj = newObject(env, cls_obj);
        setBannedUser(env, msg.banneduser, newObj, N2J);
        env->SetObjectField(pMsg, fid_ban, newObj);        
    }
    break;
    case __TTBOOL :
        env->SetBooleanField(pMsg, fid_act, msg.bActive);
        break;
    case __INT32 :
        env->SetIntField(pMsg, fid_bremain, msg.nBytesRemain);
        env->SetIntField(pMsg, fid_streamid, msg.nStreamID);
        env->SetIntField(pMsg, fid_payload, msg.nPayloadSize);
        break;
    case __STREAMTYPE :
        env->SetIntField(pMsg, fid_st, msg.nStreamType);
        break;
    case __AUDIOINPUTPROGRESS :
    {
        jclass cls_obj = env->FindClass("dk/bearware/AudioInputProgress");
        jobject newObj = newObject(env, cls_obj);
        setAudioInputProgress(env, msg.audioinputprogress, newObj, N2J);
        env->SetObjectField(pMsg, fid_aip, newObj);
    }
    break;
    case __NONE :
        break;
    default :
        assert(0 /* unknown msg.ttType */);
        break;
    }
}

void setIntPtr(JNIEnv* env, jobject intptr, jint value)
{
    if(!intptr)
        return;

    jclass cls_intptr = env->GetObjectClass(intptr); //dk.bearware.IntPtr-class
    jfieldID fid_value = env->GetFieldID(cls_intptr, "value", "I");
    assert(fid_value);
    env->SetIntField(intptr, fid_value, value);
}

jint getIntPtr(JNIEnv* env, jobject intptr)
{
    if(!intptr)
        return 0;

    jclass cls_intptr = env->GetObjectClass(intptr); //dk.bearware.IntPtr-class
    jfieldID fid_value = env->GetFieldID(cls_intptr, "value", "I");
    assert(fid_value);
    return env->GetIntField(intptr, fid_value);
}

void setAudioCodec(JNIEnv* env, AudioCodec& codec, jobject lpAudioCodec, JConvert conv)
{
    jclass cls_codec = env->GetObjectClass(lpAudioCodec);
    jfieldID fid_codec = env->GetFieldID(cls_codec, "nCodec", "I");
    assert(fid_codec);

    int conv_codec;
    if(conv == N2J)
    {
        conv_codec = codec.nCodec;
        env->SetIntField(lpAudioCodec, fid_codec, codec.nCodec);
    }
    else
    {
        ZERO_STRUCT(codec);
        conv_codec = env->GetIntField(lpAudioCodec, fid_codec);
        codec.nCodec = (Codec)env->GetIntField(lpAudioCodec, fid_codec);
    }

    switch(conv_codec)
    {
    case SPEEX_CODEC :
    {
        jfieldID fid_speex = env->GetFieldID(cls_codec,
                                             "speex",
                                             "Ldk/bearware/SpeexCodec;");
        jclass cls_spx = env->FindClass("dk/bearware/SpeexCodec");
        jfieldID fid_bandmode = env->GetFieldID(cls_spx,
                                                "nBandmode", "I");
        jfieldID fid_quality = env->GetFieldID(cls_spx,
                                               "nQuality", "I");
        jfieldID fid_msec = env->GetFieldID(cls_spx, 
                                            "nTxIntervalMSec", "I");
        jfieldID fid_stereo = env->GetFieldID(cls_spx,
                                              "bStereoPlayback", "Z");
        assert(fid_speex);
        assert(fid_bandmode);
        assert(fid_quality);
        assert(fid_msec);
        assert(fid_stereo);

        if(conv == N2J)
        {
            jobject newObj = newObject(env, cls_spx);

            env->SetIntField(newObj, fid_bandmode, codec.speex.nBandmode);
            env->SetIntField(newObj, fid_quality, codec.speex.nQuality);
            env->SetIntField(newObj, fid_msec, codec.speex.nTxIntervalMSec);
            env->SetBooleanField(newObj, fid_stereo, codec.speex.bStereoPlayback);
            env->SetObjectField(lpAudioCodec, fid_speex, newObj);
        }
        else
        {
            jobject speex_obj = env->GetObjectField(lpAudioCodec, fid_speex);
            assert(speex_obj);
            codec.speex.nBandmode = env->GetIntField(speex_obj, fid_bandmode);
            codec.speex.nQuality = env->GetIntField(speex_obj, fid_quality);
            codec.speex.nTxIntervalMSec = env->GetIntField(speex_obj, fid_msec);
            codec.speex.bStereoPlayback = env->GetBooleanField(speex_obj, fid_stereo);
        }
    }
    break;
    case SPEEX_VBR_CODEC :
    {
        jfieldID fid_speexvbr = env->GetFieldID(cls_codec,
                                                "speex_vbr",
                                                "Ldk/bearware/SpeexVBRCodec;");

        jclass cls_spx = env->FindClass("dk/bearware/SpeexVBRCodec");
        jfieldID fid_bandmode = env->GetFieldID(cls_spx,
                                                "nBandmode", "I");
        jfieldID fid_quality = env->GetFieldID(cls_spx,
                                               "nQuality", "I");
        jfieldID fid_bitrate = env->GetFieldID(cls_spx, 
                                               "nBitRate", "I");
        jfieldID fid_maxbitrate = env->GetFieldID(cls_spx, 
                                                  "nMaxBitRate", "I");
        jfieldID fid_msec = env->GetFieldID(cls_spx, 
                                            "nTxIntervalMSec", "I");
        jfieldID fid_dtx = env->GetFieldID(cls_spx,
                                           "bDTX", "Z");
        jfieldID fid_stereo = env->GetFieldID(cls_spx,
                                              "bStereoPlayback", "Z");
        assert(fid_speexvbr);
        assert(fid_stereo);
        assert(fid_dtx);
        assert(fid_msec);
        assert(fid_maxbitrate);
        assert(fid_bitrate);
        assert(fid_quality);
        assert(fid_bandmode);

        if(conv == N2J)
        {
            jobject newObj = newObject(env, cls_spx);

            env->SetIntField(newObj, fid_bandmode, codec.speex_vbr.nBandmode);
            env->SetIntField(newObj, fid_quality, codec.speex_vbr.nQuality);
            env->SetIntField(newObj, fid_bitrate, codec.speex_vbr.nBitRate);
            env->SetIntField(newObj, fid_maxbitrate, codec.speex_vbr.nMaxBitRate);
            env->SetBooleanField(newObj, fid_dtx, codec.speex_vbr.bDTX);
            env->SetIntField(newObj, fid_msec, codec.speex_vbr.nTxIntervalMSec);
            env->SetBooleanField(newObj, fid_stereo, codec.speex_vbr.bStereoPlayback);
            env->SetObjectField(lpAudioCodec, fid_speexvbr, newObj);
        }
        else
        {
            jobject speexvbr_obj = env->GetObjectField(lpAudioCodec, fid_speexvbr);
            codec.speex_vbr.nBandmode = env->GetIntField(speexvbr_obj, fid_bandmode);
            codec.speex_vbr.nQuality = env->GetIntField(speexvbr_obj, fid_quality);
            codec.speex_vbr.nBitRate = env->GetIntField(speexvbr_obj, fid_bitrate);
            codec.speex_vbr.nMaxBitRate = env->GetIntField(speexvbr_obj, fid_maxbitrate);
            codec.speex_vbr.bDTX = env->GetBooleanField(speexvbr_obj, fid_dtx);
            codec.speex_vbr.nTxIntervalMSec = env->GetIntField(speexvbr_obj, fid_msec);
            codec.speex_vbr.bStereoPlayback = env->GetBooleanField(speexvbr_obj, fid_stereo);
        }
    }
    break;
    case OPUS_CODEC :
    {
        jfieldID fid_opus = env->GetFieldID(cls_codec, "opus",
                                            "Ldk/bearware/OpusCodec;");

        jclass cls_opus = env->FindClass("dk/bearware/OpusCodec");
        jfieldID fid_sr = env->GetFieldID(cls_opus, "nSampleRate", "I");
        jfieldID fid_ch = env->GetFieldID(cls_opus, "nChannels", "I");
        jfieldID fid_app = env->GetFieldID(cls_opus, "nApplication", "I");
        jfieldID fid_comp = env->GetFieldID(cls_opus, "nComplexity", "I");
        jfieldID fid_fec = env->GetFieldID(cls_opus, "bFEC", "Z");
        jfieldID fid_dtx = env->GetFieldID(cls_opus, "bDTX", "Z");
        jfieldID fid_br = env->GetFieldID(cls_opus, "nBitRate", "I");
        jfieldID fid_vbr = env->GetFieldID(cls_opus, "bVBR", "Z");
        jfieldID fid_vbrc = env->GetFieldID(cls_opus, "bVBRConstraint", "Z");
        jfieldID fid_txmsec = env->GetFieldID(cls_opus, "nTxIntervalMSec", "I");
        jfieldID fid_frmmsec = env->GetFieldID(cls_opus, "nFrameSizeMSec", "I");

        assert(fid_opus);
        assert(fid_sr);
        assert(fid_ch);
        assert(fid_app);
        assert(fid_comp);
        assert(fid_fec);
        assert(fid_dtx);
        assert(fid_br);
        assert(fid_vbr);
        assert(fid_vbrc);
        assert(fid_txmsec);
        assert(fid_frmmsec);

        if(conv == N2J)
        {
            jobject newObj = newObject(env, cls_opus);

            env->SetIntField(newObj, fid_sr, codec.opus.nSampleRate);
            env->SetIntField(newObj, fid_ch, codec.opus.nChannels);
            env->SetIntField(newObj, fid_app, codec.opus.nApplication);
            env->SetIntField(newObj, fid_comp, codec.opus.nComplexity);
            env->SetBooleanField(newObj, fid_fec, codec.opus.bFEC);
            env->SetBooleanField(newObj, fid_dtx, codec.opus.bDTX);
            env->SetIntField(newObj, fid_br, codec.opus.nBitRate);
            env->SetBooleanField(newObj, fid_vbr, codec.opus.bVBR);
            env->SetBooleanField(newObj, fid_vbrc, codec.opus.bVBRConstraint);
            env->SetIntField(newObj, fid_txmsec, codec.opus.nTxIntervalMSec);
            env->SetIntField(newObj, fid_frmmsec, codec.opus.nFrameSizeMSec);
            env->SetObjectField(lpAudioCodec, fid_opus, newObj);
        }
        else
        {
            jobject opus_obj = env->GetObjectField(lpAudioCodec, fid_opus);
            codec.opus.nSampleRate = env->GetIntField(opus_obj, fid_sr);
            codec.opus.nChannels = env->GetIntField(opus_obj, fid_ch);
            codec.opus.nApplication = env->GetIntField(opus_obj, fid_app);
            codec.opus.nComplexity = env->GetIntField(opus_obj, fid_comp);
            codec.opus.bFEC = env->GetBooleanField(opus_obj, fid_fec);
            codec.opus.bDTX = env->GetBooleanField(opus_obj, fid_dtx);
            codec.opus.nBitRate = env->GetIntField(opus_obj, fid_br);
            codec.opus.bVBR = env->GetBooleanField(opus_obj, fid_vbr);
            codec.opus.bVBRConstraint = env->GetBooleanField(opus_obj, fid_vbrc);
            codec.opus.nTxIntervalMSec = env->GetIntField(opus_obj, fid_txmsec);
            codec.opus.nFrameSizeMSec = env->GetIntField(opus_obj, fid_frmmsec);
        }
    }
    break;
    }
}

void setAudioConfig(JNIEnv* env, AudioConfig& audcfg, jobject lpAudioConfig, JConvert conv)
{
    jclass cls_cfg = env->GetObjectClass(lpAudioConfig);
    jfieldID fid_agc = env->GetFieldID(cls_cfg, "bEnableAGC", "Z");
    jfieldID fid_gainlevel = env->GetFieldID(cls_cfg, "nGainLevel", "I");
    assert(fid_agc);
    assert(fid_gainlevel);

    if(conv == N2J)
    {
        env->SetBooleanField(lpAudioConfig, fid_agc, audcfg.bEnableAGC);
        env->SetIntField(lpAudioConfig, fid_gainlevel, audcfg.nGainLevel);
    }
    else
    {
        ZERO_STRUCT(audcfg);
        audcfg.bEnableAGC = env->GetBooleanField(lpAudioConfig, fid_agc);
        audcfg.nGainLevel = env->GetIntField(lpAudioConfig, fid_gainlevel);
    }
}

void setSoundDeviceEffects(JNIEnv* env, SoundDeviceEffects& effects, jobject lpSoundDeviceEffects, JConvert conv) {
    jclass cls = env->GetObjectClass(lpSoundDeviceEffects);
    jfieldID fid_aec = env->GetFieldID(cls, "bEnableEchoCancellation", "Z");
    jfieldID fid_agc = env->GetFieldID(cls, "bEnableAGC", "Z");
    jfieldID fid_denoise = env->GetFieldID(cls, "bEnableDenoise", "Z");

    assert(fid_aec);
    assert(fid_agc);
    assert(fid_denoise);

    if (conv == N2J)
    {
        env->SetBooleanField(lpSoundDeviceEffects, fid_aec, effects.bEnableEchoCancellation);
        env->SetBooleanField(lpSoundDeviceEffects, fid_agc, effects.bEnableAGC);
        env->SetBooleanField(lpSoundDeviceEffects, fid_denoise, effects.bEnableDenoise);
    }
    else
    {
        effects.bEnableEchoCancellation = env->GetBooleanField(lpSoundDeviceEffects, fid_aec);
        effects.bEnableAGC = env->GetBooleanField(lpSoundDeviceEffects, fid_agc);
        effects.bEnableDenoise = env->GetBooleanField(lpSoundDeviceEffects, fid_denoise);
    }
}


void setSpeexDSP(JNIEnv* env, SpeexDSP& spxdsp, jobject lpSpeexDSP, JConvert conv)
{
    jclass cls_cfg = env->GetObjectClass(lpSpeexDSP);
    jfieldID fid_agc = env->GetFieldID(cls_cfg, "bEnableAGC", "Z");
    jfieldID fid_gainlevel = env->GetFieldID(cls_cfg, "nGainLevel", "I");
    jfieldID fid_maxinc = env->GetFieldID(cls_cfg, "nMaxIncDBSec", "I");
    jfieldID fid_maxdec = env->GetFieldID(cls_cfg, "nMaxDecDBSec", "I");
    jfieldID fid_maxgain = env->GetFieldID(cls_cfg, "nMaxGainDB", "I");
    jfieldID fid_denoise = env->GetFieldID(cls_cfg, "bEnableDenoise", "Z");
    jfieldID fid_noise = env->GetFieldID(cls_cfg, "nMaxNoiseSuppressDB", "I");
    jfieldID fid_echo = env->GetFieldID(cls_cfg, "bEnableEchoCancellation", "Z");
    jfieldID fid_suppress = env->GetFieldID(cls_cfg, "nEchoSuppress", "I");
    jfieldID fid_supact = env->GetFieldID(cls_cfg, "nEchoSuppressActive", "I");
    assert(fid_agc);
    assert(fid_gainlevel);
    assert(fid_maxinc);
    assert(fid_maxdec);
    assert(fid_maxgain);
    assert(fid_denoise);
    assert(fid_noise);
    assert(fid_echo);
    assert(fid_suppress);
    assert(fid_supact);

    if(conv == N2J)
    {
        env->SetBooleanField(lpSpeexDSP, fid_agc, spxdsp.bEnableAGC);
        env->SetIntField(lpSpeexDSP, fid_gainlevel, spxdsp.nGainLevel);
        env->SetIntField(lpSpeexDSP, fid_maxinc, spxdsp.nMaxIncDBSec);
        env->SetIntField(lpSpeexDSP, fid_maxdec, spxdsp.nMaxDecDBSec);
        env->SetIntField(lpSpeexDSP, fid_maxgain, spxdsp.nMaxGainDB);
        env->SetBooleanField(lpSpeexDSP, fid_denoise, spxdsp.bEnableDenoise);
        env->SetIntField(lpSpeexDSP, fid_noise, spxdsp.nMaxNoiseSuppressDB);
        env->SetBooleanField(lpSpeexDSP, fid_echo, spxdsp.bEnableEchoCancellation);
        env->SetIntField(lpSpeexDSP, fid_suppress, spxdsp.nEchoSuppress);
        env->SetIntField(lpSpeexDSP, fid_supact, spxdsp.nEchoSuppressActive);
    }
    else
    {
        ZERO_STRUCT(spxdsp);
        spxdsp.bEnableAGC = env->GetBooleanField(lpSpeexDSP, fid_agc);
        spxdsp.nGainLevel = env->GetIntField(lpSpeexDSP, fid_gainlevel);
        spxdsp.nMaxIncDBSec = env->GetIntField(lpSpeexDSP, fid_maxinc);
        spxdsp.nMaxDecDBSec = env->GetIntField(lpSpeexDSP, fid_maxdec);
        spxdsp.nMaxGainDB = env->GetIntField(lpSpeexDSP, fid_maxgain);
        spxdsp.bEnableDenoise = env->GetBooleanField(lpSpeexDSP, fid_denoise);
        spxdsp.nMaxNoiseSuppressDB = env->GetIntField(lpSpeexDSP, fid_noise);
        spxdsp.bEnableEchoCancellation = env->GetBooleanField(lpSpeexDSP, fid_echo);
        spxdsp.nEchoSuppress = env->GetIntField(lpSpeexDSP, fid_suppress);
        spxdsp.nEchoSuppressActive = env->GetIntField(lpSpeexDSP, fid_supact);
    }
}

void setTTAudioPreprocessor(JNIEnv* env, TTAudioPreprocessor& preprocessor, jobject lpPreprocessor, JConvert conv) {
    jclass cls = env->GetObjectClass(lpPreprocessor);
    jfieldID fid_gain = env->GetFieldID(cls, "nGainLevel", "I");
    jfieldID fid_left = env->GetFieldID(cls, "bMuteLeftSpeaker", "Z");
    jfieldID fid_right = env->GetFieldID(cls, "bMuteRightSpeaker", "Z");

    assert(fid_gain);
    assert(fid_left);
    assert(fid_right);

    if (conv == N2J)
    {
        env->SetIntField(lpPreprocessor, fid_gain, preprocessor.nGainLevel);
        env->SetBooleanField(lpPreprocessor, fid_left, preprocessor.bMuteLeftSpeaker);
        env->SetBooleanField(lpPreprocessor, fid_right, preprocessor.bMuteRightSpeaker);
    }
    else
    {
        preprocessor.nGainLevel = env->GetIntField(lpPreprocessor, fid_gain);
        preprocessor.bMuteLeftSpeaker = env->GetBooleanField(lpPreprocessor, fid_left);
        preprocessor.bMuteRightSpeaker = env->GetBooleanField(lpPreprocessor, fid_right);
    }
}

void setAudioPreprocessor(JNIEnv* env, AudioPreprocessor& preprocessor, jobject lpPreprocessor, JConvert conv) {
    jclass cls = env->GetObjectClass(lpPreprocessor);
    jfieldID fid_type = env->GetFieldID(cls, "nPreprocessor", "I");
    jfieldID fid_spx = env->GetFieldID(cls, "speexdsp", "Ldk/bearware/SpeexDSP;");
    jfieldID fid_ttp = env->GetFieldID(cls, "ttpreprocessor", "Ldk/bearware/TTAudioPreprocessor;");

    assert(fid_type);
    assert(fid_spx);
    assert(fid_ttp);

    if (conv == N2J)
        env->SetIntField(lpPreprocessor, fid_type, preprocessor.nPreprocessor);
    else
        preprocessor.nPreprocessor = AudioPreprocessorType(env->GetIntField(lpPreprocessor, fid_type));
    
    jobject spx = env->GetObjectField(lpPreprocessor, fid_spx);
    jobject ttp = env->GetObjectField(lpPreprocessor, fid_ttp);
    
    switch (preprocessor.nPreprocessor) {
    case NO_AUDIOPREPROCESSOR :
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
        setSpeexDSP(env, preprocessor.speexdsp, spx, conv);
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        setTTAudioPreprocessor(env, preprocessor.ttpreprocessor, ttp, conv);
        break;
    }
}

void setServerProperties(JNIEnv* env, ServerProperties& srvprop, jobject lpServerProperties, JConvert conv)
{
    jclass cls_srv = env->GetObjectClass(lpServerProperties);
    
    jfieldID fid_name = env->GetFieldID(cls_srv, "szServerName", "Ljava/lang/String;");
    jfieldID fid_motd = env->GetFieldID(cls_srv, "szMOTD", "Ljava/lang/String;");
    jfieldID fid_motdraw = env->GetFieldID(cls_srv, "szMOTDRaw", "Ljava/lang/String;");
    jfieldID fid_maxusers = env->GetFieldID(cls_srv, "nMaxUsers", "I");
    jfieldID fid_maxattempts = env->GetFieldID(cls_srv, "nMaxLoginAttempts", "I");
    jfieldID fid_logindelay = env->GetFieldID(cls_srv, "nLoginDelayMSec", "I");
    jfieldID fid_iplogins = env->GetFieldID(cls_srv, "nMaxLoginsPerIPAddress", "I");
    jfieldID fid_voicetx = env->GetFieldID(cls_srv, "nMaxVoiceTxPerSecond", "I");
    jfieldID fid_vidcaptx = env->GetFieldID(cls_srv, "nMaxVideoCaptureTxPerSecond", "I");
    jfieldID fid_mftx = env->GetFieldID(cls_srv, "nMaxMediaFileTxPerSecond", "I");
    jfieldID fid_desktx = env->GetFieldID(cls_srv, "nMaxDesktopTxPerSecond", "I");
    jfieldID fid_totaltx = env->GetFieldID(cls_srv, "nMaxTotalTxPerSecond", "I");
    jfieldID fid_save = env->GetFieldID(cls_srv, "bAutoSave", "Z");
    jfieldID fid_tcp = env->GetFieldID(cls_srv, "nTcpPort", "I");
    jfieldID fid_udp = env->GetFieldID(cls_srv, "nUdpPort", "I");
    jfieldID fid_tmout = env->GetFieldID(cls_srv, "nUserTimeout", "I");
    jfieldID fid_srvver = env->GetFieldID(cls_srv, "szServerVersion", "Ljava/lang/String;");
    jfieldID fid_srvprot = env->GetFieldID(cls_srv, "szServerProtocolVersion", "Ljava/lang/String;");
    jfieldID fid_access = env->GetFieldID(cls_srv, "szAccessToken", "Ljava/lang/String;");

    assert(fid_name);
    assert(fid_motd);
    assert(fid_motdraw);
    assert(fid_maxusers);
    assert(fid_maxattempts);
    assert(fid_logindelay);
    assert(fid_iplogins);
    assert(fid_voicetx);
    assert(fid_vidcaptx);
    assert(fid_mftx);
    assert(fid_desktx);
    assert(fid_totaltx);
    assert(fid_save);
    assert(fid_tcp);
    assert(fid_udp);
    assert(fid_tmout);
    assert(fid_srvver);
    assert(fid_srvprot);
    assert(fid_access);

    if(conv == N2J)
    {
        env->SetObjectField(lpServerProperties, fid_name, NEW_JSTRING(env, srvprop.szServerName));
        env->SetObjectField(lpServerProperties, fid_motd, NEW_JSTRING(env, srvprop.szMOTD));
        env->SetObjectField(lpServerProperties, fid_motdraw, NEW_JSTRING(env, srvprop.szMOTDRaw));
        env->SetIntField(lpServerProperties, fid_maxusers, srvprop.nMaxUsers);
        env->SetIntField(lpServerProperties, fid_maxattempts, srvprop.nMaxLoginAttempts);
        env->SetIntField(lpServerProperties, fid_logindelay, srvprop.nLoginDelayMSec);
        env->SetIntField(lpServerProperties, fid_iplogins, srvprop.nMaxLoginsPerIPAddress);
        env->SetIntField(lpServerProperties, fid_voicetx, srvprop.nMaxVoiceTxPerSecond);
        env->SetIntField(lpServerProperties, fid_vidcaptx, srvprop.nMaxVideoCaptureTxPerSecond);
        env->SetIntField(lpServerProperties, fid_mftx, srvprop.nMaxMediaFileTxPerSecond);
        env->SetIntField(lpServerProperties, fid_desktx, srvprop.nMaxDesktopTxPerSecond);
        env->SetIntField(lpServerProperties, fid_totaltx, srvprop.nMaxTotalTxPerSecond);
        env->SetBooleanField(lpServerProperties, fid_save, srvprop.bAutoSave);
        env->SetIntField(lpServerProperties, fid_tcp, srvprop.nTcpPort);
        env->SetIntField(lpServerProperties, fid_udp, srvprop.nUdpPort);
        env->SetIntField(lpServerProperties, fid_tmout, srvprop.nUserTimeout);
        env->SetObjectField(lpServerProperties, fid_srvver, NEW_JSTRING(env, srvprop.szServerVersion));
        env->SetObjectField(lpServerProperties, fid_srvprot, NEW_JSTRING(env, srvprop.szServerProtocolVersion));
        env->SetObjectField(lpServerProperties, fid_access, NEW_JSTRING(env, srvprop.szAccessToken));
    }
    else
    {
        ZERO_STRUCT(srvprop);
        TT_STRCPY(srvprop.szServerName, ttstr(env, (jstring)env->GetObjectField(lpServerProperties, fid_name)));
        TT_STRCPY(srvprop.szMOTD, ttstr(env, (jstring)env->GetObjectField(lpServerProperties, fid_motd)));
        TT_STRCPY(srvprop.szMOTDRaw, ttstr(env, (jstring)env->GetObjectField(lpServerProperties, fid_motdraw)));
        srvprop.nMaxUsers = env->GetIntField(lpServerProperties, fid_maxusers);
        srvprop.nMaxLoginAttempts = env->GetIntField(lpServerProperties, fid_maxattempts);
        srvprop.nMaxLoginsPerIPAddress = env->GetIntField(lpServerProperties, fid_iplogins);
        srvprop.nLoginDelayMSec = env->GetIntField(lpServerProperties, fid_logindelay);
        srvprop.nMaxVoiceTxPerSecond = env->GetIntField(lpServerProperties, fid_voicetx);
        srvprop.nMaxVideoCaptureTxPerSecond = env->GetIntField(lpServerProperties, fid_vidcaptx);
        srvprop.nMaxMediaFileTxPerSecond = env->GetIntField(lpServerProperties, fid_mftx);
        srvprop.nMaxDesktopTxPerSecond = env->GetIntField(lpServerProperties, fid_desktx);
        srvprop.nMaxTotalTxPerSecond = env->GetIntField(lpServerProperties, fid_totaltx);
        srvprop.bAutoSave = env->GetBooleanField(lpServerProperties, fid_save);
        srvprop.nTcpPort = env->GetIntField(lpServerProperties, fid_tcp);
        srvprop.nUdpPort = env->GetIntField(lpServerProperties, fid_udp);
        srvprop.nUserTimeout = env->GetIntField(lpServerProperties, fid_tmout);
        TT_STRCPY(srvprop.szServerVersion, ttstr(env, (jstring)env->GetObjectField(lpServerProperties, fid_srvver)));
        TT_STRCPY(srvprop.szServerProtocolVersion, ttstr(env, (jstring)env->GetObjectField(lpServerProperties, fid_srvprot)));
        TT_STRCPY(srvprop.szAccessToken, ttstr(env, (jstring)env->GetObjectField(lpServerProperties, fid_access)));
    }
}

void setClientStatistics(JNIEnv* env, ClientStatistics& stats, jobject lpStats)
{
    jclass cls_stats = env->GetObjectClass(lpStats);

    jfieldID fid_udpsent = env->GetFieldID(cls_stats, "nUdpBytesSent", "J");
    jfieldID fid_udprecv = env->GetFieldID(cls_stats, "nUdpBytesRecv", "J");
    jfieldID fid_voicesent = env->GetFieldID(cls_stats, "nVoiceBytesSent", "J");
    jfieldID fid_voicerecv = env->GetFieldID(cls_stats, "nVoiceBytesRecv", "J");
    jfieldID fid_videosent = env->GetFieldID(cls_stats, "nVideoCaptureBytesSent", "J");
    jfieldID fid_videorecv = env->GetFieldID(cls_stats, "nVideoCaptureBytesRecv", "J");
    jfieldID fid_mfaudsent = env->GetFieldID(cls_stats, "nMediaFileAudioBytesSent", "J");
    jfieldID fid_mfaudrecv = env->GetFieldID(cls_stats, "nMediaFileAudioBytesRecv", "J");
    jfieldID fid_mfvidsent = env->GetFieldID(cls_stats, "nMediaFileVideoBytesSent", "J");
    jfieldID fid_mfvidrecv = env->GetFieldID(cls_stats, "nMediaFileVideoBytesRecv", "J");
    jfieldID fid_desksent = env->GetFieldID(cls_stats, "nDesktopBytesSent", "J");
    jfieldID fid_deskrecv = env->GetFieldID(cls_stats, "nDesktopBytesRecv", "J");
    jfieldID fid_udpping = env->GetFieldID(cls_stats, "nUdpPingTimeMs", "I");
    jfieldID fid_tcpping = env->GetFieldID(cls_stats, "nTcpPingTimeMs", "I");
    jfieldID fid_tcpsilen = env->GetFieldID(cls_stats, "nTcpServerSilenceSec", "I");
    jfieldID fid_udpsilen = env->GetFieldID(cls_stats, "nUdpServerSilenceSec", "I");

    assert(fid_udpsent);
    assert(fid_udprecv);
    assert(fid_voicesent);
    assert(fid_voicerecv);
    assert(fid_videosent);
    assert(fid_videorecv);
    assert(fid_mfaudsent);
    assert(fid_mfaudrecv);
    assert(fid_mfvidsent);
    assert(fid_mfvidrecv);
    assert(fid_desksent);
    assert(fid_deskrecv);
    assert(fid_udpping);
    assert(fid_tcpping);
    assert(fid_tcpsilen);
    assert(fid_udpsilen);

    env->SetLongField(lpStats, fid_udpsent, stats.nUdpBytesSent);
    env->SetLongField(lpStats, fid_udprecv, stats.nUdpBytesRecv);
    env->SetLongField(lpStats, fid_voicesent, stats.nVoiceBytesSent);
    env->SetLongField(lpStats, fid_voicerecv, stats.nVoiceBytesRecv);
    env->SetLongField(lpStats, fid_videosent, stats.nVideoCaptureBytesSent);
    env->SetLongField(lpStats, fid_videorecv, stats.nVideoCaptureBytesRecv);
    env->SetLongField(lpStats, fid_mfaudsent, stats.nMediaFileAudioBytesSent);
    env->SetLongField(lpStats, fid_mfaudrecv, stats.nMediaFileAudioBytesRecv);
    env->SetLongField(lpStats, fid_mfvidsent, stats.nMediaFileVideoBytesSent);
    env->SetLongField(lpStats, fid_mfvidrecv, stats.nMediaFileVideoBytesRecv);
    env->SetLongField(lpStats, fid_desksent, stats.nDesktopBytesSent);
    env->SetLongField(lpStats, fid_deskrecv, stats.nDesktopBytesRecv);
    env->SetIntField(lpStats, fid_udpping, stats.nUdpPingTimeMs);
    env->SetIntField(lpStats, fid_tcpping, stats.nTcpPingTimeMs);
    env->SetIntField(lpStats, fid_tcpsilen, stats.nTcpServerSilenceSec);
    env->SetIntField(lpStats, fid_udpsilen, stats.nUdpServerSilenceSec);
}

void setClientKeepAlive(JNIEnv* env, ClientKeepAlive& ka, jobject lpClientKeepAlive, JConvert conv) {

    jclass cls_ka = env->GetObjectClass(lpClientKeepAlive);

    jfieldID fid_conlost = env->GetFieldID(cls_ka, "nConnectionLostMSec", "I");
    jfieldID fid_tcpka = env->GetFieldID(cls_ka, "nTcpKeepAliveIntervalMSec", "I");
    jfieldID fid_udpka = env->GetFieldID(cls_ka, "nUdpKeepAliveIntervalMSec", "I");
    jfieldID fid_udprtx = env->GetFieldID(cls_ka, "nUdpKeepAliveRTXMSec", "I");
    jfieldID fid_udpcon = env->GetFieldID(cls_ka, "nUdpConnectRTXMSec", "I");
    jfieldID fid_udptm = env->GetFieldID(cls_ka, "nUdpConnectTimeoutMSec", "I");

    if(conv == N2J)
    {
        env->SetIntField(lpClientKeepAlive, fid_conlost, ka.nConnectionLostMSec);
        env->SetIntField(lpClientKeepAlive, fid_tcpka, ka.nTcpKeepAliveIntervalMSec);
        env->SetIntField(lpClientKeepAlive, fid_udpka, ka.nUdpKeepAliveIntervalMSec);
        env->SetIntField(lpClientKeepAlive, fid_udprtx, ka.nUdpKeepAliveRTXMSec);
        env->SetIntField(lpClientKeepAlive, fid_udpcon, ka.nUdpConnectRTXMSec);
        env->SetIntField(lpClientKeepAlive, fid_udptm, ka.nUdpConnectTimeoutMSec);
    }
    else
    {
        ka.nConnectionLostMSec = env->GetIntField(lpClientKeepAlive, fid_conlost);
        ka.nTcpKeepAliveIntervalMSec = env->GetIntField(lpClientKeepAlive, fid_tcpka);
        ka.nUdpKeepAliveIntervalMSec = env->GetIntField(lpClientKeepAlive, fid_udpka);
        ka.nUdpKeepAliveRTXMSec = env->GetIntField(lpClientKeepAlive, fid_udprtx);
        ka.nUdpConnectRTXMSec = env->GetIntField(lpClientKeepAlive, fid_udpcon);
        ka.nUdpConnectTimeoutMSec = env->GetIntField(lpClientKeepAlive, fid_udptm);
    }
}

void setTextMessage(JNIEnv* env, TextMessage& msg, jobject lpTextMessage, JConvert conv)
{
    jclass cls_txtmsg = env->GetObjectClass(lpTextMessage);
    
    jfieldID fid_type = env->GetFieldID(cls_txtmsg, "nMsgType", "I");
    jfieldID fid_fromid = env->GetFieldID(cls_txtmsg, "nFromUserID", "I");
    jfieldID fid_username = env->GetFieldID(cls_txtmsg, "szFromUsername", "Ljava/lang/String;");
    jfieldID fid_toid = env->GetFieldID(cls_txtmsg, "nToUserID", "I");
    jfieldID fid_chanid = env->GetFieldID(cls_txtmsg, "nChannelID", "I");
    jfieldID fid_msg = env->GetFieldID(cls_txtmsg, "szMessage", "Ljava/lang/String;");

    assert(fid_type);
    assert(fid_fromid);
    assert(fid_username);
    assert(fid_toid);
    assert(fid_chanid);
    assert(fid_msg);

    if(conv == N2J)
    {
        env->SetIntField(lpTextMessage, fid_type, msg.nMsgType);
        env->SetIntField(lpTextMessage, fid_fromid, msg.nFromUserID);
        env->SetObjectField(lpTextMessage, fid_username, NEW_JSTRING(env, msg.szFromUsername));
        env->SetIntField(lpTextMessage, fid_toid, msg.nToUserID);
        env->SetIntField(lpTextMessage, fid_chanid, msg.nChannelID);
        env->SetObjectField(lpTextMessage, fid_msg, NEW_JSTRING(env, msg.szMessage));
    }
    else
    {
        ZERO_STRUCT(msg);
        msg.nMsgType = (TextMsgType)env->GetIntField(lpTextMessage, fid_type);
        msg.nFromUserID = env->GetIntField(lpTextMessage, fid_fromid);
        TT_STRCPY(msg.szFromUsername, ttstr(env, (jstring)env->GetObjectField(lpTextMessage, fid_username)));
        msg.nToUserID = env->GetIntField(lpTextMessage, fid_toid);
        msg.nChannelID = env->GetIntField(lpTextMessage, fid_chanid);
        TT_STRCPY(msg.szMessage, ttstr(env, (jstring)env->GetObjectField(lpTextMessage, fid_msg)));
    }
}

void setUserAccount(JNIEnv* env, UserAccount& account, jobject lpAccount, JConvert conv)
{
    jclass cls_account = env->GetObjectClass(lpAccount);
    
    jfieldID fid_user = env->GetFieldID(cls_account, "szUsername", "Ljava/lang/String;");
    jfieldID fid_passwd = env->GetFieldID(cls_account, "szPassword", "Ljava/lang/String;");
    jfieldID fid_type = env->GetFieldID(cls_account, "uUserType", "I");
    jfieldID fid_data = env->GetFieldID(cls_account, "nUserData", "I");
    jfieldID fid_ur = env->GetFieldID(cls_account, "uUserRights", "I");
    jfieldID fid_note = env->GetFieldID(cls_account, "szNote", "Ljava/lang/String;");
    jfieldID fid_initchan = env->GetFieldID(cls_account, "szInitChannel", "Ljava/lang/String;");
    jfieldID fid_op = env->GetFieldID(cls_account, "autoOperatorChannels", "[I");
    jfieldID fid_audbps = env->GetFieldID(cls_account, "nAudioCodecBpsLimit", "I");
    jfieldID fid_abuse = env->GetFieldID(cls_account, "abusePrevent", "Ldk/bearware/AbusePrevention;");

    assert(fid_user);
    assert(fid_passwd);
    assert(fid_type);
    assert(fid_data);
    assert(fid_note);
    assert(fid_initchan);
    assert(fid_op);
    assert(fid_ur);
    assert(fid_audbps);
    assert(fid_abuse);

    if(conv == N2J)
    {
        env->SetObjectField(lpAccount, fid_user, NEW_JSTRING(env, account.szUsername));
        env->SetObjectField(lpAccount, fid_passwd, NEW_JSTRING(env, account.szPassword));
        env->SetIntField(lpAccount, fid_type, account.uUserType);
        env->SetIntField(lpAccount, fid_ur, account.uUserRights);
        env->SetIntField(lpAccount, fid_data, account.nUserData);
        env->SetObjectField(lpAccount, fid_note, NEW_JSTRING(env, account.szNote));
        env->SetObjectField(lpAccount, fid_initchan, NEW_JSTRING(env, account.szInitChannel));
        jintArray intArr = env->NewIntArray(TT_CHANNELS_OPERATOR_MAX);
        jint tmp[TT_CHANNELS_OPERATOR_MAX] = {};
        env->SetIntArrayRegion(intArr, 0, TT_CHANNELS_OPERATOR_MAX, TO_JINT_ARRAY(account.autoOperatorChannels, tmp, TT_CHANNELS_OPERATOR_MAX));
        env->SetObjectField(lpAccount, fid_op, intArr);
        env->SetIntField(lpAccount, fid_audbps, account.nAudioCodecBpsLimit);
        
        jobject ap_obj = newAbusePrevention(env, &account.abusePrevent);
        assert(ap_obj);
        setAbusePrevention(env, account.abusePrevent, ap_obj, conv);
        env->SetObjectField(lpAccount, fid_abuse, ap_obj);
    }
    else
    {
        ZERO_STRUCT(account);
        TT_STRCPY(account.szUsername, ttstr(env, (jstring)env->GetObjectField(lpAccount, fid_user)));
        TT_STRCPY(account.szPassword, ttstr(env, (jstring)env->GetObjectField(lpAccount, fid_passwd)));
        account.uUserType = env->GetIntField(lpAccount, fid_type);
        account.uUserRights = env->GetIntField(lpAccount, fid_ur);
        account.nUserData = env->GetIntField(lpAccount, fid_data);
        TT_STRCPY(account.szNote, ttstr(env, (jstring)env->GetObjectField(lpAccount, fid_note)));
        TT_STRCPY(account.szInitChannel, ttstr(env, (jstring)env->GetObjectField(lpAccount, fid_initchan)));
        jintArray intArr = (jintArray)env->GetObjectField(lpAccount, fid_op);
        jint tmp[TT_CHANNELS_OPERATOR_MAX] = {};
        env->GetIntArrayRegion(intArr, 0, TT_CHANNELS_OPERATOR_MAX, tmp);
        TO_INT32_ARRAY(tmp, account.autoOperatorChannels, TT_CHANNELS_OPERATOR_MAX);
        account.nAudioCodecBpsLimit = env->GetIntField(lpAccount, fid_audbps);
        jobject ap_obj = env->GetObjectField(lpAccount, fid_abuse);
        assert(ap_obj);
        setAbusePrevention(env, account.abusePrevent, ap_obj, conv);
    }
}

void setServerStatistics(JNIEnv* env, ServerStatistics& stats, jobject lpServerStatistics, JConvert conv)
{
    jclass cls_srvstats = env->GetObjectClass(lpServerStatistics);
    
    jfieldID fid_totaltx = env->GetFieldID(cls_srvstats, "nTotalBytesTX", "J");
    jfieldID fid_totalrx = env->GetFieldID(cls_srvstats, "nTotalBytesRX", "J");
    jfieldID fid_voicetx = env->GetFieldID(cls_srvstats, "nVoiceBytesTX", "J");
    jfieldID fid_voicerx = env->GetFieldID(cls_srvstats, "nVoiceBytesRX", "J");
    jfieldID fid_videotx = env->GetFieldID(cls_srvstats, "nVideoCaptureBytesTX", "J");
    jfieldID fid_videorx = env->GetFieldID(cls_srvstats, "nVideoCaptureBytesRX", "J");
    jfieldID fid_mftx = env->GetFieldID(cls_srvstats, "nMediaFileBytesTX", "J");
    jfieldID fid_mfrx = env->GetFieldID(cls_srvstats, "nMediaFileBytesRX", "J");
    jfieldID fid_desktx = env->GetFieldID(cls_srvstats, "nDesktopBytesTX", "J");
    jfieldID fid_deskrx = env->GetFieldID(cls_srvstats, "nDesktopBytesRX", "J");
    jfieldID fid_uptm = env->GetFieldID(cls_srvstats, "nUptimeMSec", "J");

    assert(fid_totaltx);
    assert(fid_totalrx);
    assert(fid_voicetx);
    assert(fid_voicerx);
    assert(fid_videotx);
    assert(fid_videorx);
    assert(fid_mftx);
    assert(fid_mfrx);
    assert(fid_desktx);
    assert(fid_deskrx);
    assert(fid_uptm);

    if(conv == N2J)
    {
        env->SetLongField(lpServerStatistics, fid_totaltx, stats.nTotalBytesTX);
        env->SetLongField(lpServerStatistics, fid_totalrx, stats.nTotalBytesRX);
        env->SetLongField(lpServerStatistics, fid_voicetx, stats.nVoiceBytesTX);
        env->SetLongField(lpServerStatistics, fid_voicerx, stats.nVoiceBytesRX);
        env->SetLongField(lpServerStatistics, fid_videotx, stats.nVideoCaptureBytesTX);
        env->SetLongField(lpServerStatistics, fid_videorx, stats.nVideoCaptureBytesRX);
        env->SetLongField(lpServerStatistics, fid_mftx, stats.nMediaFileBytesTX);
        env->SetLongField(lpServerStatistics, fid_mfrx, stats.nMediaFileBytesRX);
        env->SetLongField(lpServerStatistics, fid_desktx, stats.nDesktopBytesTX);
        env->SetLongField(lpServerStatistics, fid_deskrx, stats.nDesktopBytesRX);
        env->SetLongField(lpServerStatistics, fid_uptm, stats.nUptimeMSec);
    }
    else
    {
        ZERO_STRUCT(stats);
        stats.nTotalBytesTX = env->GetLongField(lpServerStatistics, fid_totaltx);
        stats.nTotalBytesRX = env->GetLongField(lpServerStatistics, fid_totalrx);
        stats.nVoiceBytesTX = env->GetLongField(lpServerStatistics, fid_voicetx);
        stats.nVoiceBytesRX = env->GetLongField(lpServerStatistics, fid_voicerx);
        stats.nVideoCaptureBytesTX = env->GetLongField(lpServerStatistics, fid_videotx);
        stats.nVideoCaptureBytesRX = env->GetLongField(lpServerStatistics, fid_videorx);
        stats.nMediaFileBytesTX = env->GetLongField(lpServerStatistics, fid_mftx);
        stats.nMediaFileBytesRX = env->GetLongField(lpServerStatistics, fid_mfrx);
        stats.nDesktopBytesTX = env->GetLongField(lpServerStatistics, fid_desktx);
        stats.nDesktopBytesRX = env->GetLongField(lpServerStatistics, fid_deskrx);
        stats.nUptimeMSec = env->GetLongField(lpServerStatistics, fid_uptm);
    }
}

void setRemoteFile(JNIEnv* env, RemoteFile& fileinfo, jobject lpRemoteFile, JConvert conv)
{
    jclass cls_finfo = env->GetObjectClass(lpRemoteFile);
    
    jfieldID fid_id = env->GetFieldID(cls_finfo, "nFileID", "I");
    jfieldID fid_cid = env->GetFieldID(cls_finfo, "nChannelID", "I");
    jfieldID fid_name = env->GetFieldID(cls_finfo, "szFileName", "Ljava/lang/String;");
    jfieldID fid_size = env->GetFieldID(cls_finfo, "nFileSize", "J");
    jfieldID fid_user = env->GetFieldID(cls_finfo, "szUsername", "Ljava/lang/String;");

    assert(fid_id);
    assert(fid_cid);
    assert(fid_name);
    assert(fid_size);
    assert(fid_user);

    if(conv == N2J) {
        env->SetIntField(lpRemoteFile, fid_id, fileinfo.nFileID);
        env->SetIntField(lpRemoteFile, fid_cid, fileinfo.nChannelID);
        env->SetObjectField(lpRemoteFile, fid_name, NEW_JSTRING(env, fileinfo.szFileName));
        env->SetLongField(lpRemoteFile, fid_size, fileinfo.nFileSize);
        env->SetObjectField(lpRemoteFile, fid_user, NEW_JSTRING(env, fileinfo.szUsername));
    }
    else {
        ZERO_STRUCT(fileinfo);
        fileinfo.nFileID = env->GetIntField(lpRemoteFile, fid_id);
        fileinfo.nChannelID = env->GetIntField(lpRemoteFile, fid_cid);
        TT_STRCPY(fileinfo.szFileName, ttstr(env, (jstring)env->GetObjectField(lpRemoteFile, fid_name)));
        fileinfo.nFileSize = env->GetLongField(lpRemoteFile, fid_size);
        TT_STRCPY(fileinfo.szUsername, ttstr(env, (jstring)env->GetObjectField(lpRemoteFile, fid_user)));
    }
}

void setUserStatistics(JNIEnv* env, UserStatistics& stats, jobject lpUserStatistics)
{
    jclass cls_stats = env->GetObjectClass(lpUserStatistics);
    
    jfieldID fid_voirx = env->GetFieldID(cls_stats, "nVoicePacketsRecv", "J");
    jfieldID fid_voilost = env->GetFieldID(cls_stats, "nVoicePacketsLost", "J");
    jfieldID fid_vidrx = env->GetFieldID(cls_stats, "nVideoCapturePacketsRecv", "J");
    jfieldID fid_vidftx = env->GetFieldID(cls_stats, "nVideoCaptureFramesRecv", "J");
    jfieldID fid_vidflost = env->GetFieldID(cls_stats, "nVideoCaptureFramesLost", "J");
    jfieldID fid_vidfdropped = env->GetFieldID(cls_stats, "nVideoCaptureFramesDropped", "J");

    jfieldID fid_mfaudrx = env->GetFieldID(cls_stats, "nMediaFileAudioPacketsRecv", "J");
    jfieldID fid_mfaudlost = env->GetFieldID(cls_stats, "nMediaFileAudioPacketsLost", "J");
    jfieldID fid_mfvidrx = env->GetFieldID(cls_stats, "nMediaFileVideoPacketsRecv", "J");
    jfieldID fid_mfvidftx = env->GetFieldID(cls_stats, "nMediaFileVideoFramesRecv", "J");
    jfieldID fid_mfvidflost = env->GetFieldID(cls_stats, "nMediaFileVideoFramesLost", "J");
    jfieldID fid_mfvidfdropped = env->GetFieldID(cls_stats, "nMediaFileVideoFramesDropped", "J");

    assert(fid_voirx);
    assert(fid_voilost);
    assert(fid_vidrx);
    assert(fid_vidftx);
    assert(fid_vidflost);
    assert(fid_vidfdropped);
    assert(fid_mfaudrx);
    assert(fid_mfaudlost);
    assert(fid_mfvidrx);
    assert(fid_mfvidftx);
    assert(fid_mfvidflost);
    assert(fid_mfvidfdropped);

    env->SetLongField(lpUserStatistics, fid_voirx, stats.nVoicePacketsRecv);
    env->SetLongField(lpUserStatistics, fid_voilost, stats.nVoicePacketsLost);
    env->SetLongField(lpUserStatistics, fid_vidrx, stats.nVideoCapturePacketsRecv);
    env->SetLongField(lpUserStatistics, fid_vidftx, stats.nVideoCaptureFramesRecv);
    env->SetLongField(lpUserStatistics, fid_vidflost, stats.nVideoCaptureFramesLost);
    env->SetLongField(lpUserStatistics, fid_vidfdropped, stats.nVideoCaptureFramesDropped);
    env->SetLongField(lpUserStatistics, fid_mfaudrx, stats.nMediaFileAudioPacketsRecv);
    env->SetLongField(lpUserStatistics, fid_mfaudlost, stats.nMediaFileAudioPacketsLost);
    env->SetLongField(lpUserStatistics, fid_mfvidrx, stats.nMediaFileVideoPacketsRecv);
    env->SetLongField(lpUserStatistics, fid_mfvidftx, stats.nMediaFileVideoFramesRecv);
    env->SetLongField(lpUserStatistics, fid_mfvidflost, stats.nMediaFileVideoFramesLost);
    env->SetLongField(lpUserStatistics, fid_mfvidfdropped, stats.nMediaFileVideoFramesDropped);
}

void setFileTransfer(JNIEnv* env, FileTransfer& filetx, jobject lpFileTransfer)
{
    jclass cls_ftx = env->GetObjectClass(lpFileTransfer);

    jfieldID fid_status = env->GetFieldID(cls_ftx, "nStatus", "I");
    jfieldID fid_txid = env->GetFieldID(cls_ftx, "nTransferID", "I");
    jfieldID fid_chanid = env->GetFieldID(cls_ftx, "nChannelID", "I");
    jfieldID fid_filepath = env->GetFieldID(cls_ftx, "szLocalFilePath", "Ljava/lang/String;");
    jfieldID fid_rempath = env->GetFieldID(cls_ftx, "szRemoteFileName", "Ljava/lang/String;");
    jfieldID fid_size = env->GetFieldID(cls_ftx, "nFileSize", "J");
    jfieldID fid_txed = env->GetFieldID(cls_ftx, "nTransferred", "J");
    jfieldID fid_inbound = env->GetFieldID(cls_ftx, "bInbound", "Z");

    assert(fid_status);
    assert(fid_txid);
    assert(fid_chanid);
    assert(fid_filepath);
    assert(fid_rempath);
    assert(fid_size);
    assert(fid_txed);
    assert(fid_inbound);

    env->SetIntField(lpFileTransfer, fid_status, filetx.nStatus);
    env->SetIntField(lpFileTransfer, fid_txid, filetx.nTransferID);
    env->SetIntField(lpFileTransfer, fid_chanid, filetx.nChannelID);
    env->SetObjectField(lpFileTransfer, fid_filepath, NEW_JSTRING(env, filetx.szLocalFilePath));
    env->SetObjectField(lpFileTransfer, fid_rempath, NEW_JSTRING(env, filetx.szRemoteFileName));
    env->SetLongField(lpFileTransfer, fid_size, filetx.nFileSize);
    env->SetLongField(lpFileTransfer, fid_txed, filetx.nTransferred);
    env->SetBooleanField(lpFileTransfer, fid_inbound, filetx.bInbound);
}

void setBannedUser(JNIEnv* env, BannedUser& banned, jobject lpBannedUser, JConvert conv)
{
   jclass cls_ban = env->GetObjectClass(lpBannedUser);

   jfieldID fid_ipaddr = env->GetFieldID(cls_ban, "szIPAddress", "Ljava/lang/String;");
   jfieldID fid_chan = env->GetFieldID(cls_ban, "szChannelPath", "Ljava/lang/String;");
   jfieldID fid_time = env->GetFieldID(cls_ban, "szBanTime", "Ljava/lang/String;");
   jfieldID fid_nick = env->GetFieldID(cls_ban, "szNickname", "Ljava/lang/String;");
   jfieldID fid_username = env->GetFieldID(cls_ban, "szUsername", "Ljava/lang/String;");
   jfieldID fid_bantype = env->GetFieldID(cls_ban, "uBanTypes", "I");

   assert(fid_ipaddr);
   assert(fid_chan);
   assert(fid_time);
   assert(fid_nick);
   assert(fid_username);
   assert(fid_bantype);

   if (conv == N2J)
   {
       env->SetObjectField(lpBannedUser, fid_ipaddr, NEW_JSTRING(env, banned.szIPAddress));
       env->SetObjectField(lpBannedUser, fid_chan, NEW_JSTRING(env, banned.szChannelPath));
       env->SetObjectField(lpBannedUser, fid_time, NEW_JSTRING(env, banned.szBanTime));
       env->SetObjectField(lpBannedUser, fid_nick, NEW_JSTRING(env, banned.szNickname));
       env->SetObjectField(lpBannedUser, fid_username, NEW_JSTRING(env, banned.szUsername));
       env->SetIntField(lpBannedUser, fid_bantype, banned.uBanTypes);
   }
   else
   {
       ZERO_STRUCT(banned);
       TT_STRCPY(banned.szIPAddress, ttstr(env, (jstring)env->GetObjectField(lpBannedUser, fid_ipaddr)));
       TT_STRCPY(banned.szChannelPath, ttstr(env, (jstring)env->GetObjectField(lpBannedUser, fid_chan)));
       TT_STRCPY(banned.szBanTime, ttstr(env, (jstring)env->GetObjectField(lpBannedUser, fid_time)));
       TT_STRCPY(banned.szNickname, ttstr(env, (jstring)env->GetObjectField(lpBannedUser, fid_nick)));
       TT_STRCPY(banned.szUsername, ttstr(env, (jstring)env->GetObjectField(lpBannedUser, fid_username)));
       banned.uBanTypes = env->GetIntField(lpBannedUser, fid_bantype);
   }
}

void setClientErrorMsg(JNIEnv* env, ClientErrorMsg& cemsg, jobject lpClientErrorMsg, JConvert conv)
{
   jclass cls_msg = env->GetObjectClass(lpClientErrorMsg);

   jfieldID fid_err = env->GetFieldID(cls_msg, "nErrorNo", "I");
   jfieldID fid_msg = env->GetFieldID(cls_msg, "szErrorMsg", "Ljava/lang/String;");

   assert(fid_err);
   assert(fid_msg);

   if(conv == N2J)
   {
       env->SetIntField(lpClientErrorMsg, fid_err, cemsg.nErrorNo);
       env->SetObjectField(lpClientErrorMsg, fid_msg, NEW_JSTRING(env, cemsg.szErrorMsg));
   }
   else
   {
       ZERO_STRUCT(cemsg);
       cemsg.nErrorNo = env->GetIntField(lpClientErrorMsg, fid_err);
       TT_STRCPY(cemsg.szErrorMsg, ttstr(env, (jstring)env->GetObjectField(lpClientErrorMsg, fid_msg)));
   }
}

void setDesktopInput(JNIEnv* env, DesktopInput& input, jobject lpDesktopInput, JConvert conv)
{
   jclass cls = env->GetObjectClass(lpDesktopInput);

   jfieldID fid_x = env->GetFieldID(cls, "uMousePosX", "I");
   jfieldID fid_y = env->GetFieldID(cls, "uMousePosY", "I");
   jfieldID fid_keycode = env->GetFieldID(cls, "uKeyCode", "I");
   jfieldID fid_keystate = env->GetFieldID(cls, "uKeyState", "I");

   assert(fid_x);
   assert(fid_y);
   assert(fid_keycode);
   assert(fid_keystate);

   if(conv == N2J)
   {
       env->SetIntField(lpDesktopInput, fid_x, input.uMousePosX);
       env->SetIntField(lpDesktopInput, fid_y, input.uMousePosY);
       env->SetIntField(lpDesktopInput, fid_keycode, input.uKeyCode);
       env->SetIntField(lpDesktopInput, fid_keystate, input.uKeyState);
   }
   else
   {
       ZERO_STRUCT(input);
       input.uMousePosX = (UINT16)env->GetIntField(lpDesktopInput, fid_x);
       input.uMousePosY = (UINT16)env->GetIntField(lpDesktopInput, fid_y);
       input.uKeyCode = env->GetIntField(lpDesktopInput, fid_keycode);
       input.uKeyState = env->GetIntField(lpDesktopInput, fid_keystate);
   }
}

void setDesktopWindow(JNIEnv* env, DesktopWindow& deskwnd, jobject lpDesktopWindow, JConvert conv)
{
   jclass cls = env->GetObjectClass(lpDesktopWindow);

   jfieldID fid_w = env->GetFieldID(cls, "nWidth", "I");
   jfieldID fid_h = env->GetFieldID(cls, "nHeight", "I");
   jfieldID fid_bmpfmt = env->GetFieldID(cls, "bmpFormat", "I");
   jfieldID fid_bpl = env->GetFieldID(cls, "nBytesPerLine", "I");
   jfieldID fid_sesid = env->GetFieldID(cls, "nSessionID", "I");
   jfieldID fid_frmbuf = env->GetFieldID(cls, "frameBuffer", "[B");

   assert(fid_w);
   assert(fid_h);
   assert(fid_bmpfmt);
   assert(fid_bpl);
   assert(fid_sesid);
   assert(fid_frmbuf);

   if(conv == N2J)
   {

       jbyteArray buf = env->NewByteArray(deskwnd.nFrameBufferSize);
       jbyte* bufptr = env->GetByteArrayElements(buf, 0);
       if(!bufptr)
           return;

       memcpy(bufptr, deskwnd.frameBuffer, deskwnd.nFrameBufferSize);
       env->ReleaseByteArrayElements(buf, bufptr, 0);

       env->SetIntField(lpDesktopWindow, fid_w, deskwnd.nWidth);
       env->SetIntField(lpDesktopWindow, fid_h, deskwnd.nHeight);
       env->SetIntField(lpDesktopWindow, fid_bmpfmt, deskwnd.bmpFormat);
       env->SetIntField(lpDesktopWindow, fid_bpl, deskwnd.nBytesPerLine);
       env->SetIntField(lpDesktopWindow, fid_sesid, deskwnd.nSessionID);
       env->SetObjectField(lpDesktopWindow, fid_frmbuf, buf);
   }
   else
   {
       ZERO_STRUCT(deskwnd);
       deskwnd.nWidth = env->GetIntField(lpDesktopWindow, fid_w);
       deskwnd.nHeight = env->GetIntField(lpDesktopWindow, fid_h);
       deskwnd.bmpFormat = (BitmapFormat)env->GetIntField(lpDesktopWindow, fid_bmpfmt);
       deskwnd.nBytesPerLine = env->GetIntField(lpDesktopWindow, fid_bpl);
       deskwnd.nSessionID = env->GetIntField(lpDesktopWindow, fid_sesid);
   }
}

void setVideoFrame(JNIEnv* env, VideoFrame& vidframe, jobject lpVideoFrame)
{
   jclass cls = env->GetObjectClass(lpVideoFrame);

   jfieldID fid_w = env->GetFieldID(cls, "nWidth", "I");
   jfieldID fid_h = env->GetFieldID(cls, "nHeight", "I");
   jfieldID fid_sid = env->GetFieldID(cls, "nStreamID", "I");
   jfieldID fid_kfrm = env->GetFieldID(cls, "bKeyFrame", "Z");
   jfieldID fid_frmbuf = env->GetFieldID(cls, "frameBuffer", "[B");

   assert(fid_w);
   assert(fid_h);
   assert(fid_sid);
   assert(fid_kfrm);
   assert(fid_frmbuf);

   jbyteArray buf = env->NewByteArray(vidframe.nFrameBufferSize);
   jbyte* bufptr = env->GetByteArrayElements(buf, 0);
   if(!bufptr)
       return;

   memcpy(bufptr, vidframe.frameBuffer, vidframe.nFrameBufferSize);
   env->ReleaseByteArrayElements(buf, bufptr, 0);

   env->SetIntField(lpVideoFrame, fid_w, vidframe.nWidth);
   env->SetIntField(lpVideoFrame, fid_h, vidframe.nHeight);
   env->SetIntField(lpVideoFrame, fid_sid, vidframe.nStreamID);
   env->SetBooleanField(lpVideoFrame, fid_kfrm, vidframe.bKeyFrame);
   env->SetObjectField(lpVideoFrame, fid_frmbuf, buf);
}

jbyteArray setAudioBlock(JNIEnv* env, AudioBlock& audblock, jobject lpAudioBlock, JConvert conv)
{
    jclass cls = env->GetObjectClass(lpAudioBlock);

    jfieldID fid_sid = env->GetFieldID(cls, "nStreamID", "I");
    jfieldID fid_sr = env->GetFieldID(cls, "nSampleRate", "I");
    jfieldID fid_ch = env->GetFieldID(cls, "nChannels", "I");
    jfieldID fid_audbuf = env->GetFieldID(cls, "lpRawAudio", "[B");
    jfieldID fid_sn = env->GetFieldID(cls, "nSamples", "I");
    jfieldID fid_si = env->GetFieldID(cls, "uSampleIndex", "I");

    assert(fid_sid);
    assert(fid_sr);
    assert(fid_ch);
    assert(fid_audbuf);
    assert(fid_sn);
    assert(fid_si);

    if (conv == N2J)
    {
        int size = audblock.nSamples * sizeof(short) * audblock.nChannels;
        jbyteArray buf = env->NewByteArray(size);
        if (size > 0)
        {
            jbyte* bufptr = env->GetByteArrayElements(buf, 0);
            if(!bufptr)
                return nullptr;
            memcpy(bufptr, audblock.lpRawAudio, size);
            env->ReleaseByteArrayElements(buf, bufptr, 0);
        }
        
        env->SetIntField(lpAudioBlock, fid_sid, audblock.nStreamID);
        env->SetIntField(lpAudioBlock, fid_sr, audblock.nSampleRate);
        env->SetIntField(lpAudioBlock, fid_ch, audblock.nChannels);
        env->SetObjectField(lpAudioBlock, fid_audbuf, buf);
        env->SetIntField(lpAudioBlock, fid_sn, audblock.nSamples);
        env->SetIntField(lpAudioBlock, fid_si, audblock.uSampleIndex);
    }
    else
    {
        audblock.nStreamID = env->GetIntField(lpAudioBlock, fid_sid);
        audblock.nSampleRate = env->GetIntField(lpAudioBlock, fid_sr);
        audblock.nChannels = env->GetIntField(lpAudioBlock, fid_ch);
        audblock.nSamples = env->GetIntField(lpAudioBlock, fid_sn);
        audblock.uSampleIndex = env->GetIntField(lpAudioBlock, fid_si);
        jbyteArray byteArr = jbyteArray(env->GetObjectField(lpAudioBlock, fid_audbuf));
        if (byteArr)
            audblock.lpRawAudio = env->GetByteArrayElements(byteArr, nullptr);
        return byteArr;
    }
    return nullptr;
}

void setMediaFileInfo(JNIEnv* env, MediaFileInfo& mfi, jobject lpMediaFileInfo, JConvert conv)
{
   jclass cls = env->GetObjectClass(lpMediaFileInfo);

   jfieldID fid_status = env->GetFieldID(cls, "nStatus", "I");
   jfieldID fid_fname = env->GetFieldID(cls, "szFileName", "Ljava/lang/String;");
   jfieldID fid_audfmt = env->GetFieldID(cls, "audioFmt", "Ldk/bearware/AudioFormat;");
   jfieldID fid_vidfmt = env->GetFieldID(cls, "videoFmt", "Ldk/bearware/VideoFormat;");
   jfieldID fid_dur = env->GetFieldID(cls, "uDurationMSec", "I");
   jfieldID fid_elap = env->GetFieldID(cls, "uElapsedMSec", "I");

   assert(fid_status);
   assert(fid_fname);
   assert(fid_audfmt);
   assert(fid_vidfmt);
   assert(fid_dur);
   assert(fid_elap);

   jclass cls_audfmt = env->FindClass("dk/bearware/AudioFormat");
   jclass cls_vidfmt = env->FindClass("dk/bearware/VideoFormat");

   if (conv == N2J) {
       env->SetIntField(lpMediaFileInfo, fid_status, mfi.nStatus);
       env->SetObjectField(lpMediaFileInfo, fid_fname, NEW_JSTRING(env, mfi.szFileName));
   
       jobject audfmt_obj = newObject(env, cls_audfmt);
       jobject vidfmt_obj = newObject(env, cls_vidfmt);
       setAudioFormat(env, mfi.audioFmt, audfmt_obj, conv);
       setVideoFormat(env, mfi.videoFmt, vidfmt_obj, conv);
       env->SetObjectField(lpMediaFileInfo, fid_audfmt, audfmt_obj);
       env->SetObjectField(lpMediaFileInfo, fid_vidfmt, vidfmt_obj);
       env->SetIntField(lpMediaFileInfo, fid_dur, mfi.uDurationMSec);
       env->SetIntField(lpMediaFileInfo, fid_elap, mfi.uElapsedMSec);
   }
   else {
       mfi.nStatus = MediaFileStatus(env->GetIntField(lpMediaFileInfo, fid_status));
       TT_STRCPY(mfi.szFileName, ttstr(env, (jstring)env->GetObjectField(lpMediaFileInfo, fid_fname)));
       jobject audfmt_obj = env->GetObjectField(lpMediaFileInfo, fid_audfmt);
       jobject vidfmt_obj = env->GetObjectField(lpMediaFileInfo, fid_vidfmt);
       setAudioFormat(env, mfi.audioFmt, audfmt_obj, conv);
       setVideoFormat(env, mfi.videoFmt, vidfmt_obj, conv);
       mfi.uDurationMSec = env->GetIntField(lpMediaFileInfo, fid_dur);
       mfi.uElapsedMSec = env->GetIntField(lpMediaFileInfo, fid_elap);
   }
}

void setAudioFormat(JNIEnv* env, AudioFormat& fmt, jobject lpAudioFormat, JConvert conv)
{
    jclass cls = env->GetObjectClass(lpAudioFormat);
    jfieldID fid_audfmt = env->GetFieldID(cls, "nAudioFmt", "I");
    jfieldID fid_sr = env->GetFieldID(cls, "nSampleRate", "I");
    jfieldID fid_ch = env->GetFieldID(cls, "nChannels", "I");
    
    assert(fid_audfmt);
    assert(fid_sr);
    assert(fid_ch);

    if (conv == N2J) {
        env->SetIntField(lpAudioFormat, fid_audfmt, fmt.nAudioFmt);
        env->SetIntField(lpAudioFormat, fid_sr, fmt.nSampleRate);
        env->SetIntField(lpAudioFormat, fid_ch, fmt.nChannels);
    }
    else {
        fmt.nAudioFmt = AudioFileFormat(env->GetIntField(lpAudioFormat, fid_audfmt));
        fmt.nSampleRate = env->GetIntField(lpAudioFormat, fid_sr);
        fmt.nChannels = env->GetIntField(lpAudioFormat, fid_ch);
    }
}

void setVideoFormat(JNIEnv* env, VideoFormat& fmt, jobject lpVideoFormat, JConvert conv)
{
    jclass cls = env->GetObjectClass(lpVideoFormat);
    jfieldID fid_w = env->GetFieldID(cls, "nWidth", "I");
    jfieldID fid_h = env->GetFieldID(cls, "nHeight", "I");
    jfieldID fid_fpsN = env->GetFieldID(cls, "nFPS_Numerator", "I");
    jfieldID fid_fpsD = env->GetFieldID(cls, "nFPS_Denominator", "I");
    jfieldID fid_fcc = env->GetFieldID(cls, "picFourCC", "I");
    
    assert(fid_w);
    assert(fid_h);
    assert(fid_fpsN);
    assert(fid_fpsD);
    assert(fid_fcc);

    if(conv == N2J)
    {
        env->SetIntField(lpVideoFormat, fid_w, fmt.nWidth);
        env->SetIntField(lpVideoFormat, fid_h, fmt.nHeight);
        env->SetIntField(lpVideoFormat, fid_fpsN, fmt.nFPS_Numerator);
        env->SetIntField(lpVideoFormat, fid_fpsD, fmt.nFPS_Denominator);
        env->SetIntField(lpVideoFormat, fid_fcc, fmt.picFourCC);
    }
    else
    {
        fmt.nWidth = env->GetIntField(lpVideoFormat, fid_w);
        fmt.nHeight = env->GetIntField(lpVideoFormat, fid_h);
        fmt.nFPS_Numerator = env->GetIntField(lpVideoFormat, fid_fpsN);
        fmt.nFPS_Denominator = env->GetIntField(lpVideoFormat, fid_fpsD);
        fmt.picFourCC = (FourCC)env->GetIntField(lpVideoFormat, fid_fcc);
    }
}

void setVideoCodec(JNIEnv* env, VideoCodec& codec, jobject lpVideoCodec, JConvert conv)
{
    jclass cls = env->GetObjectClass(lpVideoCodec);
    jfieldID fid_codec = env->GetFieldID(cls, "nCodec", "I");
    jfieldID fid_webm = env->GetFieldID(cls, "webm_vp8", "Ldk/bearware/WebMVP8Codec;");
    jclass cls_webm = env->FindClass("dk/bearware/WebMVP8Codec");

    assert(fid_codec);
    assert(fid_webm);


    if(conv == N2J)
    {
        env->SetIntField(lpVideoCodec, fid_codec, codec.nCodec);
        switch(codec.nCodec)
        {
        case WEBM_VP8_CODEC : {
            jobject webm_obj = newObject(env, cls_webm);
            assert(webm_obj);
            setWebMVP8Codec(env, codec.webm_vp8, webm_obj, conv);
            env->SetObjectField(lpVideoCodec, fid_webm, webm_obj);
            break;
        }
        case NO_CODEC :
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
            // make compiler shut up
            break;
        }
    }
    else
    {
        codec.nCodec = (Codec)env->GetIntField(lpVideoCodec, fid_codec);
        switch(codec.nCodec)
        {
        case WEBM_VP8_CODEC :
            setWebMVP8Codec(env, codec.webm_vp8, env->GetObjectField(lpVideoCodec, fid_webm), conv);
            break;
        case NO_CODEC :
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
            // make compiler shut up
            break;
        }
    }
}

void setWebMVP8Codec(JNIEnv* env, WebMVP8Codec& webm_vp8, jobject lpWebMVP8Codec, JConvert conv)
{
    jclass cls = env->GetObjectClass(lpWebMVP8Codec);
    jfieldID fid_br = env->GetFieldID(cls, "nRcTargetBitrate", "I");
    assert(fid_br);

    if(conv == N2J)
        env->SetIntField(lpWebMVP8Codec, fid_br, webm_vp8.nRcTargetBitrate);
    else
        webm_vp8.nRcTargetBitrate = env->GetIntField(lpWebMVP8Codec, fid_br);
}

void setAbusePrevention(JNIEnv* env, AbusePrevention& abuse, jobject lpAbusePrevention, JConvert conv) {
    jclass cls = env->GetObjectClass(lpAbusePrevention);
    jfieldID fid_cmds = env->GetFieldID(cls, "nCommandsLimit", "I");
    jfieldID fid_msec = env->GetFieldID(cls, "nCommandsIntervalMSec", "I");
    assert(fid_cmds);
    assert(fid_msec);

    if(conv == N2J) {
        env->SetIntField(lpAbusePrevention, fid_cmds, abuse.nCommandsLimit);
        env->SetIntField(lpAbusePrevention, fid_msec, abuse.nCommandsIntervalMSec);
    }
    else {
        abuse.nCommandsLimit = env->GetIntField(lpAbusePrevention, fid_cmds);
        abuse.nCommandsIntervalMSec = env->GetIntField(lpAbusePrevention, fid_msec);
    }
}

void setMediaFilePlayback(JNIEnv* env, MediaFilePlayback& playback, jobject lpMediaPlayback, JConvert conv) {

    jclass cls = env->GetObjectClass(lpMediaPlayback);
    jfieldID fid_off = env->GetFieldID(cls, "uOffsetMSec", "I");
    jfieldID fid_pause = env->GetFieldID(cls, "bPaused", "Z");
    jfieldID fid_pre = env->GetFieldID(cls, "audioPreprocessor", "Ldk/bearware/AudioPreprocessor;");

    assert(fid_off);
    assert(fid_pause);
    assert(fid_pre);

    jobject pre = env->GetObjectField(lpMediaPlayback, fid_pre);
    
    if (conv == N2J) {
        env->SetIntField(lpMediaPlayback, fid_off, playback.uOffsetMSec);
        env->SetBooleanField(lpMediaPlayback, fid_pause, playback.bPaused);
        setAudioPreprocessor(env, playback.audioPreprocessor, pre, conv);
    }
    else
    {
        playback.uOffsetMSec = env->GetIntField(lpMediaPlayback, fid_off);
        playback.bPaused = env->GetBooleanField(lpMediaPlayback, fid_pause);
        setAudioPreprocessor(env, playback.audioPreprocessor, pre, conv);
    }
}

void setAudioInputProgress(JNIEnv* env, AudioInputProgress& aip, jobject lpAudioInputProgress, JConvert conv)
{
    jclass cls = env->GetObjectClass(lpAudioInputProgress);
    jfieldID fid_sid = env->GetFieldID(cls, "nStreamID", "I");
    jfieldID fid_queue = env->GetFieldID(cls, "uQueueMSec", "I");
    jfieldID fid_elapsed = env->GetFieldID(cls, "uElapsedMSec", "I");

    assert(fid_sid);
    assert(fid_queue);
    assert(fid_elapsed);

    if (conv == N2J) {
        env->SetIntField(lpAudioInputProgress, fid_sid, aip.nStreamID);
        env->SetIntField(lpAudioInputProgress, fid_queue, aip.uQueueMSec);
        env->SetIntField(lpAudioInputProgress, fid_elapsed, aip.uElapsedMSec);
    }
    else
    {
        aip.nStreamID = env->GetIntField(lpAudioInputProgress, fid_sid);
        aip.uQueueMSec = env->GetIntField(lpAudioInputProgress, fid_queue);
        aip.uElapsedMSec = env->GetIntField(lpAudioInputProgress, fid_elapsed);
    }
}

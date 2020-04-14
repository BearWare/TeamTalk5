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


#include "TeamTalk.h"

#include <teamtalk/Common.h>
#include <teamtalk/client/ClientUser.h>
#include <teamtalk/client/ClientNode.h>
#include <codec/MediaUtil.h>

#if defined(ENABLE_TEAMTALKPRO)
#include <teamtalk/server/ServerUser.h>
#include <teamtalk/server/ServerChannel.h>
#endif

#define ZERO_STRUCT(x) ACE_OS::memset(&x, 0, sizeof(x))

bool TranslateDesktopInput(TTKeyTranslate nTranslate,
                           const DesktopInput& input,
                           DesktopInput& output);

bool Convert(const teamtalk::AudioCodec& codec, AudioCodec& result);
bool Convert(const AudioCodec& codec, teamtalk::AudioCodec& result);

void Convert(const teamtalk::AudioConfig& audcfg, AudioConfig& result);
void Convert(const AudioConfig& audcfg, teamtalk::AudioConfig& result);

void Convert(const SoundDeviceEffects& effects, teamtalk::SoundDeviceEffects& result);
void Convert(const teamtalk::SoundDeviceEffects& effects, SoundDeviceEffects& result);

void Convert(const AudioPreprocessor& audpreprocess, teamtalk::AudioPreprocessor& result);
void Convert(const teamtalk::AudioPreprocessor& audpreprocess, AudioPreprocessor& result);
void Convert(const TTAudioPreprocessor& ttpreprocess, teamtalk::TTAudioPreprocessor& result);
void Convert(const teamtalk::TTAudioPreprocessor& ttpreprocess, TTAudioPreprocessor& result);
void Convert(const SpeexDSP& spxdsp, teamtalk::SpeexDSP& result);
void Convert(const teamtalk::SpeexDSP& dspdsp, SpeexDSP& result);

bool Convert(const teamtalk::ChannelProp& chanprop, Channel& channel);
bool Convert(const Channel& channel, teamtalk::ChannelProp& chanprop);

void Convert(const teamtalk::ClientUser& clientuser, User& user);
#if defined(ENABLE_TEAMTALKPRO)
void Convert(const teamtalk::ServerUser& clientuser, User& user);
#endif

void Convert(const VideoCodec& vidcodec, teamtalk::VideoCodec& result);

void Convert(const teamtalk::UserAccount& useraccount,UserAccount& result);
void Convert(const UserAccount& useraccount, teamtalk::UserAccount& result);

void Convert(const teamtalk::ServerProperties& srvprop, ServerProperties& result);
void Convert(const teamtalk::ServerInfo& srvprop, ServerProperties& result);
#if defined(ENABLE_TEAMTALKPRO)
void Convert(const teamtalk::ServerSettings& srvprop, ServerProperties& result);
#endif
void Convert(const ServerProperties& srvprop, teamtalk::ServerProperties& result);
void Convert(const ServerProperties& srvprop, teamtalk::ServerInfo& result);
#if defined(ENABLE_TEAMTALKPRO)
void Convert(const ServerProperties& srvprop, teamtalk::ServerSettings& result);
#endif

ClientErrorMsg& Convert(const teamtalk::ErrorMsg& cmderr, ClientErrorMsg& result);
teamtalk::ErrorMsg& Convert(const ClientErrorMsg& cmderr, teamtalk::ErrorMsg& result);

void Convert(const teamtalk::BannedUser& banuser, BannedUser& result);
void Convert(const BannedUser& banuser, teamtalk::BannedUser& result);

void Convert(const teamtalk::FileTransfer& transfer, FileTransfer& result);

void Convert(const teamtalk::RemoteFile& remotefile, RemoteFile& result);
void Convert(const RemoteFile& remotefile, const TTCHAR* szPath, teamtalk::RemoteFile& result);
void Convert(const teamtalk::ServerStats& stats, ServerStatistics& result);

void Convert(const teamtalk::TextMessage& txtmsg, TextMessage& result);
void Convert(const TextMessage& txtmsg, teamtalk::TextMessage& result);

void Convert(const MediaFileProp& mediaprop, MediaFileInfo& result);
void Convert(teamtalk::MediaFileStatus status, const teamtalk::VoiceLogFile& vlog,
             MediaFileInfo& result);

void Convert(const AudioInputStatus& ais, AudioInputProgress& result);

void Convert(const media::VideoFrame& imgframe, VideoFrame& result);
void Convert(const VideoFormat& vidfmt, media::VideoFormat& result);

void Convert(const teamtalk::ClientUserStats& stats, UserStatistics& result);

void Convert(const teamtalk::ClientStats& stats, ClientStatistics& result);

void Convert(const ClientKeepAlive& ka, teamtalk::ClientKeepAlive& result);
void Convert(const teamtalk::ClientKeepAlive& ka, ClientKeepAlive& result);

void Convert(const teamtalk::DesktopInput& input, DesktopInput& result);
void Convert(const DesktopInput& input, teamtalk::DesktopInput& result);

void Convert(const std::set<int>& intset, int* int_array, int max_elements);
void Convert(const int* int_array, int max_elements, std::set<int>& intset);

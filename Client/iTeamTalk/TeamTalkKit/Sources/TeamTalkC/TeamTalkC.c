#include "TeamTalkC.h"
#include <string.h>

enum {
    TTKitDefaultMSecPerPacket = 40,
    TTKitDefaultOpusSampleRate = 48000,
    TTKitDefaultOpusChannels = 1,
    TTKitDefaultOpusComplexity = 10,
    TTKitDefaultOpusBitrate = 32000,
    TTKitDefaultSpeexBandmode = 1,
    TTKitDefaultSpeexQuality = 4,
    TTKitDefaultSpeexVBRBandmode = 1,
    TTKitDefaultSpeexVBRQuality = 4,
    TTKitDefaultSpeexVBRMaxBitrate = 0
};

AudioCodec TTKitMakeAudioCodec(Codec codec) {
    AudioCodec audioCodec = {0};

    switch (codec) {
    case OPUS_CODEC: {
        OpusCodec opusCodec = TTKitMakeOpusCodec();
        TTKitSetOpusCodec(&audioCodec, &opusCodec);
        break;
    }
    case SPEEX_CODEC: {
        SpeexCodec speexCodec = TTKitMakeSpeexCodec();
        TTKitSetSpeexCodec(&audioCodec, &speexCodec);
        break;
    }
    case SPEEX_VBR_CODEC: {
        SpeexVBRCodec speexVBRCodec = TTKitMakeSpeexVBRCodec();
        TTKitSetSpeexVBRCodec(&audioCodec, &speexVBRCodec);
        break;
    }
    case NO_CODEC:
    default:
        audioCodec.nCodec = NO_CODEC;
        break;
    }

    return audioCodec;
}

OpusCodec TTKitMakeOpusCodec(void) {
    OpusCodec opusCodec = {
        TTKitDefaultOpusSampleRate,
        TTKitDefaultOpusChannels,
        OPUS_APPLICATION_VOIP,
        TTKitDefaultOpusComplexity,
        TRUE,
        FALSE,
        TTKitDefaultOpusBitrate,
        TRUE,
        FALSE,
        TTKitDefaultMSecPerPacket,
        0
    };
    return opusCodec;
}

SpeexCodec TTKitMakeSpeexCodec(void) {
    SpeexCodec speexCodec = {
        TTKitDefaultSpeexBandmode,
        TTKitDefaultSpeexQuality,
        TTKitDefaultMSecPerPacket,
        FALSE
    };
    return speexCodec;
}

SpeexVBRCodec TTKitMakeSpeexVBRCodec(void) {
    SpeexVBRCodec speexVBRCodec = {
        TTKitDefaultSpeexVBRBandmode,
        TTKitDefaultSpeexVBRQuality,
        0,
        TTKitDefaultSpeexVBRMaxBitrate,
        TRUE,
        TTKitDefaultMSecPerPacket,
        FALSE
    };
    return speexVBRCodec;
}

OpusCodec TTKitGetOpusCodec(const AudioCodec* audioCodec) {
    return audioCodec->opus;
}

SpeexCodec TTKitGetSpeexCodec(const AudioCodec* audioCodec) {
    return audioCodec->speex;
}

SpeexVBRCodec TTKitGetSpeexVBRCodec(const AudioCodec* audioCodec) {
    return audioCodec->speex_vbr;
}

void TTKitSetOpusCodec(AudioCodec* audioCodec, const OpusCodec* opusCodec) {
    audioCodec->nCodec = OPUS_CODEC;
    audioCodec->opus = *opusCodec;
}

void TTKitSetSpeexCodec(AudioCodec* audioCodec, const SpeexCodec* speexCodec) {
    audioCodec->nCodec = SPEEX_CODEC;
    audioCodec->speex = *speexCodec;
}

void TTKitSetSpeexVBRCodec(AudioCodec* audioCodec, const SpeexVBRCodec* speexVBRCodec) {
    audioCodec->nCodec = SPEEX_VBR_CODEC;
    audioCodec->speex_vbr = *speexVBRCodec;
}

Channel TTKitMessageChannel(const TTMessage* message) {
    return message->channel;
}

User TTKitMessageUser(const TTMessage* message) {
    return message->user;
}

ServerProperties TTKitMessageServerProperties(const TTMessage* message) {
    return message->serverproperties;
}

ServerStatistics TTKitMessageServerStatistics(const TTMessage* message) {
    return message->serverstatistics;
}

UserAccount TTKitMessageUserAccount(const TTMessage* message) {
    return message->useraccount;
}

BannedUser TTKitMessageBannedUser(const TTMessage* message) {
    return message->banneduser;
}

ClientErrorMsg TTKitMessageClientError(const TTMessage* message) {
    return message->clienterrormsg;
}

TextMessage TTKitMessageTextMessage(const TTMessage* message) {
    return message->textmessage;
}

RemoteFile TTKitMessageRemoteFile(const TTMessage* message) {
    return message->remotefile;
}

FileTransfer TTKitMessageFileTransfer(const TTMessage* message) {
    return message->filetransfer;
}

TTBOOL TTKitMessageActiveFlag(const TTMessage* message) {
    return message->bActive;
}

const TTCHAR* TTKitGetUserString(TTKitUserStringProperty property, const User* user) {
    switch (property) {
    case TTKitUserStringNickname:
        return user->szNickname;
    case TTKitUserStringUsername:
        return user->szUsername;
    case TTKitUserStringStatusMessage:
        return user->szStatusMsg;
    case TTKitUserStringIPAddress:
        return user->szIPAddress;
    case TTKitUserStringClientName:
        return user->szClientName;
    }
    return "";
}

const TTCHAR* TTKitGetChannelString(TTKitChannelStringProperty property, const Channel* channel) {
    switch (property) {
    case TTKitChannelStringName:
        return channel->szName;
    case TTKitChannelStringPassword:
        return channel->szPassword;
    case TTKitChannelStringTopic:
        return channel->szTopic;
    case TTKitChannelStringOperatorPassword:
        return channel->szOpPassword;
    }
    return "";
}

const TTCHAR* TTKitGetTextMessageString(const TextMessage* message) {
    return message->szMessage;
}

const TTCHAR* TTKitGetServerPropertiesString(TTKitServerStringProperty property, const ServerProperties* serverProperties) {
    switch (property) {
    case TTKitServerStringName:
        return serverProperties->szServerName;
    case TTKitServerStringMessageOfTheDay:
        return serverProperties->szMOTD;
    case TTKitServerStringRawMessageOfTheDay:
        return serverProperties->szMOTDRaw;
    case TTKitServerStringVersion:
        return serverProperties->szServerVersion;
    case TTKitServerStringProtocolVersion:
        return serverProperties->szServerProtocolVersion;
    case TTKitServerStringAccessToken:
        return serverProperties->szAccessToken;
    }
    return "";
}

const TTCHAR* TTKitGetClientErrorMessageString(const ClientErrorMsg* clientError) {
    return clientError->szErrorMsg;
}

const TTCHAR* TTKitGetUserAccountString(TTKitUserAccountStringProperty property, const UserAccount* userAccount) {
    switch (property) {
    case TTKitUserAccountStringUsername:
        return userAccount->szUsername;
    case TTKitUserAccountStringPassword:
        return userAccount->szPassword;
    case TTKitUserAccountStringInitialChannel:
        return userAccount->szInitChannel;
    case TTKitUserAccountStringNote:
        return userAccount->szNote;
    case TTKitUserAccountStringLastModified:
        return userAccount->szLastModified;
    case TTKitUserAccountStringLastLoginTime:
        return userAccount->szLastLoginTime;
    }
    return "";
}

const TTCHAR* TTKitGetBannedUserString(TTKitBannedUserStringProperty property, const BannedUser* bannedUser) {
    switch (property) {
    case TTKitBannedUserStringIPAddress:
        return bannedUser->szIPAddress;
    case TTKitBannedUserStringChannelPath:
        return bannedUser->szChannelPath;
    case TTKitBannedUserStringBanTime:
        return bannedUser->szBanTime;
    case TTKitBannedUserStringNickname:
        return bannedUser->szNickname;
    case TTKitBannedUserStringUsername:
        return bannedUser->szUsername;
    case TTKitBannedUserStringOwner:
        return bannedUser->szOwner;
    }
    return "";
}

const TTCHAR* TTKitGetRemoteFileString(TTKitRemoteFileStringProperty property, const RemoteFile* remoteFile) {
    switch (property) {
    case TTKitRemoteFileStringFileName:
        return remoteFile->szFileName;
    case TTKitRemoteFileStringUsername:
        return remoteFile->szUsername;
    case TTKitRemoteFileStringUploadTime:
        return remoteFile->szUploadTime;
    }
    return "";
}

const TTCHAR* TTKitGetFileTransferString(TTKitFileTransferStringProperty property, const FileTransfer* fileTransfer) {
    switch (property) {
    case TTKitFileTransferStringLocalFilePath:
        return fileTransfer->szLocalFilePath;
    case TTKitFileTransferStringRemoteFileName:
        return fileTransfer->szRemoteFileName;
    }
    return "";
}

void TTKitSetChannelString(TTKitChannelStringProperty property, Channel* channel, const TTCHAR* string) {
    switch (property) {
    case TTKitChannelStringName:
        strncpy(channel->szName, string, TT_STRLEN);
        channel->szName[TT_STRLEN - 1] = '\0';
        break;
    case TTKitChannelStringPassword:
        strncpy(channel->szPassword, string, TT_STRLEN);
        channel->szPassword[TT_STRLEN - 1] = '\0';
        break;
    case TTKitChannelStringTopic:
        strncpy(channel->szTopic, string, TT_STRLEN);
        channel->szTopic[TT_STRLEN - 1] = '\0';
        break;
    case TTKitChannelStringOperatorPassword:
        strncpy(channel->szOpPassword, string, TT_STRLEN);
        channel->szOpPassword[TT_STRLEN - 1] = '\0';
        break;
    }
}

INT32 TTKitGetChannelTransmitUserID(const Channel* channel, INT32 index) {
    if (!channel || index < 0 || index >= TT_TRANSMITUSERS_MAX) {
        return 0;
    }
    return channel->transmitUsers[index][0];
}

StreamTypes TTKitGetChannelTransmitUserStreamTypes(const Channel* channel, INT32 index) {
    if (!channel || index < 0 || index >= TT_TRANSMITUSERS_MAX) {
        return STREAMTYPE_NONE;
    }
    return (StreamTypes) channel->transmitUsers[index][1];
}

void TTKitSetChannelTransmitUser(Channel* channel, INT32 index, INT32 userID, StreamTypes streamTypes) {
    if (!channel || index < 0 || index >= TT_TRANSMITUSERS_MAX) {
        return;
    }
    channel->transmitUsers[index][0] = userID;
    channel->transmitUsers[index][1] = (INT32) streamTypes;
}

INT32 TTKitGetChannelTransmitQueueUserID(const Channel* channel, INT32 index) {
    if (!channel || index < 0 || index >= TT_TRANSMITQUEUE_MAX) {
        return 0;
    }
    return channel->transmitUsersQueue[index];
}

void TTKitSetChannelTransmitQueueUser(Channel* channel, INT32 index, INT32 userID) {
    if (!channel || index < 0 || index >= TT_TRANSMITQUEUE_MAX) {
        return;
    }
    channel->transmitUsersQueue[index] = userID;
}

void TTKitSetServerPropertiesString(TTKitServerStringProperty property, ServerProperties* serverProperties, const TTCHAR* string) {
    switch (property) {
    case TTKitServerStringName:
        strncpy(serverProperties->szServerName, string, TT_STRLEN);
        serverProperties->szServerName[TT_STRLEN - 1] = '\0';
        break;
    case TTKitServerStringMessageOfTheDay:
        strncpy(serverProperties->szMOTD, string, TT_STRLEN);
        serverProperties->szMOTD[TT_STRLEN - 1] = '\0';
        break;
    case TTKitServerStringRawMessageOfTheDay:
        strncpy(serverProperties->szMOTDRaw, string, TT_STRLEN);
        serverProperties->szMOTDRaw[TT_STRLEN - 1] = '\0';
        break;
    case TTKitServerStringVersion:
        strncpy(serverProperties->szServerVersion, string, TT_STRLEN);
        serverProperties->szServerVersion[TT_STRLEN - 1] = '\0';
        break;
    case TTKitServerStringProtocolVersion:
        strncpy(serverProperties->szServerProtocolVersion, string, TT_STRLEN);
        serverProperties->szServerProtocolVersion[TT_STRLEN - 1] = '\0';
        break;
    case TTKitServerStringAccessToken:
        strncpy(serverProperties->szAccessToken, string, TT_STRLEN);
        serverProperties->szAccessToken[TT_STRLEN - 1] = '\0';
        break;
    }
}

void TTKitSetUserAccountString(TTKitUserAccountStringProperty property, UserAccount* userAccount, const TTCHAR* string) {
    switch (property) {
    case TTKitUserAccountStringUsername:
        strncpy(userAccount->szUsername, string, TT_STRLEN);
        userAccount->szUsername[TT_STRLEN - 1] = '\0';
        break;
    case TTKitUserAccountStringPassword:
        strncpy(userAccount->szPassword, string, TT_STRLEN);
        userAccount->szPassword[TT_STRLEN - 1] = '\0';
        break;
    case TTKitUserAccountStringInitialChannel:
        strncpy(userAccount->szInitChannel, string, TT_STRLEN);
        userAccount->szInitChannel[TT_STRLEN - 1] = '\0';
        break;
    case TTKitUserAccountStringNote:
        strncpy(userAccount->szNote, string, TT_STRLEN);
        userAccount->szNote[TT_STRLEN - 1] = '\0';
        break;
    case TTKitUserAccountStringLastModified:
        strncpy(userAccount->szLastModified, string, TT_STRLEN);
        userAccount->szLastModified[TT_STRLEN - 1] = '\0';
        break;
    case TTKitUserAccountStringLastLoginTime:
        strncpy(userAccount->szLastLoginTime, string, TT_STRLEN);
        userAccount->szLastLoginTime[TT_STRLEN - 1] = '\0';
        break;
    }
}

void TTKitSetUserAccountAutoOperatorChannels(UserAccount* userAccount, const INT32* channelIDs, INT32 count) {
    for (INT32 i = 0; i < TT_CHANNELS_OPERATOR_MAX; ++i) {
        userAccount->autoOperatorChannels[i] = 0;
    }

    INT32 upperBound = count < TT_CHANNELS_OPERATOR_MAX ? count : TT_CHANNELS_OPERATOR_MAX;
    for (INT32 i = 0; i < upperBound; ++i) {
        userAccount->autoOperatorChannels[i] = channelIDs[i];
    }
}

INT32 TTKitGetUserAccountAutoOperatorChannel(const UserAccount* userAccount, INT32 index) {
    if (index < 0 || index >= TT_CHANNELS_OPERATOR_MAX) {
        return 0;
    }
    return userAccount->autoOperatorChannels[index];
}

void TTKitSetBannedUserString(TTKitBannedUserStringProperty property, BannedUser* bannedUser, const TTCHAR* string) {
    switch (property) {
    case TTKitBannedUserStringIPAddress:
        strncpy(bannedUser->szIPAddress, string, TT_STRLEN);
        bannedUser->szIPAddress[TT_STRLEN - 1] = '\0';
        break;
    case TTKitBannedUserStringChannelPath:
        strncpy(bannedUser->szChannelPath, string, TT_STRLEN);
        bannedUser->szChannelPath[TT_STRLEN - 1] = '\0';
        break;
    case TTKitBannedUserStringBanTime:
        strncpy(bannedUser->szBanTime, string, TT_STRLEN);
        bannedUser->szBanTime[TT_STRLEN - 1] = '\0';
        break;
    case TTKitBannedUserStringNickname:
        strncpy(bannedUser->szNickname, string, TT_STRLEN);
        bannedUser->szNickname[TT_STRLEN - 1] = '\0';
        break;
    case TTKitBannedUserStringUsername:
        strncpy(bannedUser->szUsername, string, TT_STRLEN);
        bannedUser->szUsername[TT_STRLEN - 1] = '\0';
        break;
    case TTKitBannedUserStringOwner:
        strncpy(bannedUser->szOwner, string, TT_STRLEN);
        bannedUser->szOwner[TT_STRLEN - 1] = '\0';
        break;
    }
}

void TTKitSetTextMessageString(TextMessage* message, const TTCHAR* string) {
    strncpy(message->szMessage, string, TT_STRLEN);
    message->szMessage[TT_STRLEN - 1] = '\0';
}

void TTKitSetEncryptionString(TTKitEncryptionStringProperty property, EncryptionContext* encryption, const TTCHAR* string) {
    switch (property) {
    case TTKitEncryptionCAFile:
        strncpy(encryption->szCAFile, string, TT_STRLEN);
        encryption->szCAFile[TT_STRLEN - 1] = '\0';
        break;
    case TTKitEncryptionCertificateFile:
        strncpy(encryption->szCertificateFile, string, TT_STRLEN);
        encryption->szCertificateFile[TT_STRLEN - 1] = '\0';
        break;
    case TTKitEncryptionPrivateKeyFile:
        strncpy(encryption->szPrivateKeyFile, string, TT_STRLEN);
        encryption->szPrivateKeyFile[TT_STRLEN - 1] = '\0';
        break;
    }
}

#include "TeamTalk.h"

typedef enum {
    TTKitUserStringNickname,
    TTKitUserStringUsername,
    TTKitUserStringStatusMessage,
    TTKitUserStringIPAddress,
    TTKitUserStringClientName
} TTKitUserStringProperty;

typedef enum {
    TTKitChannelStringName,
    TTKitChannelStringPassword,
    TTKitChannelStringTopic,
    TTKitChannelStringOperatorPassword
} TTKitChannelStringProperty;

typedef enum {
    TTKitServerStringName,
    TTKitServerStringMessageOfTheDay,
    TTKitServerStringRawMessageOfTheDay,
    TTKitServerStringVersion,
    TTKitServerStringProtocolVersion,
    TTKitServerStringAccessToken
} TTKitServerStringProperty;

typedef enum {
    TTKitUserAccountStringUsername,
    TTKitUserAccountStringPassword,
    TTKitUserAccountStringInitialChannel,
    TTKitUserAccountStringNote,
    TTKitUserAccountStringLastModified,
    TTKitUserAccountStringLastLoginTime
} TTKitUserAccountStringProperty;

typedef enum {
    TTKitBannedUserStringIPAddress,
    TTKitBannedUserStringChannelPath,
    TTKitBannedUserStringBanTime,
    TTKitBannedUserStringNickname,
    TTKitBannedUserStringUsername,
    TTKitBannedUserStringOwner
} TTKitBannedUserStringProperty;

typedef enum {
    TTKitEncryptionCAFile,
    TTKitEncryptionCertificateFile,
    TTKitEncryptionPrivateKeyFile
} TTKitEncryptionStringProperty;

typedef enum {
    TTKitRemoteFileStringFileName,
    TTKitRemoteFileStringUsername,
    TTKitRemoteFileStringUploadTime
} TTKitRemoteFileStringProperty;

typedef enum {
    TTKitFileTransferStringLocalFilePath,
    TTKitFileTransferStringRemoteFileName
} TTKitFileTransferStringProperty;

AudioCodec TTKitMakeAudioCodec(Codec codec);
OpusCodec TTKitMakeOpusCodec(void);
SpeexCodec TTKitMakeSpeexCodec(void);
SpeexVBRCodec TTKitMakeSpeexVBRCodec(void);

OpusCodec TTKitGetOpusCodec(const AudioCodec* audioCodec);
SpeexCodec TTKitGetSpeexCodec(const AudioCodec* audioCodec);
SpeexVBRCodec TTKitGetSpeexVBRCodec(const AudioCodec* audioCodec);

void TTKitSetOpusCodec(AudioCodec* audioCodec, const OpusCodec* opusCodec);
void TTKitSetSpeexCodec(AudioCodec* audioCodec, const SpeexCodec* speexCodec);
void TTKitSetSpeexVBRCodec(AudioCodec* audioCodec, const SpeexVBRCodec* speexVBRCodec);

Channel TTKitMessageChannel(const TTMessage* message);
User TTKitMessageUser(const TTMessage* message);
ServerProperties TTKitMessageServerProperties(const TTMessage* message);
ServerStatistics TTKitMessageServerStatistics(const TTMessage* message);
UserAccount TTKitMessageUserAccount(const TTMessage* message);
BannedUser TTKitMessageBannedUser(const TTMessage* message);
ClientErrorMsg TTKitMessageClientError(const TTMessage* message);
TextMessage TTKitMessageTextMessage(const TTMessage* message);
RemoteFile TTKitMessageRemoteFile(const TTMessage* message);
FileTransfer TTKitMessageFileTransfer(const TTMessage* message);
TTBOOL TTKitMessageActiveFlag(const TTMessage* message);

const TTCHAR* TTKitGetUserString(TTKitUserStringProperty property, const User* user);
const TTCHAR* TTKitGetChannelString(TTKitChannelStringProperty property, const Channel* channel);
INT32 TTKitGetChannelTransmitUserID(const Channel* channel, INT32 index);
StreamTypes TTKitGetChannelTransmitUserStreamTypes(const Channel* channel, INT32 index);
INT32 TTKitGetChannelTransmitQueueUserID(const Channel* channel, INT32 index);
const TTCHAR* TTKitGetTextMessageString(const TextMessage* message);
const TTCHAR* TTKitGetServerPropertiesString(TTKitServerStringProperty property, const ServerProperties* serverProperties);
const TTCHAR* TTKitGetClientErrorMessageString(const ClientErrorMsg* clientError);
const TTCHAR* TTKitGetUserAccountString(TTKitUserAccountStringProperty property, const UserAccount* userAccount);
const TTCHAR* TTKitGetBannedUserString(TTKitBannedUserStringProperty property, const BannedUser* bannedUser);
const TTCHAR* TTKitGetRemoteFileString(TTKitRemoteFileStringProperty property, const RemoteFile* remoteFile);
const TTCHAR* TTKitGetFileTransferString(TTKitFileTransferStringProperty property, const FileTransfer* fileTransfer);

void TTKitSetChannelString(TTKitChannelStringProperty property, Channel* channel, const TTCHAR* string);
void TTKitSetChannelTransmitUser(Channel* channel, INT32 index, INT32 userID, StreamTypes streamTypes);
void TTKitSetChannelTransmitQueueUser(Channel* channel, INT32 index, INT32 userID);
void TTKitSetServerPropertiesString(TTKitServerStringProperty property, ServerProperties* serverProperties, const TTCHAR* string);
void TTKitSetUserAccountString(TTKitUserAccountStringProperty property, UserAccount* userAccount, const TTCHAR* string);
void TTKitSetUserAccountAutoOperatorChannels(UserAccount* userAccount, const INT32* channelIDs, INT32 count);
INT32 TTKitGetUserAccountAutoOperatorChannel(const UserAccount* userAccount, INT32 index);
void TTKitSetBannedUserString(TTKitBannedUserStringProperty property, BannedUser* bannedUser, const TTCHAR* string);
void TTKitSetTextMessageString(TextMessage* message, const TTCHAR* string);
void TTKitSetEncryptionString(TTKitEncryptionStringProperty property, EncryptionContext* encryption, const TTCHAR* string);

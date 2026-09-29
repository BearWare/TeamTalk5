#include "../../../../../../Library/TeamTalk_DLL/TeamTalk.h"

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
    TTKitServerStringAccessToken
} TTKitServerStringProperty;

typedef enum {
    TTKitUserAccountStringInitialChannel
} TTKitUserAccountStringProperty;

typedef enum {
    TTKitEncryptionCAFile,
    TTKitEncryptionCertificateFile,
    TTKitEncryptionPrivateKeyFile
} TTKitEncryptionStringProperty;

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
UserAccount TTKitMessageUserAccount(const TTMessage* message);
ClientErrorMsg TTKitMessageClientError(const TTMessage* message);
TextMessage TTKitMessageTextMessage(const TTMessage* message);
TTBOOL TTKitMessageActiveFlag(const TTMessage* message);

const TTCHAR* TTKitGetUserString(TTKitUserStringProperty property, const User* user);
const TTCHAR* TTKitGetChannelString(TTKitChannelStringProperty property, const Channel* channel);
const TTCHAR* TTKitGetTextMessageString(const TextMessage* message);
const TTCHAR* TTKitGetServerPropertiesString(TTKitServerStringProperty property, const ServerProperties* serverProperties);
const TTCHAR* TTKitGetClientErrorMessageString(const ClientErrorMsg* clientError);
const TTCHAR* TTKitGetUserAccountString(TTKitUserAccountStringProperty property, const UserAccount* userAccount);

void TTKitSetChannelString(TTKitChannelStringProperty property, Channel* channel, const TTCHAR* string);
void TTKitSetTextMessageString(TextMessage* message, const TTCHAR* string);
void TTKitSetEncryptionString(TTKitEncryptionStringProperty property, EncryptionContext* encryption, const TTCHAR* string);

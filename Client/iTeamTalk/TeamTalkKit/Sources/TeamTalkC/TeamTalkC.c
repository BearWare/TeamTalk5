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

UserAccount TTKitMessageUserAccount(const TTMessage* message) {
    return message->useraccount;
}

ClientErrorMsg TTKitMessageClientError(const TTMessage* message) {
    return message->clienterrormsg;
}

TextMessage TTKitMessageTextMessage(const TTMessage* message) {
    return message->textmessage;
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
    case TTKitUserAccountStringInitialChannel:
        return userAccount->szInitChannel;
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

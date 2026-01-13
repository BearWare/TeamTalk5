#include "AACBackend.h"

// Adjust this include to however the TeamTalk C API is exposed in your build.
// In upstream it’s typically something like:
//   #include "TeamTalk.h"
#include "TeamTalk.h"

AACBackend::AACBackend(TTInstance* ttInstance, QObject* parent)
    : QObject(parent)
    , m_ttInstance(ttInstance)
{
}

void AACBackend::refreshChannels()
{
    if (!m_ttInstance) {
        emit channelsEnumerated({});
        return;
    }

    const QList<ChannelInfo> channels = enumerateChannels();
    emit channelsEnumerated(channels);
}

QList<ChannelInfo> AACBackend::enumerateChannels() const
{
    QList<ChannelInfo> out;

    if (!m_ttInstance)
        return out;

    // The exact enumeration logic depends on the TeamTalk C API in your branch.
    // The *shape* will look roughly like this:
    //
    // 1. Get all channel IDs (or walk the tree starting from the root).
    // 2. For each channel, call TT_GetChannel (or equivalent).
    // 3. Fill a ChannelInfo and append to 'out'.

    // --- PSEUDOCODE / TEMPLATE START ---
    //
    // Example pattern; replace with real calls from your TeamTalk.h:
    //
    // int channelIDs[TT_MAX_CHANNEL]; // or use std::vector<int>
    // int count = 0;
    //
    // if (TT_GetServerChannels(m_ttInstance, channelIDs, &count)) {
    //     for (int i = 0; i < count; ++i) {
    //         TTChannel chan = {};
    //         if (!TT_GetChannel(m_ttInstance, channelIDs[i], &chan))
    //             continue;
    //
    //         ChannelInfo ci;
    //         ci.id = chan.nChannelID;
    //         ci.name = QString::fromUtf8(chan.szName);
    //         ci.userCount = chan.nUsers;
    //         ci.isPasswordProtected = chan.bPassword;
    //         ci.topic = QString::fromUtf8(chan.szTopic);
    //
    //         out.append(ci);
    //     }
    // }
    //
    // --- PSEUDOCODE / TEMPLATE END ---

    return out;
}

#pragma once

#include <QString>
#include <QList>

// ------------------------------------------------------------
// Connection state
// ------------------------------------------------------------
enum class ConnectionState {
    Connecting,
    Connected,
    Disconnected
};

// ------------------------------------------------------------
// Channel info
// ------------------------------------------------------------
struct ChannelInfo {
    int id;
    int parentId;
    QString name;
};

// ------------------------------------------------------------
// Channel events
// ------------------------------------------------------------
enum class ChannelEventType {
    Joined,
    Left
};

struct ChannelEvent {
    ChannelEventType type;
    int channelId;
};

// ------------------------------------------------------------
// Error events
// ------------------------------------------------------------
struct ErrorEvent {
    QString message;
};

// ------------------------------------------------------------
// Self voice state
// ------------------------------------------------------------
enum class SelfVoiceState {
    Silent,
    Transmitting
};

struct SelfVoiceEvent {
    SelfVoiceState state;
};

// ------------------------------------------------------------
// Other users' voice state
// ------------------------------------------------------------
enum class OtherUserVoiceState {
    Silent,
    Speaking
};

struct OtherUserVoiceEvent {
    int userId;
    QString username;
    OtherUserVoiceState state;
};

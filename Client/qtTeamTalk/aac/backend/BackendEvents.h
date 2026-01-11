#pragma once

#include <QString>

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class ChannelEventType {
    Joined,
    Left
};

struct ChannelEvent {
    ChannelEventType type;
    int channelId;
};

struct ErrorEvent {
    QString message;
};

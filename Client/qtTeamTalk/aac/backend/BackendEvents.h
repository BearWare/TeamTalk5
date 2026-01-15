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

// ---------------------------------------------------------------------
// AAC‑RELEVANT ADDITIONS
// ---------------------------------------------------------------------

// 1. Self voice state (for your own transmit indicator)
enum class SelfVoiceState {
    Silent,
    Talking
};

struct SelfVoiceEvent {
    SelfVoiceState state;
};

// 2. Audio device events (mic/headset added/removed/failure)
enum class AudioDeviceEventType {
    Added,
    Removed,
    Failed
};

struct AudioDeviceEvent {
    AudioDeviceEventType type;
};

// 3. Optional: incoming text messages (for TTS surfaces)
struct TextMessageEvent {
    int fromUserId;
    QString message;
};

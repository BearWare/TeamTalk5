#pragma once

namespace AAC {

enum class EventType {
    None,
    ConnectRequested,
    Connected,
    ConnectionFailed,
    Disconnected
};

struct Event {
    EventType type = EventType::None;
};

} // namespace AAC

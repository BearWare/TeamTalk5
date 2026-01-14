#pragma once

// Placeholder for AAC UI event types.
// Future revisions will define a full event model for the AAC state machine.

enum class EventType {
    None,
    ConnectRequested,
    Connected,
    ConnectionFailed,
    Disconnected
};

struct Event {
    EventType type = EventType::None;
    // TODO: Add event payload fields as needed
};

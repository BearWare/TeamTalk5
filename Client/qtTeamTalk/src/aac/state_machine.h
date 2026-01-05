#pragma once

#include "events.h"

// AAC UI state machine.
//
// This class will eventually drive the AAC-native interaction flow
// for the Qt client. The current version only defines basic states
// and a placeholder event handler. No real behavior is implemented yet.

namespace AAC {

enum class State {
    Idle,
    Connecting,
    Connected,
    Error
};

class StateMachine {
public:
    StateMachine() = default;

    State currentState() const { return state_; }

    // Handle an incoming event.
    // Current implementation is placeholder-only and does not change state.
    void handleEvent(const Event& event);

private:
    State state_ = State::Idle;
};

} // namespace AAC

#pragma once
#include "events.h"

enum class UIState {
    Idle,
    Connecting,
    Connected,
    Error
};

class StateMachine {
public:
    StateMachine();
    void dispatch(const Event& e);
    UIState current() const;

private:
    UIState state;
    void transition(UIState next);
};

#pragma once

#include "ui_intent.h"
#include "state_machine.h"

namespace AAC {
namespace UI {
namespace Intent {

class UiIntentRouter {
public:
    explicit UiIntentRouter(Core::StateMachine &sm);

    void handleConnect(const ConnectIntent &intent);
    void handleDisconnect(const DisconnectIntent &intent);
    void handleToggleTransmit(const ToggleTransmitIntent &intent);
    void handleToggleMute(const ToggleMuteIntent &intent);

private:
    Core::StateMachine &sm_;
};

} // namespace Intent
} // namespace UI
} // namespace AAC

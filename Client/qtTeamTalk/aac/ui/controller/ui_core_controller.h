#pragma once

#include "ui_event_dispatcher.h"
#include "state_machine.h"

namespace AAC {
namespace UI {
namespace Controller {

class UiCoreController {
public:
    UiCoreController(Events::UiEventDispatcherInterface &dispatcher,
                     Core::StateMachine &stateMachine);
    ~UiCoreController();

private:
    void handleToggleTransmit(bool enabled);
    void handleToggleMute(bool muted);
    void handleConnectRequested(const QString &host, int port);
    void handleDisconnectRequested();

    Events::UiEventDispatcherInterface *dispatcher_;
    Core::StateMachine *stateMachine_;

    int toggleTransmitHandlerId_;
    int toggleMuteHandlerId_;
    int connectHandlerId_;
    int disconnectHandlerId_;
};

} // namespace Controller
} // namespace UI
} // namespace AAC

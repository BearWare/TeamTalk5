#include "ui_core_controller.h"

#include <QDebug>

namespace AAC {
namespace UI {
namespace Controller {

UiCoreController::UiCoreController(Events::UiEventDispatcherInterface &dispatcher,
                                   Core::StateMachine &stateMachine)
    : dispatcher_(&dispatcher),
      stateMachine_(&stateMachine)
{
    toggleTransmitHandlerId_ = dispatcher_->subscribe(
        Events::UiEventType::ToggleTransmit,
        [this](const Events::UiEvent &event) {
            handleToggleTransmit(event.flag);
        });

    toggleMuteHandlerId_ = dispatcher_->subscribe(
        Events::UiEventType::ToggleMute,
        [this](const Events::UiEvent &event) {
            handleToggleMute(event.flag);
        });

    connectHandlerId_ = dispatcher_->subscribe(
        Events::UiEventType::ConnectRequested,
        [this](const Events::UiEvent &event) {
            handleConnectRequested(event.host, event.port);
        });

    disconnectHandlerId_ = dispatcher_->subscribe(
        Events::UiEventType::DisconnectRequested,
        [this](const Events::UiEvent &) {
            handleDisconnectRequested();
        });
}

UiCoreController::~UiCoreController()
{
    dispatcher_->unsubscribe(Events::UiEventType::ToggleTransmit, toggleTransmitHandlerId_);
    dispatcher_->unsubscribe(Events::UiEventType::ToggleMute, toggleMuteHandlerId_);
    dispatcher_->unsubscribe(Events::UiEventType::ConnectRequested, connectHandlerId_);
    dispatcher_->unsubscribe(Events::UiEventType::DisconnectRequested, disconnectHandlerId_);
}

void UiCoreController::handleToggleTransmit(bool enabled)
{
    stateMachine_->requestToggleTransmit(enabled);
}

void UiCoreController::handleToggleMute(bool muted)
{
    stateMachine_->requestToggleMute(muted);
}

void UiCoreController::handleConnectRequested(const QString &host, int port)
{
    Core::ConnectRequest req;
    req.host = host;
    req.port = port;
    stateMachine_->requestConnect(req);
}

void UiCoreController::handleDisconnectRequested()
{
    stateMachine_->requestDisconnect();
}

} // namespace Controller
} // namespace UI
} // namespace AAC

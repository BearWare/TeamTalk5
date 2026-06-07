#include "ui_intent_router.h"

namespace AAC {
namespace UI {
namespace Intent {

UiIntentRouter::UiIntentRouter(Core::StateMachine &sm)
    : sm_(sm)
{
}

void UiIntentRouter::handleConnect(const ConnectIntent &intent)
{
    Core::ConnectRequest req;
    req.host = intent.host;
    req.port = intent.port;
    sm_.requestConnect(req);
}

void UiIntentRouter::handleDisconnect(const DisconnectIntent &)
{
    sm_.requestDisconnect();
}

void UiIntentRouter::handleToggleTransmit(const ToggleTransmitIntent &intent)
{
    sm_.requestToggleTransmit(intent.enabled);
}

void UiIntentRouter::handleToggleMute(const ToggleMuteIntent &intent)
{
    sm_.requestToggleMute(intent.muted);
}

} // namespace Intent
} // namespace UI
} // namespace AAC

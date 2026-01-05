#include "state_machine.h"
#include "events.h"

namespace AAC {

void StateMachine::handleEvent(const Event& event)
{
    // TODO: Implement real state transitions in a future PR.

    switch (event.type) {
    case EventType::ConnectRequested:
        // Placeholder: future logic will move from Idle -> Connecting.
        break;

    case EventType::Connected:
        // Placeholder: future logic will move to Connected.
        break;

    case EventType::ConnectionFailed:
        // Placeholder: future logic will move to Error.
        break;

    case EventType::Disconnected:
        // Placeholder: future logic will move back to Idle.
        break;

    case EventType::None:
    default:
        // No action.
        break;
    }
}

} // namespace AAC

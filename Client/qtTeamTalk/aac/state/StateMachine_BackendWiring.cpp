#include "StateMachine.h"
#include "BackendAdapter.h"

void StateMachine::attachBackend(BackendAdapter* backend)
{
    m_backend = backend;

    // Backend → StateMachine
    connect(backend, &BackendAdapter::connectionStateChanged,
            this, &StateMachine::onConnectionStateChanged);

    connect(backend, &BackendAdapter::channelEvent,
            this, &StateMachine::onChannelEvent);

    connect(backend, &BackendAdapter::errorOccurred,
            this, &StateMachine::onErrorOccurred);
}

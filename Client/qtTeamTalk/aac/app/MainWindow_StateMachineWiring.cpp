#include "MainWindow.h"
#include "StateMachine.h"
#include "backend/BackendAdapter.h"

void MainWindow::attachStateMachine(StateMachine* sm)
{
    // Backend → StateMachine
    connect(m_backend, &BackendAdapter::connectionStateChanged,
            sm, &StateMachine::onConnectionStateChanged);

    connect(m_backend, &BackendAdapter::channelEvent,
            sm, &StateMachine::onChannelEvent);

    connect(m_backend, &BackendAdapter::errorOccurred,
            sm, &StateMachine::onBackendError);

    // StateMachine → UI
    connect(sm, &StateMachine::requestUiUpdate,
            this, [this]() {
                // TODO: trigger AAC-native UI refresh
            });
}

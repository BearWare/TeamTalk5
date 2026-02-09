#pragma once

#include "BackendEvents.h"

struct BackendState {
    ConnectionState connectionState = ConnectionState::Disconnected;
    int currentChannelId = -1;
};

#pragma once

#include <QString>

namespace AAC {
namespace UI {
namespace Intent {

struct ConnectIntent {
    QString host;
    int port;
};

struct ToggleTransmitIntent {
    bool enabled;
};

struct ToggleMuteIntent {
    bool muted;
};

struct DisconnectIntent { };

} // namespace Intent
} // namespace UI
} // namespace AAC

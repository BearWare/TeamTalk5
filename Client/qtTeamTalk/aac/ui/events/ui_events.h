#pragma once

#include <QString>

namespace AAC {
namespace UI {
namespace Events {

enum class UiEventType {
    ToggleTransmit,
    ToggleMute,
    ConnectRequested,
    DisconnectRequested
};

struct UiEvent {
    UiEventType type;

    bool flag = false;
    QString host;
    int port = 0;

    static UiEvent toggleTransmit(bool enabled)
    {
        UiEvent e;
        e.type = UiEventType::ToggleTransmit;
        e.flag = enabled;
        return e;
    }

    static UiEvent toggleMute(bool muted)
    {
        UiEvent e;
        e.type = UiEventType::ToggleMute;
        e.flag = muted;
        return e;
    }

    static UiEvent connectRequested(const QString &host, int port)
    {
        UiEvent e;
        e.type = UiEventType::ConnectRequested;
        e.host = host;
        e.port = port;
        return e;
    }

    static UiEvent disconnectRequested()
    {
        UiEvent e;
        e.type = UiEventType::DisconnectRequested;
        return e;
    }
};

} // namespace Events
} // namespace UI
} // namespace AAC

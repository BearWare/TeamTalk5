#pragma once

#include <QString>
#include <functional>

namespace AAC {
namespace Core {

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class AudioState {
    Idle,
    Transmitting,
    Muted
};

struct ConnectRequest {
    QString host;
    int port;
};

struct ErrorInfo {
    QString message;
};

class StateMachine {
public:
    StateMachine();

    // UI → Core
    void requestConnect(const ConnectRequest &req);
    void requestDisconnect();
    void requestToggleTransmit(bool enabled);
    void requestToggleMute(bool muted);

    // Backend → Core (QtTeamTalk backend will call these)
    void onBackendConnected();
    void onBackendDisconnected();
    void onBackendError(const ErrorInfo &err);

    // Observers
    std::function<void(ConnectionState)> connectionStateChanged;
    std::function<void(AudioState)> audioStateChanged;
    std::function<void(const ErrorInfo &)> errorOccurred;

    ConnectionState connectionState() const { return connectionState_; }
    AudioState audioState() const { return audioState_; }

private:
    void setConnectionState(ConnectionState s);
    void setAudioState(AudioState s);

    ConnectionState connectionState_;
    AudioState audioState_;
};

} // namespace Core
} // namespace AAC

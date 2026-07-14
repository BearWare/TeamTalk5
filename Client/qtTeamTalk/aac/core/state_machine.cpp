#include "state_machine.h"

namespace AAC {
namespace Core {

StateMachine::StateMachine()
    : connectionState_(ConnectionState::Disconnected),
      audioState_(AudioState::Idle)
{
}

void StateMachine::requestConnect(const ConnectRequest & /*req*/)
{
    if (connectionState_ != ConnectionState::Disconnected)
        return;

    setConnectionState(ConnectionState::Connecting);
}

void StateMachine::requestDisconnect()
{
    if (connectionState_ == ConnectionState::Disconnected)
        return;

    setConnectionState(ConnectionState::Disconnected);
    setAudioState(AudioState::Idle);
}

void StateMachine::requestToggleTransmit(bool enabled)
{
    if (connectionState_ != ConnectionState::Connected)
        return;

    setAudioState(enabled ? AudioState::Transmitting : AudioState::Idle);
}

void StateMachine::requestToggleMute(bool muted)
{
    if (connectionState_ != ConnectionState::Connected)
        return;

    setAudioState(muted ? AudioState::Muted : AudioState::Idle);
}

void StateMachine::onBackendConnected()
{
    setConnectionState(ConnectionState::Connected);
}

void StateMachine::onBackendDisconnected()
{
    setConnectionState(ConnectionState::Disconnected);
    setAudioState(AudioState::Idle);
}

void StateMachine::onBackendError(const ErrorInfo &err)
{
    setConnectionState(ConnectionState::Error);
    if (errorOccurred)
        errorOccurred(err);
}

void StateMachine::setConnectionState(ConnectionState s)
{
    if (connectionState_ == s)
        return;

    connectionState_ = s;
    if (connectionStateChanged)
        connectionStateChanged(s);
}

void StateMachine::setAudioState(AudioState s)
{
    if (audioState_ == s)
        return;

    audioState_ = s;
    if (audioStateChanged)
        audioStateChanged(s);
}

} // namespace Core
} // namespace AAC

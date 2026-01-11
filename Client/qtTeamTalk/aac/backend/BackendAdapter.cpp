#include "BackendAdapter.h"

BackendAdapter::BackendAdapter(QObject* parent)
    : QObject(parent)
{
}

void BackendAdapter::connectToServer(const QString& host, int port)
{
    m_state.connectionState = ConnectionState::Connecting;
    emit connectionStateChanged(m_state.connectionState);

    // TODO: Replace with real TeamTalk connect callback
    m_state.connectionState = ConnectionState::Connected;
    emit connectionStateChanged(m_state.connectionState);
}

void BackendAdapter::disconnectFromServer()
{
    m_state.connectionState = ConnectionState::Disconnected;
    emit connectionStateChanged(m_state.connectionState);
}

void BackendAdapter::joinChannel(int channelId)
{
    m_state.currentChannelId = channelId;

    ChannelEvent ev;
    ev.type = ChannelEventType::Joined;
    ev.channelId = channelId;

    emit channelEvent(ev);
}

void BackendAdapter::leaveChannel()
{
    if (m_state.currentChannelId < 0)
        return;

    ChannelEvent ev;
    ev.type = ChannelEventType::Left;
    ev.channelId = m_state.currentChannelId;

    m_state.currentChannelId = -1;

    emit channelEvent(ev);
}

void BackendAdapter::setTransmitEnabled(bool enabled)
{
    // TODO: real transmit toggle
}

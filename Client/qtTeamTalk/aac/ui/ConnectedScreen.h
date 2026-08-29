#pragma once
#include <QWidget>

class ConnectedScreen : public QWidget {
    Q_OBJECT
public:
    explicit ConnectedScreen(QWidget* parent = nullptr);
    void setChannels(const QList<QPair<int, QString>>& channels);

private:
    QVBoxLayout* m_channelListLayout = nullptr;

signals:
    void joinChannelRequested(int channelId);
};

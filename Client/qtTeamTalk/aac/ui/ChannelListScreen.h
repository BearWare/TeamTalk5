#pragma once

#include <QWidget>

class QListWidget;
class QPushButton;

class ChannelListScreen : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelListScreen(QWidget* parent = nullptr);

    void setChannels(const QList<QPair<int, QString>>& channels);

signals:
    void refreshRequested();
    void joinChannelRequested(int channelId);

private:
    QListWidget* m_list = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QPushButton* m_joinButton = nullptr;
};

#pragma once

#include <QWidget>
#include "../backend/channelinfo.h"

class QListWidget;
class QPushButton;

class ChannelListScreen : public QWidget {
    Q_OBJECT
public:
    explicit ChannelListScreen(QWidget* parent = nullptr);

public slots:
    void setChannels(const QList<ChannelInfo>& channels);

signals:
    void refreshRequested();
    void joinChannelRequested(int channelId);

private:
    QListWidget* m_list;
    QPushButton* m_refreshButton;
};

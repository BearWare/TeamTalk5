#pragma once

#include "AACScreen.h"
#include "aac/backend/channelinfo.h"

class QListWidget;
class QPushButton;

class ChannelListScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ChannelListScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

public slots:
    void setChannels(const QList<ChannelInfo>& channels);

signals:
    void refreshRequested();
    void joinChannelRequested(int channelId);

private:
    QListWidget* m_list = nullptr;
    QPushButton* m_refreshButton = nullptr;

    void updateRowHeight();
};

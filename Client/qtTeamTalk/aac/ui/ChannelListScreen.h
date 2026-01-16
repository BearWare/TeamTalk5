#pragma once

#include "AACScreen.h"
#include "../backend/channelinfo.h"

class QListWidget;
class QPushButton;

class ChannelListScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ChannelListScreen(QWidget* parent = nullptr);

public slots:
    void setChannels(const QList<ChannelInfo>& channels);

    // Override to apply AAC scaling
    void applyLargeTargetMode(bool enabled) override;

signals:
    void refreshRequested();
    void joinChannelRequested(int channelId);

private:
    QListWidget* m_list = nullptr;
    QPushButton* m_refreshButton = nullptr;
};

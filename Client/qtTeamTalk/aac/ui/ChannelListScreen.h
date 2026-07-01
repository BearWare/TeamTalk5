#pragma once

#include "AACScreen.h"
#include "aac/backend/channelinfo.h"

class QListWidget;
class QPushButton;

// Channel list screen with:
// - Channel list
// - Refresh button
// - Settings (App Settings)
// - AAC Access (AAC Settings)
class ChannelListScreen : public AACScreen {
    Q_OBJECT
public:
    explicit ChannelListScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

public slots:
    void setChannels(const QList<ChannelInfo>& channels);

signals:
    void refreshRequested();
    void joinChannelRequested(int channelId);
    void settingsRequested();
    void aacSettingsRequested();

private:
    QListWidget* m_list = nullptr;
    QPushButton* m_refreshButton = nullptr;

    void updateRowHeight();
};

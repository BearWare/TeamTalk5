#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "aac/AACFramework.h" // AACScreenAdapter, AACAccessibilityManager

class BackendAdapter;
class AACMainScreen;

struct SelfVoiceState;
struct OtherUserVoiceEvent;

class InChannelScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit InChannelScreen(AACAccessibilityManager* aac,
                             BackendAdapter* backend,
                             QWidget* parent = nullptr);

    // AACScreenAdapter
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override;

    void setChannelName(const QString& name);

signals:
    void leaveChannelRequested();
    void transmitToggled(bool enabled);

public slots:
    void updateSelfVoiceState(const SelfVoiceState& state);
    void updateOtherUserVoiceState(const OtherUserVoiceEvent& event);
    void setEventMessage(const QString& msg);

private slots:
    void onTextCommitted(const QString& text);
    void onLeaveClicked();
    void onPTToggled(bool checked);

private:
    AACAccessibilityManager* m_aac = nullptr;
    BackendAdapter* m_backend = nullptr;

    QVBoxLayout* m_rootLayout = nullptr;
    AACMainScreen* m_aacMain = nullptr;
    QPushButton* m_leaveButton = nullptr;
    QPushButton* m_pttButton = nullptr;
    QLabel* m_channelLabel = nullptr;
    QLabel* m_eventLabel = nullptr;
};

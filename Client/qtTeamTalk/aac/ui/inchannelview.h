#pragma once
#include <QWidget>

class QLabel;
class QPushButton;
class QListView;
class QTextEdit;
class QAbstractItemModel;

class InChannelView : public QWidget {
    Q_OBJECT
public:
    explicit InChannelView(QWidget *parent = nullptr);

    void setChannelLabel(const QString &label);
    void setUserListModel(QAbstractItemModel *model);

signals:
    void leaveChannelRequested();
    void toggleTransmitRequested();
    void toggleMuteRequested();
    void openSettingsRequested();

private:
    QLabel *m_channelLabel;
    QPushButton *m_transmitButton;
    QPushButton *m_muteButton;
    QPushButton *m_leaveButton;
    QPushButton *m_settingsButton;

    QListView *m_userList;
    QTextEdit *m_chatPlaceholder;

    void setupUi();
    void setupConnections();
    void updateTransmitVisual(bool active);
    void updateMuteVisual(bool muted);
};

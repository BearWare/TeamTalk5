#pragma once
#include <QWidget>

class ConnectedScreen : public QWidget {
    Q_OBJECT
public:
    explicit ConnectedScreen(QWidget* parent = nullptr);
signals:
    void joinChannelRequested(int channelId);
};

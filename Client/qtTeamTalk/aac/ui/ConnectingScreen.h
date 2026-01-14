#pragma once

#include <QWidget>

class QLabel;

class ConnectingScreen : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectingScreen(QWidget* parent = nullptr);

private:
    QLabel* m_label = nullptr;
};

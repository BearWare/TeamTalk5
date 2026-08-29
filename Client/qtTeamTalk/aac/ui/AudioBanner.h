#pragma once
#include <QWidget>

class AudioBanner : public QWidget {
    Q_OBJECT
public:
    explicit AudioBanner(QWidget* parent = nullptr);
    void showMessage(const QString& msg);
};

#pragma once
#include <QWidget>

class PublicServerListWidget : public QWidget {
    Q_OBJECT
public:
    explicit PublicServerListWidget(QWidget *parent = nullptr);

signals:
    void backRequested();
    void connectToPublic(int index);

private:
    void setupUi();
};

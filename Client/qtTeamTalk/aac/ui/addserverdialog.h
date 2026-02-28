#pragma once
#include <QDialog>

class QLineEdit;

class AddServerDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddServerDialog(QWidget *parent = nullptr);

    QString label() const;
    QString host() const;
    int port() const;

private:
    QLineEdit *m_label;
    QLineEdit *m_host;
    QLineEdit *m_port;

    void setupUi();
};

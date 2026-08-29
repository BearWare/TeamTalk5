#pragma once
#include <QWidget>

class QLabel;
class QPushButton;

class ConnectionView : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionView(QWidget *parent = nullptr);

    void setServerLabel(const QString &label);
    void setStatusText(const QString &text);

signals:
    void cancelRequested();
    void retryRequested();

private:
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QPushButton *m_cancelButton;
    QPushButton *m_retryButton;

    void setupUi();
    void setupConnections();
};

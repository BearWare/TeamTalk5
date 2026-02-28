#pragma once

#include <QPushButton>
#include <QElapsedTimer>
#include <QTimer>

class DeepWellButton : public QPushButton {
    Q_OBJECT
public:
    explicit DeepWellButton(const QString& text, QWidget* parent = nullptr);

    void setHoldDurationMs(int ms);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* e) override;

private slots:
    void onTick();

private:
    int m_holdMs = 800;
    QElapsedTimer m_elapsed;
    QTimer m_timer;
    bool m_inside = false;
};

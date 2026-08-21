#pragma once

#include <QWidget>
#include <QVector>

class QPushButton;
class QHBoxLayout;

struct PredictiveAction {
    QString label;
    QString id; // e.g. "reconnect", "refresh", "join-most-active"
};

class PredictiveStrip : public QWidget {
    Q_OBJECT
public:
    explicit PredictiveStrip(QWidget* parent = nullptr);

    void setActions(const QVector<PredictiveAction>& actions);

signals:
    void actionTriggered(const QString& id);

private:
    QHBoxLayout* m_layout = nullptr;
};

#pragma once
#include <QObject>

class StateMachine;

class UiStateAdapter : public QObject {
    Q_OBJECT
public:
    explicit UiStateAdapter(StateMachine *machine, QObject *parent = nullptr);

signals:
    void stateChanged();

private:
    StateMachine *m_machine;
};

#include "ui_state_adapter.h"
#include "state_machine.h"

UiStateAdapter::UiStateAdapter(StateMachine *machine, QObject *parent)
    : QObject(parent),
      m_machine(machine)
{
    // Connect state machine signals to UI signals
}

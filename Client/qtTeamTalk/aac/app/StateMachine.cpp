#include "StateMachine.h"

// This file intentionally remains minimal.
// All logic is already implemented inline in StateMachine.h.
// We keep this translation unit so the build system has a .cpp to compile.

StateMachine::StateMachine(QObject* parent)
    : QObject(parent)
{
    // No additional initialization required.
}

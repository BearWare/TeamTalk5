#pragma once

#include "AACScreen.h"

class QGridLayout;
class AACKeyButton;

class AACKeyboardScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACKeyboardScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

private:
    void buildKeyboard();
    void addRow(const QStringList& keys, int row);

    QGridLayout* m_grid = nullptr;
};

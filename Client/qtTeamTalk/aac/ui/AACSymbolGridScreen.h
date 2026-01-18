#pragma once

#include "AACScreen.h"

class QGridLayout;
class AACSymbolButton;

class AACSymbolGridScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACSymbolGridScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

private:
    void buildGrid();

    QGridLayout* m_grid = nullptr;
};

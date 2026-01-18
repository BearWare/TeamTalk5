#include "AACKeyboardScreen.h"
#include "AACKeyButton.h"
#include <QGridLayout>

AACKeyboardScreen::AACKeyboardScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : AACScreen(mgr, parent)
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);
    buildKeyboard();
}

void AACKeyboardScreen::buildKeyboard()
{
    addRow({"Q","W","E","R","T","Y","U","I","O","P"}, 0);
    addRow({"A","S","D","F","G","H","J","K","L"}, 1);
    addRow({"Z","X","C","V","B","N","M"}, 2);

    // Space bar
    AACKeyButton* space = new AACKeyButton(" ", m_aac, this);
    m_grid->addWidget(space, 3, 0, 1, 10);
    registerInteractive(space, true);
}

void AACKeyboardScreen::addRow(const QStringList& keys, int row)
{
    int col = 0;
    for (const QString& k : keys) {
        AACKeyButton* btn = new AACKeyButton(k, m_aac, this);
        m_grid->addWidget(btn, row, col++);
        registerInteractive(btn, true);
    }
}

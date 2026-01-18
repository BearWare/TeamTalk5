#include "AACSymbolGridScreen.h"
#include "AACSymbolButton.h"
#include <QGridLayout>

AACSymbolGridScreen::AACSymbolGridScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : AACScreen(mgr, parent)
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);
    buildGrid();
}

void AACSymbolGridScreen::buildGrid()
{
    // Example core vocabulary
    QStringList words = {"I", "want", "go", "stop", "help", "you", "yes", "no"};

    int row = 0, col = 0;
    for (const QString& w : words) {
        AACSymbolButton* btn = new AACSymbolButton(w, m_aac, this);
        m_grid->addWidget(btn, row, col++);
        registerInteractive(btn, true);

        if (col >= 4) {
            col = 0;
            row++;
        }
    }
}

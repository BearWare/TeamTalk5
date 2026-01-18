#include "AACCategoryScreen.h"
#include "AACCategoryButton.h"
#include <QGridLayout>

AACCategoryScreen::AACCategoryScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : AACScreen(mgr, parent)
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);
    buildCategories();
}

void AACCategoryScreen::buildCategories()
{
    QStringList cats = {"People", "Food", "Places", "Actions", "Feelings", "Things"};

    int row = 0, col = 0;
    for (const QString& c : cats) {
        AACCategoryButton* btn = new AACCategoryButton(c, m_aac, this);
        m_grid->addWidget(btn, row, col++);
        registerInteractive(btn, true);

        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

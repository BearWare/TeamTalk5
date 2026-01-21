#include "AACCategoryScreen.h"
#include "AACCategoryButton.h"
#include "AACVocabularyManager.h"

#include <QGridLayout>

AACCategoryScreen::AACCategoryScreen(AACAccessibilityManager* mgr,
                                     AACVocabularyManager* vocab,
                                     QWidget* parent)
    : AACScreen(mgr, parent),
      m_vocab(vocab)
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);

    buildCategories();

    // React to live updates
    connect(m_vocab, &AACVocabularyManager::categoriesChanged,
            this, &AACCategoryScreen::buildCategories);
}

void AACCategoryScreen::clearCategories()
{
    QLayoutItem* item;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
}

void AACCategoryScreen::buildCategories()
{
    clearCategories();

    QStringList cats = m_vocab->categories();

    int row = 0, col = 0;
    for (const QString& c : cats) {
        AACCategoryButton* btn = new AACCategoryButton(c, m_aac, this);

connect(btn, &AACCategoryButton::clicked,
        this, [this, c]() {
            emit categorySelected(c);
        });
        m_grid->addWidget(btn, row, col++);
        registerInteractive(btn, true);

        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

#include "AACSymbolGridScreen.h"
#include "AACSymbolButton.h"
#include "AACVocabularyManager.h"

#include <QGridLayout>

AACSymbolGridScreen::AACSymbolGridScreen(AACAccessibilityManager* mgr,
                                         AACVocabularyManager* vocab,
                                         QWidget* parent)
    : AACScreen(mgr, parent),
      m_vocab(vocab)
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);

    // React to live vocabulary updates
    connect(m_vocab, &AACVocabularyManager::vocabularyChanged,
            this, &AACSymbolGridScreen::buildSymbols);
}

void AACSymbolGridScreen::setCategory(const QString& category)
{
    m_category = category;
    buildSymbols();
}

void AACSymbolGridScreen::clearSymbols()
{
    QLayoutItem* item;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
}

void AACSymbolGridScreen::buildSymbols()
{
    clearSymbols();

    if (m_category.isEmpty())
        return;

    QVector<AACVocabItem> items = m_vocab->wordsInCategory(m_category);

    int row = 0, col = 0;
    for (const AACVocabItem& item : items) {

        AACSymbolButton* btn = new AACSymbolButton(
            item.label,
            item.iconPath,
            m_aac,
            this
        );

        // If your button supports IDs, this is where you'd pass it
        // btn->setWordId(item.id);

        m_grid->addWidget(btn, row, col++);
        registerInteractive(btn, true);

        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

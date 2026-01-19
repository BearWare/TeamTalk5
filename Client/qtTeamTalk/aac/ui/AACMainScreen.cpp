#include "AACMainScreen.h"

#include "AACAccessibilityManager.h"
#include "AACTextBar.h"
#include "PredictiveStrip.h"
#include "AACKeyboardScreen.h"
#include "AACSymbolGridScreen.h"
#include "AACCategoryScreen.h"
#include "AACVocabularyManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>

AACMainScreen::AACMainScreen(AACAccessibilityManager* mgr,
                             AACVocabularyManager* vocab,
                             QWidget* parent)
    : QWidget(parent),
      m_mgr(mgr),
      m_vocab(vocab)
{
    buildUi();
    connectSignals();
}

void AACMainScreen::buildUi()
{
    auto* root = new QVBoxLayout(this);

    // Top text bar + predictive strip
    m_textBar = new AACTextBar(m_mgr, this);
    m_predictive = new PredictiveStrip(this);

    root->addWidget(m_textBar);
    root->addWidget(m_predictive);

    // Navigation buttons
    auto* navRow = new QHBoxLayout();
    m_keyboardBtn   = new QPushButton("Keyboard", this);
    m_symbolsBtn    = new QPushButton("Symbols", this);
    m_categoriesBtn = new QPushButton("Categories", this);

    navRow->addWidget(m_keyboardBtn);
    navRow->addWidget(m_symbolsBtn);
    navRow->addWidget(m_categoriesBtn);

    root->addLayout(navRow);

    // Stacked widget for the three AAC screens
    m_stack = new QStackedWidget(this);

    m_keyboardScreen = new AACKeyboardScreen(m_mgr, this);
    m_symbolScreen   = new AACSymbolGridScreen(m_mgr, m_vocab, this);
    m_categoryScreen = new AACCategoryScreen(m_mgr, m_vocab, this);

    m_stack->addWidget(m_keyboardScreen);   // index 0
    m_stack->addWidget(m_symbolScreen);     // index 1
    m_stack->addWidget(m_categoryScreen);   // index 2

    root->addWidget(m_stack);

    // Default view
    switchToCategories();
}

void AACMainScreen::connectSignals()
{
    connect(m_keyboardBtn,   &QPushButton::clicked,
            this,            &AACMainScreen::switchToKeyboard);

    connect(m_symbolsBtn,    &QPushButton::clicked,
            this,            &AACMainScreen::switchToSymbols);

    connect(m_categoriesBtn, &QPushButton::clicked,
            this,            &AACMainScreen::switchToCategories);

    // When a category is selected, show the symbol grid
connect(m_categoryScreen, &AACCategoryScreen::categorySelected,
        this, [this](const QString& cat) {
            m_symbolScreen->setCategory(cat);
            switchToSymbols();
        });

connect(m_symbolScreen, &AACSymbolGridScreen::wordSelected,
        this, [this](const QString& word) {
            m_textBar->appendWord(word);
        });

}

void AACMainScreen::switchToKeyboard()
{
    m_stack->setCurrentWidget(m_keyboardScreen);
}

void AACMainScreen::switchToSymbols()
{
    m_stack->setCurrentWidget(m_symbolScreen);
}

void AACMainScreen::switchToCategories()
{
    m_stack->setCurrentWidget(m_categoryScreen);
}

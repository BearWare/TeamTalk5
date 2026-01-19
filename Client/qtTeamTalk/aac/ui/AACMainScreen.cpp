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
    //
    // KEYBOARD → TEXT BAR
    //
    connect(m_keyboardScreen, &AACKeyboardScreen::characterTyped,
            m_textBar,        &AACTextBar::insertCharacter);

    connect(m_keyboardScreen, &AACKeyboardScreen::backspaceRequested,
            m_textBar,        &AACTextBar::backspace);

    connect(m_keyboardScreen, &AACKeyboardScreen::moveCursorLeft,
            m_textBar,        &AACTextBar::moveCursorLeft);

    connect(m_keyboardScreen, &AACKeyboardScreen::moveCursorRight,
            m_textBar,        &AACTextBar::moveCursorRight);

    connect(m_keyboardScreen, &AACKeyboardScreen::spaceRequested,
            m_textBar,        &AACTextBar::insertSpace);


    //
    // TEXT BAR → KEYBOARD (keep keyboard stateful)
    //
    connect(m_textBar, &AACTextBar::textChanged,
            m_keyboardScreen, &AACKeyboardScreen::setText);


    //
    // TEXT BAR → PREDICTIVE STRIP
    //
    connect(m_textBar, &AACTextBar::textChanged,
            m_predictive, &PredictiveStrip::setContext);


    //
    // PREDICTIVE STRIP → TEXT BAR
    //
    connect(m_predictive, &PredictiveStrip::suggestionChosen,
            m_textBar,     &AACTextBar::appendWord);


    //
    // PREDICTIVE STRIP → KEYBOARD (optional adaptive behaviour)
    //
    connect(m_predictive, &PredictiveStrip::suggestionChosen,
            m_keyboardScreen, &AACKeyboardScreen::onSuggestionAccepted);


    //
    // KEYBOARD → PREDICTIVE STRIP (update predictions as user types)
    //
    connect(m_keyboardScreen, &AACKeyboardScreen::characterTyped,
            m_predictive,      &PredictiveStrip::onCharacterTyped);


    //
    // CATEGORY SCREEN → MAIN SCREEN (navigation)
    //
    connect(m_categoryScreen, &AACCategoryScreen::categorySelected,
            this,             &AACMainScreen::onCategorySelected);


    //
    // SYMBOL GRID → TEXT BAR (insert chosen word)
    //
    connect(m_symbolScreen, &AACSymbolGridScreen::wordChosen,
            m_textBar,       &AACTextBar::appendWord);


    //
    // SYMBOL GRID → MAIN SCREEN (navigation back to categories)
    //
    connect(m_symbolScreen, &AACSymbolGridScreen::backRequested,
            this,           &AACMainScreen::switchToCategories);


    //
    // NAVIGATION BUTTONS (AACButtons)
    //
    connect(m_btnKeyboard,   &QPushButton::clicked,
            this,            &AACMainScreen::switchToKeyboard);

    connect(m_btnSymbols,    &QPushButton::clicked,
            this,            &AACMainScreen::switchToSymbols);

    connect(m_btnCategories, &QPushButton::clicked,
            this,            &AACMainScreen::switchToCategories);


    //
    // VOCABULARY UPDATES
    //
    connect(m_vocab, &AACVocabularyManager::categoriesChanged,
            m_categoryScreen, &AACCategoryScreen::reloadCategories);

    connect(m_vocab, &AACVocabularyManager::vocabularyChanged,
            m_symbolScreen,   &AACSymbolGridScreen::reloadSymbols);
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

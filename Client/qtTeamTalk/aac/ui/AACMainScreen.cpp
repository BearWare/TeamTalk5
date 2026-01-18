#include "AACMainScreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>

#include "AACTextBar.h"
#include "AACKeyboardScreen.h"
#include "AACSymbolGridScreen.h"
#include "AACCategoryScreen.h"
#include "PredictiveStrip.h" // your widget
#include "aac/aac/AACFramework.h"

AACMainScreen::AACMainScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    buildUi();
    connectSignals();
}

void AACMainScreen::buildUi()
{
    auto* root = new QVBoxLayout(this);

    // Top: Text bar
    m_textBar = new AACTextBar(m_mgr, this);
    root->addWidget(m_textBar);

    // Predictive strip (your existing widget)
    m_predictive = new PredictiveStrip(this);
    root->addWidget(m_predictive);

    // Stacked widget for AAC screens
    m_stack = new QStackedWidget(this);

    m_keyboardScreen = new AACKeyboardScreen(m_mgr, this);
    m_symbolScreen = new AACSymbolGridScreen(m_mgr, this);
    m_categoryScreen = new AACCategoryScreen(m_mgr, this);

    m_stack->addWidget(m_keyboardScreen);
    m_stack->addWidget(m_symbolScreen);
    m_stack->addWidget(m_categoryScreen);

    root->addWidget(m_stack, 1);

    // Navigation bar
    auto* nav = new QHBoxLayout();

    m_keyboardBtn = new QPushButton(tr("Keyboard"), this);
    m_symbolsBtn = new QPushButton(tr("Symbols"), this);
    m_categoriesBtn = new QPushButton(tr("Categories"), this);

    nav->addWidget(m_keyboardBtn);
    nav->addWidget(m_symbolsBtn);
    nav->addWidget(m_categoriesBtn);

    root->addLayout(nav);

    setLayout(root);

    // Default screen
    switchToKeyboard();
}

void AACMainScreen::connectSignals()
{
    connect(m_keyboardBtn, &QPushButton::clicked,
            this, &AACMainScreen::switchToKeyboard);

    connect(m_symbolsBtn, &QPushButton::clicked,
            this, &AACMainScreen::switchToSymbols);

    connect(m_categoriesBtn, &QPushButton::clicked,
            this, &AACMainScreen::switchToCategories);
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

#pragma once

#include <QWidget>
#include <QPointer>

class QStackedWidget;
class QPushButton;
class AACAccessibilityManager;
class AACTextBar;
class AACKeyboardScreen;
class AACSymbolGridScreen;
class AACCategoryScreen;
class AACVocabularyManager;
class PredictiveStrip;

class AACMainScreen : public QWidget {
    Q_OBJECT
public:
    explicit AACMainScreen(AACAccessibilityManager* mgr,
                           AACVocabularyManager* vocab,
                           QWidget* parent = nullptr);

private:
    void buildUi();
    void connectSignals();
    void switchToKeyboard();
    void switchToSymbols();
    void switchToCategories();

    AACAccessibilityManager* m_mgr = nullptr;
    AACVocabularyManager* m_vocab = nullptr;

    AACTextBar* m_textBar = nullptr;
    PredictiveStrip* m_predictive = nullptr;

    QStackedWidget* m_stack = nullptr;

    AACKeyboardScreen* m_keyboardScreen = nullptr;
    AACSymbolGridScreen* m_symbolScreen = nullptr;
    AACCategoryScreen* m_categoryScreen = nullptr;

    QPushButton* m_keyboardBtn = nullptr;
    QPushButton* m_symbolsBtn = nullptr;
    QPushButton* m_categoriesBtn = nullptr;
};

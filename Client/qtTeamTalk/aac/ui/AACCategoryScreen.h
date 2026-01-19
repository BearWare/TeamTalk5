#pragma once

#include "AACScreen.h"

class QGridLayout;
class AACCategoryButton;

class AACVocabularyManager;

class AACCategoryScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACCategoryScreen(AACAccessibilityManager* mgr,
                               AACVocabularyManager* vocab,
                               QWidget* parent = nullptr);

private:
    void buildCategories();
    void clearCategories();

    QGridLayout* m_grid = nullptr;
    AACVocabularyManager* m_vocab = nullptr;
};

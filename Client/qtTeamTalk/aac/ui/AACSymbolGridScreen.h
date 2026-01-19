#pragma once

#include "AACScreen.h"

class QGridLayout;
class AACSymbolButton;
class AACVocabularyManager;

class AACSymbolGridScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACSymbolGridScreen(AACAccessibilityManager* mgr,
                                 AACVocabularyManager* vocab,
                                 QWidget* parent = nullptr);

    void setCategory(const QString& category);

signals:
    void wordSelected(const QString& word);

private:
    void buildSymbols();
    void clearSymbols();

    QString m_category;
    QGridLayout* m_grid = nullptr;
    AACVocabularyManager* m_vocab = nullptr;
};

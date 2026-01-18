#pragma once

#include "AACScreen.h"

class QGridLayout;
class AACCategoryButton;

class AACCategoryScreen : public AACScreen {
    Q_OBJECT
public:
    explicit AACCategoryScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

private:
    void buildCategories();

    QGridLayout* m_grid = nullptr;
};

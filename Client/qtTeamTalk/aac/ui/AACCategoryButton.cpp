#include "AACCategoryButton.h"
#include <QIcon>

AACCategoryButton::AACCategoryButton(const QString& categoryName,
                                     const QString& iconPath,
                                     AACAccessibilityManager* aac,
                                     QWidget* parent)
    : QPushButton(parent)
    , m_aac(aac)
    , m_categoryName(categoryName)
    , m_iconPath(iconPath)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(120, 120);

    setIcon(QIcon(iconPath));
    setIconSize(QSize(64, 64));
    setText(categoryName);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    connect(this, &QPushButton::clicked,
            this, &AACCategoryButton::handleClick);
}

void AACCategoryButton::handleClick()
{
    emit categorySelected(m_categoryName);
}

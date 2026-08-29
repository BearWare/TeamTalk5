#include "AACSymbolButton.h"
#include "aac/aac/AACFramework.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

AACSymbolButton::AACSymbolButton(const QString& label,
                                 const QString& iconPath,
                                 AACAccessibilityManager* aac,
                                 QWidget* parent)
    : QPushButton(parent)
    , m_aac(aac)
    , m_label(label)
    , m_iconPath(iconPath)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100);

    setIcon(QIcon(iconPath));
    setIconSize(QSize(64, 64));
    setText(label);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    connect(this, &QPushButton::clicked,
            this, &AACSymbolButton::handleClick);
}

void AACSymbolButton::handleClick()
{
    emit symbolActivated(m_label);

    if (m_aac) {
        m_aac->inputController()->insertText(m_label + " ");
        m_aac->speechEngine()->speak(m_label);
    }
}

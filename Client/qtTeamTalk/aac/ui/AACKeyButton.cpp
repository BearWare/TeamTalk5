#include "AACKeyButton.h"
#include "aac/aac/AACFramework.h"

AACKeyButton::AACKeyButton(const QString& text,
                           AACAccessibilityManager* aac,
                           QWidget* parent)
    : QPushButton(text, parent)
    , m_aac(aac)
    , m_keyText(text)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(60, 60);

    connect(this, &QPushButton::clicked,
            this, &AACKeyButton::handleClick);
}

void AACKeyButton::handleClick()
{
    emit keyActivated(m_keyText);

    if (m_aac)
        m_aac->inputController()->insertText(m_keyText);
}

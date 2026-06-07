#include "AACSymbolGridScreen.h"

#include <QPushButton>

AACSymbolGridScreen::AACSymbolGridScreen(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
    m_rootLayout = new QGridLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    if (m_aac)
        m_currentCategory = m_aac->activeCategory();

    rebuildGrid();
}

QList<QWidget*> AACSymbolGridScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    for (AACButton* b : m_buttons)
        out << b;
    return out;
}

QList<QWidget*> AACSymbolGridScreen::primaryWidgets() const
{
    QList<QWidget*> out;
    for (AACButton* b : m_buttons)
        out << b;
    return out;
}

QLayout* AACSymbolGridScreen::rootLayout() const
{
    return m_rootLayout;
}

void AACSymbolGridScreen::setCategory(const QString& category)
{
    if (m_currentCategory == category)
        return;
    m_currentCategory = category;
    rebuildGrid();
}

void AACSymbolGridScreen::rebuildGrid()
{
    // clear
    QLayoutItem* item = nullptr;
    while ((item = m_rootLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }
    m_buttons.clear();

    if (!m_aac || !m_aac->vocabularyManager())
        return;

    const auto symbols = m_aac->vocabularyManager()->symbolsForCategory(m_currentCategory);
    int row = 0, col = 0;
    const int maxCols = 4;

    for (const auto& sym : symbols) {
        QString word = sym.word; // adjust to your symbol type

        AACButton* btn = new AACButton(m_aac, this);
        btn->setText(word);
        btn->setDeepWell(m_aac->modes().deepWells);

        connect(btn, &QPushButton::clicked,
                this, &AACSymbolGridScreen::onSymbolClicked);

        m_rootLayout->addWidget(btn, row, col);
        m_buttons << btn;

        if (++col >= maxCols) {
            col = 0;
            ++row;
        }
    }
}

void AACSymbolGridScreen::onSymbolClicked()
{
    if (!m_aac)
        return;

    AACButton* btn = qobject_cast<AACButton*>(sender());
    if (!btn)
        return;

    const QString word = btn->text();

    if (m_aac->predictionEngine())
        m_aac->predictionEngine()->setLastSymbolWord(word);

    emit symbolActivated(word);
}

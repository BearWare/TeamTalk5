#include "AACSymbolGridScreen.h"

#include <QPushButton>

AACSymbolGridScreen::AACSymbolGridScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    m_rootLayout = new QGridLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    if (m_mgr)
        m_currentCategory = m_mgr->activeCategory();

    rebuildGrid();
}

QList<QWidget*> AACSymbolGridScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    for (AACButton* b : m_symbolButtons)
        out << b;
    return out;
}

QList<QWidget*> AACSymbolGridScreen::primaryWidgets() const
{
    QList<QWidget*> out;
    for (AACButton* b : m_symbolButtons)
        out << b;
    return out;
}

QLayout* AACSymbolGridScreen::rootLayout() const
{
    return m_rootLayout;
}

QWidget* AACSymbolGridScreen::predictiveStripContainer() const
{
    return nullptr;
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
    // Clear existing buttons
    QLayoutItem* item = nullptr;
    while ((item = m_rootLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }
    m_symbolButtons.clear();

    if (!m_mgr || !m_mgr->vocabularyManager())
        return;

    const auto symbols =
        m_mgr->vocabularyManager()->symbolsForCategory(m_currentCategory);
    // Assuming symbolsForCategory returns a list of objects with .word or QStrings.
    // If it's QStringList, adjust accordingly.

    int row = 0;
    int col = 0;
    const int maxCols = 4;

    for (const auto& sym : symbols) {
        QString word;
        if constexpr (std::is_same_v<decltype(sym), QString>) {
            word = sym;
        } else {
            word = sym.word; // adjust to your actual symbol type
        }

        AACButton* btn = new AACButton(m_mgr, this);
        btn->setText(word);
        btn->setDeepWell(m_mgr->modes().deepWells);

        connect(btn, &QPushButton::clicked,
                this, &AACSymbolGridScreen::onSymbolClicked);

        m_rootLayout->addWidget(btn, row, col);
        m_symbolButtons << btn;

        ++col;
        if (col >= maxCols) {
            col = 0;
            ++row;
        }
    }
}

void AACSymbolGridScreen::onSymbolClicked()
{
    if (!m_mgr)
        return;

    AACButton* btn = qobject_cast<AACButton*>(sender());
    if (!btn)
        return;

    const QString word = btn->text();

    if (m_mgr->predictionEngine())
        m_mgr->predictionEngine()->setLastSymbolWord(word);

    emit symbolActivated(word);
}

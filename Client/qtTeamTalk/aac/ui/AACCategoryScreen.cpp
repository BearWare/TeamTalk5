#include "AACCategoryScreen.h"

AACCategoryScreen::AACCategoryScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : QWidget(parent)
    , m_mgr(mgr)
{
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    m_rootLayout->addWidget(m_list);
    m_rootLayout->addStretch(1);

    if (m_mgr && m_mgr->vocabularyManager()) {
        const QStringList cats = m_mgr->vocabularyManager()->categories();
        for (const QString& c : cats)
            m_list->addItem(c);
    }

    connect(m_list, &QListWidget::itemClicked,
            this, &AACCategoryScreen::onCategoryClicked);
}

QList<QWidget*> AACCategoryScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<QListWidget*>(m_list);
    return out;
}

QList<QWidget*> AACCategoryScreen::primaryWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<QListWidget*>(m_list);
    return out;
}

QLayout* AACCategoryScreen::rootLayout() const
{
    return m_rootLayout;
}

QWidget* AACCategoryScreen::predictiveStripContainer() const
{
    return nullptr;
}

void AACCategoryScreen::onCategoryClicked(QListWidgetItem* item)
{
    if (!item || !m_mgr)
        return;

    const QString category = item->text();
    m_mgr->setActiveCategory(category);
    emit categoryChosen(category);
}

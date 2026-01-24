#include "AACCategoryScreen.h"

#include <QAbstractItemView>

AACCategoryScreen::AACCategoryScreen(AACAccessibilityManager* aac, QWidget* parent)
    : QWidget(parent)
    , m_aac(aac)
{
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_rootLayout->addWidget(m_list);
    m_rootLayout->addStretch(1);

    if (m_aac && m_aac->vocabularyManager()) {
        const QStringList cats = m_aac->vocabularyManager()->categories();
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

void AACCategoryScreen::onCategoryClicked(QListWidgetItem* item)
{
    if (!item || !m_aac)
        return;

    const QString category = item->text();
    m_aac->setActiveCategory(category); // forwards to prediction engine
    emit categoryChosen(category);
}

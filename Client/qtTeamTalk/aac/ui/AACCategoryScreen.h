#pragma once

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QString>

#include "AACFramework.h" // for AACScreenAdapter, AACAccessibilityManager

/**
 * AACCategoryScreen
 *
 * Shows AAC vocabulary categories.
 * Selecting a category:
 *  - Updates AACAccessibilityManager::activeCategory
 *  - Updates AACPredictionEngine category context via setActiveCategory()
 */
class AACCategoryScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit AACCategoryScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

    // AACScreenAdapter
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override;

signals:
    void categoryChosen(const QString& category);

private slots:
    void onCategoryClicked(QListWidgetItem* item);

private:
    AACAccessibilityManager* m_mgr = nullptr;

    QVBoxLayout*  m_rootLayout = nullptr;
    QListWidget*  m_list = nullptr;
};

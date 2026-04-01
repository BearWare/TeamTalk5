#pragma once

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>

#include "aac/AACFramework.h" // AACScreenAdapter, AACAccessibilityManager

class AACCategoryScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit AACCategoryScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    // AACScreenAdapter
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override { return nullptr; }

signals:
    void categoryChosen(const QString& category);

private slots:
    void onCategoryClicked(QListWidgetItem* item);

private:
    AACAccessibilityManager* m_aac = nullptr;
    QVBoxLayout* m_rootLayout = nullptr;
    QListWidget* m_list = nullptr;
};

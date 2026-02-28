#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QList>

#include "aac/AACFramework.h" // AACScreenAdapter, AACAccessibilityManager, AACButton

class AACSymbolGridScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit AACSymbolGridScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    // AACScreenAdapter
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override { return nullptr; }

signals:
    void symbolActivated(const QString& word);

public slots:
    void setCategory(const QString& category);

private slots:
    void onSymbolClicked();

private:
    AACAccessibilityManager* m_aac = nullptr;
    QGridLayout* m_rootLayout = nullptr;
    QString m_currentCategory;
    QList<AACButton*> m_buttons;

    void rebuildGrid();
};

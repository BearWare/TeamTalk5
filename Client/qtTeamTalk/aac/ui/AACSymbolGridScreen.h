#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QList>
#include <QString>

#include "AACFramework.h" // for AACScreenAdapter, AACAccessibilityManager
#include "AACFramework.h" // AACButton is declared there in your project

class AACSymbolGridScreen : public QWidget, public AACScreenAdapter
{
    Q_OBJECT
public:
    explicit AACSymbolGridScreen(AACAccessibilityManager* mgr, QWidget* parent = nullptr);

    // AACScreenAdapter
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override;

signals:
    void symbolActivated(const QString& word);

public slots:
    void setCategory(const QString& category);

private slots:
    void onSymbolClicked();

private:
    AACAccessibilityManager* m_mgr = nullptr;

    QGridLayout* m_rootLayout = nullptr;
    QString      m_currentCategory;

    QList<AACButton*> m_symbolButtons;

    void rebuildGrid();
};

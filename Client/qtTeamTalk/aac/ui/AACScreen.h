#pragma once

#include <QWidget>
#include <QPointer>
#include <QList>
#include <QEvent>

#include "aac/aac/AACFramework.h"

// Base class for all AAC-aware screens.
// Acts as an adapter to the AAC framework.
class AACScreen : public QWidget, public AACScreenAdapter {
    Q_OBJECT
public:
    explicit AACScreen(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    // AACScreenAdapter interface
    QList<QWidget*> interactiveWidgets() const override;
    QList<QWidget*> primaryWidgets() const override;
    QLayout* rootLayout() const override;
    QWidget* predictiveStripContainer() const override { return nullptr; }

protected:
    AACAccessibilityManager* m_aac = nullptr;

    QList<QWidget*> m_interactive;
    QList<QWidget*> m_primary;

    // Register an interactive widget (optionally primary for Ultra-Minimal Mode)
    void registerInteractive(QWidget* w, bool primary = false);

    bool eventFilter(QObject* obj, QEvent* event) override;

    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
};

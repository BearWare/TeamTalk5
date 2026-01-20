#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

class AACAccessibilityManager;

class AACPredictiveStrip : public QWidget
{
    Q_OBJECT
public:
    explicit AACPredictiveStrip(AACAccessibilityManager* aac,
                                QLineEdit* targetEdit,
                                QWidget* parent = nullptr);

public slots:
    void refresh(const QString& text);

private:
    AACAccessibilityManager* m_aac = nullptr;
    QLineEdit* m_targetEdit = nullptr;
    QHBoxLayout* m_layout = nullptr;
    QList<QPushButton*> m_buttons;

    void applySuggestion(const QString& suggestion);
};

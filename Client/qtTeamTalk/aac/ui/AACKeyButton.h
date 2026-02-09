#pragma once

#include <QPushButton>
#include <QString>

class AACAccessibilityManager;

class AACKeyButton : public QPushButton {
    Q_OBJECT
public:
    AACKeyButton(const QString& text,
                 AACAccessibilityManager* aac,
                 QWidget* parent = nullptr);

signals:
    void keyActivated(const QString& text);

private slots:
    void handleClick();

private:
    AACAccessibilityManager* m_aac = nullptr;
    QString m_keyText;
};

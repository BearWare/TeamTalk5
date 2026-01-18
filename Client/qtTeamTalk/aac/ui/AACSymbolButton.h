#pragma once

#include <QPushButton>
#include <QString>

class AACAccessibilityManager;

class AACSymbolButton : public QPushButton {
    Q_OBJECT
public:
    AACSymbolButton(const QString& label,
                    const QString& iconPath,
                    AACAccessibilityManager* aac,
                    QWidget* parent = nullptr);

signals:
    void symbolActivated(const QString& label);

private slots:
    void handleClick();

private:
    AACAccessibilityManager* m_aac = nullptr;
    QString m_label;
    QString m_iconPath;
};

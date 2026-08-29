#pragma once

#include <QPushButton>
#include <QString>

class AACAccessibilityManager;

class AACCategoryButton : public QPushButton {
    Q_OBJECT
public:
    AACCategoryButton(const QString& categoryName,
                      const QString& iconPath,
                      AACAccessibilityManager* aac,
                      QWidget* parent = nullptr);

signals:
    void categorySelected(const QString& categoryName);

private slots:
    void handleClick();

private:
    AACAccessibilityManager* m_aac = nullptr;
    QString m_categoryName;
    QString m_iconPath;
};

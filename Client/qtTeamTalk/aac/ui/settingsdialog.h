#pragma once
#include <QDialog>

class QTabWidget;
class QWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    QTabWidget *m_tabs;

    QWidget *createAudioTab();
    QWidget *createInputTab();
    QWidget *createAppearanceTab();
    QWidget *createAccessibilityTab();

    void setupUi();
};

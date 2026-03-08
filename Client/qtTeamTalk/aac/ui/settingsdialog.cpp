#include "settingsdialog.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void SettingsDialog::setupUi() {
    auto *layout = new QVBoxLayout(this);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(createAudioTab(), tr("Audio"));
    m_tabs->addTab(createInputTab(), tr("Input"));
    m_tabs->addTab(createAppearanceTab(), tr("Appearance"));
    m_tabs->addTab(createAccessibilityTab(), tr("Accessibility"));

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    layout->addWidget(m_tabs);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QWidget *SettingsDialog::createAudioTab() {
    auto *w = new QWidget(this);
    auto *lay = new QVBoxLayout(w);
    lay->addWidget(new QLabel(tr("Audio settings (placeholder)."), w));
    lay->addStretch();
    return w;
}

QWidget *SettingsDialog::createInputTab() {
    auto *w = new QWidget(this);
    auto *lay = new QVBoxLayout(w);
    lay->addWidget(new QLabel(tr("Input settings (placeholder)."), w));
    lay->addStretch();
    return w;
}

QWidget *SettingsDialog::createAppearanceTab() {
    auto *w = new QWidget(this);
    auto *lay = new QVBoxLayout(w);
    lay->addWidget(new QLabel(tr("Appearance settings (placeholder)."), w));
    lay->addStretch();
    return w;
}

QWidget *SettingsDialog::createAccessibilityTab() {
    auto *w = new QWidget(this);
    auto *lay = new QVBoxLayout(w);
    lay->addWidget(new QLabel(tr("Accessibility settings (placeholder)."), w));
    lay->addStretch();
    return w;
}

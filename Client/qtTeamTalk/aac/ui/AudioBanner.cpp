#include "AudioBanner.h"
#include <QLabel>
#include <QVBoxLayout>

AudioBanner::AudioBanner(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background: #333; color: white;");
    setFixedHeight(40);

    auto layout = new QVBoxLayout(this);
    auto label = new QLabel(this);
    label->setObjectName("bannerLabel");
    layout->addWidget(label);
}

void AudioBanner::showMessage(const QString& msg)
{
    auto label = findChild<QLabel*>("bannerLabel");
    label->setText(msg);
    show();
}

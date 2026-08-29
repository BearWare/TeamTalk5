#include "publicserverlistwidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

PublicServerListWidget::PublicServerListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void PublicServerListWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Public Servers (placeholder)", this));

    auto *backBtn = new QPushButton("Back", this);
    layout->addWidget(backBtn);

    connect(backBtn, &QPushButton::clicked, this, &PublicServerListWidget::backRequested);
}

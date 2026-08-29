#include "connectionview.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ConnectionView::ConnectionView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();
}

void ConnectionView::setupUi() {
    auto *layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(tr("Connecting…"), this);
    m_statusLabel = new QLabel(tr("Connecting to server…"), this);

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_retryButton = new QPushButton(tr("Retry"), this);
    m_retryButton->setEnabled(false);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_statusLabel);
    layout->addStretch();
    layout->addWidget(m_retryButton);
    layout->addWidget(m_cancelButton);
}

void ConnectionView::setupConnections() {
    connect(m_cancelButton, &QPushButton::clicked,
            this, &ConnectionView::cancelRequested);
    connect(m_retryButton, &QPushButton::clicked,
            this, &ConnectionView::retryRequested);
}

void ConnectionView::setServerLabel(const QString &label) {
    m_titleLabel->setText(tr("Connecting to %1…").arg(label));
}

void ConnectionView::setStatusText(const QString &text) {
    m_statusLabel->setText(text);
}

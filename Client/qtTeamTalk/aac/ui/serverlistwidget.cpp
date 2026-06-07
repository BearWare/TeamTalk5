#include "serverlistwidget.h"
#include "serverlistmodel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListView>

ServerListWidget::ServerListWidget(QWidget *parent)
    : QWidget(parent),
      m_model(new ServerListModel(this))
{
    setupUi();
    setupConnections();
}

void ServerListWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);

    auto *list = new QListView(this);
    list->setModel(m_model);

    auto *addBtn = new QPushButton("Add Server", this);
    auto *publicBtn = new QPushButton("Public Servers", this);
    auto *connectBtn = new QPushButton("Connect", this);

    layout->addWidget(list);
    layout->addWidget(addBtn);
    layout->addWidget(publicBtn);
    layout->addWidget(connectBtn);
}

void ServerListWidget::setupConnections() {
    // Placeholder signals
}

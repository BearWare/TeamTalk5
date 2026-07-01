#include "addserverdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>

AddServerDialog::AddServerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void AddServerDialog::setupUi() {
    auto *layout = new QFormLayout(this);

    m_label = new QLineEdit(this);
    m_host = new QLineEdit(this);
    m_port = new QLineEdit(this);

    layout->addRow("Label:", m_label);
    layout->addRow("Host:", m_host);
    layout->addRow("Port:", m_port);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString AddServerDialog::label() const { return m_label->text(); }
QString AddServerDialog::host() const { return m_host->text(); }
int AddServerDialog::port() const { return m_port->text().toInt(); }

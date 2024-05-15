/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "custominputdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCursor>

CustomInputDialog::CustomInputDialog(const QString& title, const QString& labelText, const QString& initialText, const QHash<QString, QString>& variables, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(labelText, this);
    m_lineEdit = new QLineEdit(initialText, this);

    label->setBuddy(m_lineEdit);
    mainLayout->addWidget(label);
    mainLayout->addWidget(m_lineEdit);

    m_variableButton = new QPushButton(tr("Variables..."), this);
    mainLayout->addWidget(m_variableButton);

    m_variableMenu = new QMenu(this);
    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
    {
        QAction* action = m_variableMenu->addAction(it.value());
        action->setData(it.key());
        connect(action, &QAction::triggered, this, &CustomInputDialog::insertVariable);
    }

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(m_variableButton, &QPushButton::clicked, this, &CustomInputDialog::showVariableMenu);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString CustomInputDialog::getText() const
{
    return m_lineEdit->text();
}

void CustomInputDialog::showVariableMenu()
{
    m_variableMenu->exec(QCursor::pos());
}

void CustomInputDialog::insertVariable()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QString variable = action->data().toString();
        int cursorPos = m_lineEdit->cursorPosition();
        m_lineEdit->insert(variable);
        m_lineEdit->setCursorPosition(cursorPos + variable.length());
    }
}

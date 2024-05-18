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

#ifndef CUSTOMINPUTDIALOG_H
#define CUSTOMINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QHash>

class CustomInputDialog : public QDialog
{
    Q_OBJECT

public:
    CustomInputDialog(const QString& title, const QString& labelText, const QString& initialText, const QString& defaultValue, const QHash<QString, QString>& variables, QWidget* parent = nullptr);

    QString getText() const;

private slots:
    void showVariableMenu();
    void insertVariable();
    void resetDefaultValue();

private:
    QLineEdit* m_lineEdit;
    QPushButton* m_variableButton;
    QMenu* m_variableMenu;
    QString m_defaultValue;
};

#endif // CUSTOMINPUTDIALOG_H

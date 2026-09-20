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

#ifndef CHATTEMPLATEEDITDLG_H
#define CHATTEMPLATEEDITDLG_H

#include "ui_chattemplateeditdlg.h"
#include "utilui.h"

class ChatTemplateEditDlg : public QDialog
{
    Q_OBJECT

public:
    ChatTemplateEditDlg(ChatTemplates templateId, QWidget* parent = nullptr);

    QString getMessage() const { return ui.CTMsgEdit->text(); }

private:
    void insertVariable();
    void slotRestoreDefault();

private:
    Ui::ChatTemplateEditDlg ui;
    QMenu* m_CTVarMenu;
    ChatTemplates m_templateId;

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

#endif

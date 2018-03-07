/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#ifndef AUDIOSTORAGEDLG_H
#define AUDIOSTORAGEDLG_H

#include "ui_mediastorage.h"

class MediaStorageDlg : public QDialog
{
    Q_OBJECT

public:
    MediaStorageDlg(QWidget * parent = 0);

public slots:
    void accept();
private slots:
    void slotSetMediaFolder();
    void slotSetChanLogFolder();
    void slotSetUserLogFolder();
private:
    QString getFolder();
    Ui::MediaStorageDlg ui;
};

#endif

/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef USERVOLUMEDLG_H
#define USERVOLUMEDLG_H

#include "ui_uservolume.h"

class UserVolumeDlg : public QDialog
{
    Q_OBJECT

public:
    UserVolumeDlg(int userid, QWidget * parent = 0);

private slots:
    void slotVolumeChanged(int);
    void slotMuteChannel();
    void slotDefaults();

private:
    int m_userid;
    Ui::UserVolumeDlg ui;
};
#endif

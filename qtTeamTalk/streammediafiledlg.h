/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef STREAMMEDIAFILEDLG_H
#define STREAMMEDIAFILEDLG_H

#include "ui_streammediafile.h"

class StreamMediaFileDlg : public QDialog
{
    Q_OBJECT

public:
    StreamMediaFileDlg(QWidget* parent = 0);
    
private slots:
    void slotAccepted();
    void slotSelectFile();
    void showMediaFormatInfo();

private:
    Ui::StreamMediaFileDlg ui;
};

#endif

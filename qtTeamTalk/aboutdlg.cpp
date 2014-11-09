/*
 * Copyright (c) 2005-2014, BearWare.dk
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

#include "aboutdlg.h"
#include "appinfo.h"
#include "common.h"

AboutDlg::AboutDlg(QWidget* parent)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    QString compile = "Compiled on " __DATE__ " " __TIME__ ".\r\n"
                      "Version " TEAMTALK_VERSION ".\r\n";
    if(sizeof(void*) == 8)
        compile += QString("TeamTalk 64-bit DLL version %1.").arg(_Q(TT_GetVersion()));
    else
        compile += QString("TeamTalk 32-bit DLL version %1.").arg(_Q(TT_GetVersion()));
    ui.compileLabel->setText(compile);
    ui.versionLabel->setText(APPVERSION);
    ui.iconLabel->setPixmap(QPixmap(APPICON));
}

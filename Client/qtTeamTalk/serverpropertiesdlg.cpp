/*
 * Copyright (c) 2005-2017, BearWare.dk
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

#include "serverpropertiesdlg.h"
#include "appinfo.h"

#include <QMessageBox>

extern TTInstance* ttInst;

ServerPropertiesDlg::ServerPropertiesDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    bool editable = (TT_GetMyUserRights(ttInst) & USERRIGHT_UPDATE_SERVERPROPERTIES);

    if(!editable)
        ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);

    connect(ui.motdChkBox, SIGNAL(clicked(bool)), SLOT(slotShowMOTDVars(bool)));
    connect(this, SIGNAL(accepted()), SLOT(slotAccepted()));
    ZERO_STRUCT(m_srvprop);

    TT_GetServerProperties(ttInst, &m_srvprop);

    ui.servernameEdit->setText(_Q(m_srvprop.szServerName));
    ui.maxusersSpinBox->setValue(m_srvprop.nMaxUsers);
    ui.motdTextEdit->setPlainText(_Q(m_srvprop.szMOTD));
    ui.tcpportSpinBox->setValue(m_srvprop.nTcpPort);
    ui.udpportSpinBox->setValue(m_srvprop.nUdpPort);
    ui.usertimeoutSpinBox->setValue(m_srvprop.nUserTimeout);
    ui.autosaveBox->setChecked(m_srvprop.bAutoSave);
    ui.maxloginattemptsSpinBox->setValue(m_srvprop.nMaxLoginAttempts);
    ui.maxiploginsSpinBox->setValue(m_srvprop.nMaxLoginsPerIPAddress);
    ui.audtxSpinBox->setValue(m_srvprop.nMaxVoiceTxPerSecond/1024);
    ui.videotxSpinBox->setValue(m_srvprop.nMaxVideoCaptureTxPerSecond/1024);
    ui.mediafiletxSpinBox->setValue(m_srvprop.nMaxMediaFileTxPerSecond/1024);
    ui.desktoptxSpinBox->setValue(m_srvprop.nMaxDesktopTxPerSecond/1024);
    ui.totaltxSpinBox->setValue(m_srvprop.nMaxTotalTxPerSecond/1024);
    ui.serverversionEdit->setText(_Q(m_srvprop.szServerVersion));

    if(!editable)
    {
        ui.servernameEdit->setEnabled(false);
        ui.maxusersSpinBox->setEnabled(false);
        ui.motdTextEdit->setEnabled(false);
        ui.tcpportSpinBox->setEnabled(false);
        ui.udpportSpinBox->setEnabled(false);
        ui.usertimeoutSpinBox->setEnabled(false);
        ui.autosaveBox->setEnabled(false);
        ui.maxloginattemptsSpinBox->setEnabled(false);
        ui.maxiploginsSpinBox->setEnabled(false);
        ui.audtxSpinBox->setEnabled(false);
        ui.videotxSpinBox->setEnabled(false);
        ui.mediafiletxSpinBox->setEnabled(false);
        ui.desktoptxSpinBox->setEnabled(false);
        ui.totaltxSpinBox->setEnabled(false);
        ui.motdChkBox->hide();
    }
}

void ServerPropertiesDlg::slotAccepted()
{
    COPY_TTSTR(m_srvprop.szServerName, ui.servernameEdit->text());
    m_srvprop.nMaxUsers = ui.maxusersSpinBox->value();
    m_srvprop.nMaxLoginAttempts = ui.maxloginattemptsSpinBox->value();
    m_srvprop.nMaxLoginsPerIPAddress = ui.maxiploginsSpinBox->value();
    if(_Q(m_srvprop.szMOTDRaw).size() &&
       _Q(m_srvprop.szMOTDRaw) != ui.motdTextEdit->toPlainText())
    {
        if(QMessageBox::question(this,  tr("Message of the day"), 
                    tr("Change message of the day?"), 
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
           COPY_TTSTR(m_srvprop.szMOTDRaw, ui.motdTextEdit->toPlainText());
    }
    m_srvprop.nTcpPort = ui.tcpportSpinBox->value();
    m_srvprop.nUdpPort = ui.udpportSpinBox->value();
    m_srvprop.nUserTimeout = ui.usertimeoutSpinBox->value();
    m_srvprop.bAutoSave = ui.autosaveBox->isChecked();
    m_srvprop.nMaxVoiceTxPerSecond = ui.audtxSpinBox->value()*1024;
    m_srvprop.nMaxVideoCaptureTxPerSecond = ui.videotxSpinBox->value()*1024;
    m_srvprop.nMaxMediaFileTxPerSecond = ui.mediafiletxSpinBox->value()*1024;
    m_srvprop.nMaxDesktopTxPerSecond = ui.desktoptxSpinBox->value()*1024;
    m_srvprop.nMaxTotalTxPerSecond = ui.totaltxSpinBox->value()*1024;

    TT_DoUpdateServer(ttInst, &m_srvprop);
}

void ServerPropertiesDlg::slotShowMOTDVars(bool checked)
{
    if(checked)
        ui.motdTextEdit->setPlainText(_Q(m_srvprop.szMOTDRaw));
    else
        ui.motdTextEdit->setPlainText(_Q(m_srvprop.szMOTD));
}

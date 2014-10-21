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
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */


#include "generatettfiledlg.h"
#include "keycompdlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QFileDialog>
#include <QMessageBox>

extern QSettings* ttSettings;

GenerateTTFileDlg::GenerateTTFileDlg(const HostEntry& entry, QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_hostentry(entry)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    setWindowTitle(windowTitle() + " - " + entry.ipaddr + ":" +
                   QString::number(entry.tcpport));

    ui.usernameEdit->setText(m_hostentry.username);
    ui.passwordEdit->setText(m_hostentry.password);

    connect(ui.saveBtn, SIGNAL(clicked()), SLOT(slotSaveTTFile()));
    connect(ui.closeBtn, SIGNAL(clicked()), SLOT(accept()));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.nicknameEdit, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.maleRadioButton, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.femaleRadioButton, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.pttChkBox, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.setupkeysButton, SLOT(setEnabled(bool)));

    connect(ui.pttChkBox, SIGNAL(clicked(bool)), ui.setupkeysButton, SLOT(setEnabled(bool)));
    connect(ui.pttChkBox, SIGNAL(clicked(bool)), ui.keycompEdit, SLOT(setEnabled(bool)));
    connect(ui.setupkeysButton, SIGNAL(clicked()), SLOT(slotSetupHotkey()));
}

void GenerateTTFileDlg::slotSetupHotkey()
{
    KeyCompDlg dlg(this);
    if(!dlg.exec())
        return;

    m_hostentry.hotkey = dlg.m_hotkey;
    ui.keycompEdit->setText(getHotKeyText(dlg.m_hotkey));
}

void GenerateTTFileDlg::slotSaveTTFile()
{
    if(ui.overrideChkBox->isChecked())
    {
        m_hostentry.username = ui.usernameEdit->text();
        m_hostentry.password = ui.passwordEdit->text();
        m_hostentry.nickname = ui.nicknameEdit->text();
        m_hostentry.gender = ui.femaleRadioButton->isChecked()?GENDER_FEMALE:GENDER_MALE;
        if(!ui.pttChkBox->isChecked())
            m_hostentry.hotkey.clear();
    }
    QByteArray xml = generateTTFile(m_hostentry);

    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();

    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
        start_dir, tr("%1 File (*%1)").arg(TTFILE_EXT));
    if(filename.size())
    {
        ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());

        QFile file(filename);
        if(!file.open(QIODevice::WriteOnly))
            QMessageBox::critical(this, tr("Save File"), tr("Unable to save file"));
        else
            file.write(xml);
    }
}

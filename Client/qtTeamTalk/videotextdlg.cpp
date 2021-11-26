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

#include "videotextdlg.h"
#include "appinfo.h"
#include "settings.h"
#include "utilui.h"

#include <QColorDialog>
#include <QColorDialog>
#include <QTimer>
#include <QPushButton>

extern QSettings* ttSettings;

VideoTextDlg::VideoTextDlg(QWidget* parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Ok"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    quint32 videotext = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT,
                                          SETTINGS_DISPLAY_VIDEOTEXT_DEFAULT).toUInt();
    switch(videotext & VIDTEXT_SHOW_MASK)
    {
    case VIDTEXT_SHOW_USERNAME :
        ui.usernameRadioButton->setChecked(true);
        break;
    case VIDTEXT_SHOW_STATUSTEXT :
        ui.statusRadioButton->setChecked(true);
        break;
    case VIDTEXT_SHOW_NICKNAME :
    default :
        ui.nicknameRadioButton->setChecked(true);
        break;
    }

    switch(videotext & VIDTEXT_POSITION_MASK)
    {
    case VIDTEXT_POSITION_TOPLEFT :
        ui.pos_tlBtn->setChecked(true);
        break;
    case VIDTEXT_POSITION_TOPRIGHT :
        ui.pos_trBtn->setChecked(true);
        break;
    case VIDTEXT_POSITION_BOTTOMLEFT :
        ui.pos_blBtn->setChecked(true);
        break;
    case VIDTEXT_POSITION_BOTTOMRIGHT :
        ui.pos_brBtn->setChecked(true);
        break;
    }

    ui.widthSpinBox->setValue(ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_WIDTH,
                              SETTINGS_DISPLAY_VIDEOTEXT_WIDTH_DEFAULT).toInt());
    ui.heightSpinBox->setValue(ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT,
                              SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT_DEFAULT).toInt());

    m_bgColor = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR,
                                  SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR_DEFAULT)
                                  .value<QColor>();
    m_fgColor = ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR,
                                  SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR_DEFAULT)
                                  .value<QColor>();

    connect(ui.fontBtn, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotSelectFont);
    connect(ui.bgBtn, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotSelectBackground);
    connect(ui.nicknameRadioButton, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.usernameRadioButton, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.statusRadioButton, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.pos_tlBtn, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.pos_trBtn, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.pos_blBtn, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.pos_brBtn, &QAbstractButton::clicked,
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VideoTextDlg::slotUpdateExample);
    connect(ui.heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VideoTextDlg::slotUpdateExample);

    QTimer::singleShot(0, this, &VideoTextDlg::slotUpdateExample);
}

void VideoTextDlg::accept()
{
    ttSettings->setValue(SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR,
                         m_fgColor);

    ttSettings->setValue(SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR,
                         m_bgColor);

    ttSettings->setValue(SETTINGS_DISPLAY_VIDEOTEXT,
                         (int)getVideoTextPos());


    ttSettings->setValue(SETTINGS_DISPLAY_VIDEOTEXT_WIDTH,
                         ui.widthSpinBox->value());
    ttSettings->setValue(SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT, ui.heightSpinBox->value());
    QDialog::accept();
}

quint32 VideoTextDlg::getVideoTextPos() const
{
    quint32 ret = 0;

    if(ui.pos_tlBtn->isChecked())
        ret |= VIDTEXT_POSITION_TOPLEFT;
    else if(ui.pos_trBtn->isChecked())
        ret |= VIDTEXT_POSITION_TOPRIGHT;
    else if(ui.pos_blBtn->isChecked())
        ret |= VIDTEXT_POSITION_BOTTOMLEFT;
    else
        ret |= VIDTEXT_POSITION_BOTTOMRIGHT;

    if(ui.statusRadioButton->isChecked())
        ret |= VIDTEXT_SHOW_STATUSTEXT;
    else if(ui.usernameRadioButton->isChecked())
        ret |= VIDTEXT_SHOW_USERNAME;
    else
        ret |= VIDTEXT_SHOW_NICKNAME;

    return ret;
}

void VideoTextDlg::slotSelectFont()
{
    QColorDialog dlg(m_fgColor, this);
    if(dlg.exec())
    {
        m_fgColor = dlg.selectedColor();
        slotUpdateExample();
    }
}

void VideoTextDlg::slotSelectBackground()
{
    QColorDialog dlg(m_bgColor, this);
    if(dlg.exec())
    {
        m_bgColor = dlg.selectedColor();
        slotUpdateExample();
    }
}

void VideoTextDlg::slotUpdateExample()
{
    int src_w = ui.exampleLabel->width();
    int src_h = ui.exampleLabel->height();

    QPixmap pixmap(src_w, src_h);
    QPainter p(&pixmap);
    p.fillRect(0, 0, src_w, src_h, Qt::gray);

    QString text = "Example";
    switch(getVideoTextPos() & VIDTEXT_SHOW_MASK)
    {
    case VIDTEXT_SHOW_NICKNAME :
        text = tr("Nickname");
        break;
    case VIDTEXT_SHOW_USERNAME :
        text = tr("Username");
        break;
    case VIDTEXT_SHOW_STATUSTEXT :
        text = tr("Status message");
        break;
    }

    setVideoTextBox(QRect(0, 0, src_w, src_h), m_bgColor, m_fgColor, text,
                    getVideoTextPos(), ui.widthSpinBox->value(),
                    ui.heightSpinBox->value(), p);

    ui.exampleLabel->setPixmap(pixmap);
}

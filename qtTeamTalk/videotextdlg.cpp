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

#include "videotextdlg.h"
#include "appinfo.h"
#include "settings.h"
#include "common.h"

#include <QColorDialog>
#include <QColorDialog>
#include <QTimer>

extern QSettings* ttSettings;

VideoTextDlg::VideoTextDlg(QWidget* parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

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

    connect(ui.fontBtn, SIGNAL(clicked()),
            SLOT(slotSelectFont()));
    connect(ui.bgBtn, SIGNAL(clicked()),
            SLOT(slotSelectBackground()));
    connect(ui.nicknameRadioButton, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.usernameRadioButton, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.statusRadioButton, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.pos_tlBtn, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.pos_trBtn, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.pos_blBtn, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.pos_brBtn, SIGNAL(clicked()),
            SLOT(slotUpdateExample()));
    connect(ui.widthSpinBox, SIGNAL(valueChanged(int)),
            SLOT(slotUpdateExample()));
    connect(ui.heightSpinBox, SIGNAL(valueChanged(int)),
            SLOT(slotUpdateExample()));

    QTimer::singleShot(0, this, SLOT(slotUpdateExample()));
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

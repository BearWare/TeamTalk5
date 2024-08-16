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

#include "utilui.h"
#include "settings.h"
#include "bearwarelogindlg.h"
#include "appinfo.h"
#include "utilsound.h"
#include "utilhotkey.h"

#include <QTranslator>
#include <QDir>
#include <QDateTime>
#include <QLocale>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QDesktopWidget>
#include <QApplication>
#else
#include <QScreen>
#include <QGuiApplication>
#endif

extern TTInstance* ttInst;
extern QSettings* ttSettings;
extern QTranslator* ttTranslator;

void migrateSettings()
{
    // Before version "5.4" the version information was not written or updated.
    // So basically the pre v5.4 migrations were never activated.
    QString iniversion = ttSettings->value(SETTINGS_GENERAL_VERSION, "5.0").toString();

    if (!versionSameOrLater(iniversion, "5.1"))
    {
        // Volume defaults changed in 5.1 format
        ttSettings->remove(SETTINGS_SOUND_MASTERVOLUME);
        ttSettings->remove(SETTINGS_SOUND_MICROPHONEGAIN);
    }
    if (!versionSameOrLater(iniversion, "5.2"))
    {
        // Gender changed in 5.2 format
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        if (ttSettings->value(SETTINGS_GENERAL_GENDER).type() == QVariant::Bool)
#else
        if (ttSettings->value(SETTINGS_GENERAL_GENDER).typeId() == QMetaType::Bool)
#endif
        {
            Gender gender = ttSettings->value(SETTINGS_GENERAL_GENDER).toBool() ? GENDER_MALE : GENDER_FEMALE;
            ttSettings->setValue(SETTINGS_GENERAL_GENDER, gender);
        }
    }
    if (!versionSameOrLater(iniversion, "5.3"))
    {
        if (ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME, "").toString().size())
        {
            QFile::setPermissions(ttSettings->fileName(), QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }
    }
    if (!versionSameOrLater(iniversion, "5.4"))
    {
        // Latest hosts changed in 5.4 format
        ttSettings->beginGroup("latesthosts");
        int index = 0;
        while (ttSettings->contains(QString("%1_hostaddr").arg(index)))
        {
            QString hostAddr = ttSettings->value(QString("%1_hostaddr").arg(index)).toString();
            int tcpPort = ttSettings->value(QString("%1_tcpport").arg(index)).toInt();
            ttSettings->setValue(QString("%1_name").arg(index), QString("%1_%2").arg(hostAddr).arg(tcpPort));
            index++;
        }
        ttSettings->endGroup();

        // Sound Events changed in 5.4 format
        SoundEvents activeEvents = SOUNDEVENT_NONE;
        auto eventMap = UtilSound::eventToSettingMap();
        for (SoundEvents event = SOUNDEVENT_NEWUSER; event < SOUNDEVENT_NEXT_UNUSED; event <<= 1)
        {
            SoundEvents eventId = SoundEvent(event);
            const SoundEventInfo& eventInfo = eventMap[eventId];
            if (!ttSettings->value(eventInfo.settingKey).toString().isEmpty())
                activeEvents |= event;
        }

        ttSettings->setValue(SETTINGS_SOUNDEVENT_ACTIVEEVENTS, activeEvents);

        // TTS options removed in 5.4 format
        ttSettings->remove("texttospeech/announce-server-name");
#if defined(Q_OS_DARWIN)
        ttSettings->remove("texttospeech/speak-lists");
#endif

        // Language files was renamed in 5.4 format
        if (ttSettings->contains(SETTINGS_DISPLAY_LANGUAGE))
        {
            QString lc_code;
            QString lang = ttSettings->value(SETTINGS_DISPLAY_LANGUAGE, SETTINGS_DISPLAY_LANGUAGE_DEFAULT).toString();
            if (lang == "Bulgarian") lc_code = "bg";
            else if (lang == "Chinese_Simplified") lc_code = "zh_CN";
            else if (lang == "Chinese_Traditional") lc_code = "zh_TW";
            else if (lang == "Croatian") lc_code = "hr";
            else if (lang == "Czech") lc_code = "cs";
            else if (lang == "Danish") lc_code = "da";
            else if (lang == "Dutch") lc_code = "nl";
            else if (lang == "English") lc_code = "en";
            else if (lang == "French") lc_code = "fr";
            else if (lang == "German") lc_code = "de";
            else if (lang == "Hebrew") lc_code = "he";
            else if (lang == "Hungarian") lc_code = "hu";
            else if (lang == "Indonesian") lc_code = "id";
            else if (lang == "Italian") lc_code = "it";
            else if (lang == "Korean") lc_code = "ko";
            else if (lang == "Persian") lc_code = "fa";
            else if (lang == "Polish") lc_code = "pl";
            else if (lang == "Portuguese_BR") lc_code = "pt_BR";
            else if (lang == "Portuguese_EU") lc_code = "pt_PT";
            else if (lang == "Russian") lc_code = "ru";
            else if (lang == "Slovak") lc_code = "sk";
            else if (lang == "Slovenian") lc_code = "sl";
            else if (lang == "Spanish") lc_code = "es";
            else if (lang == "Thai") lc_code = "th";
            else if (lang == "Turkish") lc_code = "tr";
            else if (lang == "Vietnamese") lc_code = "vi";
            ttSettings->setValue(SETTINGS_DISPLAY_LANGUAGE, lc_code);
        }

        // Shortcuts changed in 5.4 format
        Hotkeys hks = HOTKEY_NONE;

        for (Hotkeys hk = HOTKEY_FIRST; hk < HOTKEY_NEXT_UNUSED; hk <<= 1)
        {
            hotkey_t hotkey;
            HotKeyID hki = static_cast<HotKeyID>(hk);
            if (loadHotKeySettings(hki, hotkey))
            {
                hks = static_cast<Hotkeys>(hks | hk);
            }
        }

        ttSettings->setValue(SETTINGS_SHORTCUTS_ACTIVEHKS, hks);
        ttSettings->remove("general_/push-to-talk");
    }
    if (!versionSameOrLater(iniversion, "5.5"))
    {
        // Setting to display emoji in channel list changed in 5.5 format
        if (ttSettings->contains("display/show-emoji") && ttSettings->value("display/show-emoji").toBool() == false)
            ttSettings->setValue(SETTINGS_DISPLAY_INFOSTYLE, STYLE_NONE);
        ttSettings->remove("display/show-emoji");
    }

    if (ttSettings->value(SETTINGS_GENERAL_VERSION).toString() != SETTINGS_VERSION)
        ttSettings->setValue(SETTINGS_GENERAL_VERSION, SETTINGS_VERSION);
}

QHash<StatusBarEvents, StatusBarEventInfo> UtilUI::eventToSettingMap()
{
    static QHash<StatusBarEvents, StatusBarEventInfo> map =
    {
        { STATUSBAR_USER_LOGGEDIN, {SETTINGS_STATUSBARMSG_USER_LOGGEDIN, {{"{user}", tr("User's nickname who logged in")}, {"{username}", tr("User's username who logged in")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { STATUSBAR_USER_LOGGEDOUT, {SETTINGS_STATUSBARMSG_USER_LOGGEDOUT, {{"{user}", tr("User's nickname who logged out")}, {"{username}", tr("User's username who logged out")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { STATUSBAR_USER_JOINED, {SETTINGS_STATUSBARMSG_USER_JOINED, {{"{user}", tr("User's nickname who joined channel")}, {"{username}", tr("User's username who joined channel")}, {"{channel}", tr("Channel's name joined by user")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { STATUSBAR_USER_LEFT, {SETTINGS_STATUSBARMSG_USER_LEFT, {{"{user}", tr("User's nickname who left channel")}, {"{username}", tr("User's username who left channel")}, {"{channel}", tr("Channel's name left by user")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { STATUSBAR_USER_JOINED_SAME, {SETTINGS_STATUSBARMSG_USER_JOINED_SAME, {{"{user}", tr("User's nickname who joined channel")}, {"{username}", tr("User's username who joined channel")}}, "" } },
        { STATUSBAR_USER_LEFT_SAME, {SETTINGS_STATUSBARMSG_USER_LEFT_SAME, {{"{user}", tr("User's nickname who left channel")}, {"{username}", tr("User's username who left channel")}}, "" } },
        { STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_VOICE, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_VIDEO, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_DESKTOP, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_MEDIAFILE, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE, {SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { STATUSBAR_CLASSROOM_CHANMSG_TX, {SETTINGS_STATUSBARMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { STATUSBAR_CLASSROOM_VOICE_TX, {SETTINGS_STATUSBARMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { STATUSBAR_CLASSROOM_VIDEO_TX, {SETTINGS_STATUSBARMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { STATUSBAR_CLASSROOM_DESKTOP_TX, {SETTINGS_STATUSBARMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { STATUSBAR_CLASSROOM_MEDIAFILE_TX, {SETTINGS_STATUSBARMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { STATUSBAR_FILE_ADD, {SETTINGS_STATUSBARMSG_FILE_ADDED, {{"{filename}", tr("File name")}, {"{user}", tr("User's nickname who added the file")}, {"{username}", tr("User's username who added the file")}, {"{filesize}", tr("File size")}}, "" } },
        { STATUSBAR_FILE_REMOVE, {SETTINGS_STATUSBARMSG_FILE_REMOVED, {{"{file}", tr("File name")}, {"{user}", tr("User's nickname who removed the file")}, {"{username}", tr("User's username who removed the file")}}, "" } },
    };
    return map;
}

void setVideoTextBox(const QRect& rect, const QColor& bgcolor,
                     const QColor& fgcolor, const QString& text,
                     quint32 text_pos, int w_percent, int h_percent,
                     QPainter& painter)
{
    int w = w_percent / 100. * rect.width();
    int h = h_percent / 100. * rect.height();

    int x, y;
    switch(text_pos & VIDTEXT_POSITION_MASK)
    {
    case VIDTEXT_POSITION_TOPLEFT :
        x = 0; y = 0;
        break;
    case VIDTEXT_POSITION_TOPRIGHT :
        x = rect.width() - w;
        y = 0;
        break;
    case VIDTEXT_POSITION_BOTTOMLEFT :
        x = 0;
        y = rect.height() - h;
        break;
    case VIDTEXT_POSITION_BOTTOMRIGHT :
    default :
        x = rect.width() - w;
        y = rect.height() - h;
        break;
    }

    if(h>0 && w>0)
    {
        const QFont font = painter.font();
        if(font.pixelSize() != h)
        {
            QFont newFont(font);
            newFont.setPixelSize(h);
            painter.setFont(newFont);
        }
        painter.fillRect(x, y, w, h, bgcolor);
        painter.setPen(fgcolor);
        painter.drawText(x, y, w, h, Qt::AlignHCenter | Qt::AlignCenter, text);

        if(font.pixelSize() != h)
            painter.setFont(font);
    }
}

#if defined(Q_OS_DARWIN)
void setMacResizeMargins(QDialog* dlg, QLayout* layout)
{
    QSize size = dlg->size();
    QMargins margins = layout->contentsMargins();
    margins.setBottom(margins.bottom()+12);
    layout->setContentsMargins(margins);
    size += QSize(0, 12);
    dlg->resize(size);
}
#endif /* Q_OS_DARWIN */

void setCurrentItemData(QComboBox* cbox, const QVariant& itemdata)
{
    int index = cbox->findData(itemdata);
    if(index>=0)
        cbox->setCurrentIndex(index);
}

QVariant getCurrentItemData(QComboBox* cbox, const QVariant& not_found/* = QVariant()*/)
{
    if(cbox->currentIndex()>=0)
        return cbox->itemData(cbox->currentIndex());
    return not_found;
}

QString getBearWareWebLogin(QWidget* parent)
{
    QString username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
    if (username.isEmpty())
    {
        BearWareLoginDlg dlg(parent);
        if (dlg.exec())
        {
            username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
        }
    }
    return username;
}

textmessages_t buildTextMessages(const TextMessage& msg, const QString& content)
{
    Q_ASSERT(msg.szMessage[0] == '\0');

    textmessages_t result;
    MyTextMessage newmsg(msg);
    QString remain = content;

    if (remain.toUtf8().size() <= TT_STRLEN - 1)
    {
        COPY_TTSTR(newmsg.szMessage, remain);
        newmsg.bMore = FALSE;
        result.append(newmsg);
        return result;
    }

    newmsg.bMore = TRUE;

    int curlen = remain.size();
    while (remain.left(curlen).toUtf8().size() > TT_STRLEN - 1)
        curlen /= 2;

    int half = TT_STRLEN / 2;
    while (half > 0)
    {
        auto utf8str = remain.left(curlen + half).toUtf8();
        if (utf8str.size() <= TT_STRLEN - 1)
            curlen += half;
        if (utf8str.size() == TT_STRLEN - 1)
            break;
        half /= 2;
    }

    COPY_TTSTR(newmsg.szMessage, remain.left(curlen));
    result.append(newmsg);
    newmsg.szMessage[0] = {'\0'};
    result.append(buildTextMessages(newmsg, remain.mid(curlen)));
    return result;
}

textmessages_t sendTextMessage(const TextMessage& msg, const QString& content)
{
    bool sent = true;
    auto messages = buildTextMessages(msg, content);
    for (const auto& m : messages)
    {
        sent = sent && TT_DoTextMessage(ttInst, &m) > 0;
    }
    return sent ? messages : textmessages_t();
}

RestoreIndex::RestoreIndex(QAbstractItemView* view)
    : m_view(view)
{
    m_parent = view->currentIndex().parent();
    m_row = view->currentIndex().row();
    m_column = view->currentIndex().column();
}

RestoreIndex::~RestoreIndex()
{
    if (m_view->model()->rowCount() == 0 || m_view->model()->columnCount() == 0)
        return;

    m_row = std::min(m_row, m_view->model()->rowCount() - 1);
    m_column = std::min(m_column, m_view->model()->columnCount() - 1);
    m_view->setCurrentIndex(m_view->model()->index(m_row, m_column, m_parent));
}

RestoreItemData::RestoreItemData(QAbstractItemView* view, QSortFilterProxyModel* model /*= nullptr*/)
    : m_view(view)
    , m_proxy(model)
{
    QItemSelectionModel* selModel = view->selectionModel();
    QModelIndexList indexes = selModel->selectedRows();
    for (auto& i : indexes)
    {
        if (model)
            m_intdata.push_back(model->mapToSource(i).internalId());
        else
            m_intdata.push_back(i.internalId());
    }
}

RestoreItemData::~RestoreItemData()
{
    if (m_view->model()->rowCount() == 0 || m_view->model()->columnCount() == 0)
        return;

    for (int r=0;r<m_view->model()->rowCount();++r)
    {
        QModelIndex i = m_view->model()->index(r, 0);
        quintptr id = i.internalId();
        if (m_proxy)
            id = m_proxy->mapToSource(i).internalId();

        if (m_intdata.contains(id))
        {
            m_view->setCurrentIndex(i);
            break;
        }
    }
}

void saveWindowPosition(const QString& setting, QWidget* widget)
{
    if (widget->windowState() == Qt::WindowNoState)
    {
        QRect r = widget->geometry();
        QVariantList windowpos;
        windowpos.push_back(r.x());
        windowpos.push_back(r.y());
        windowpos.push_back(r.width());
        windowpos.push_back(r.height());
        ttSettings->setValue(setting, windowpos);
    }
}

bool restoreWindowPosition(const QString& setting, QWidget* widget)
{
    bool success = false;
    QVariantList windowpos = ttSettings->value(setting).toList();
    if (windowpos.size() == 4)
    {
        int x = windowpos[0].toInt();
        int y = windowpos[1].toInt();
        int w = windowpos[2].toInt();
        int h = windowpos[3].toInt();

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        int desktopW = QApplication::desktop()->width();
        int desktopH = QApplication::desktop()->height();
        if(x <= desktopW && y <= desktopH)
        {
            widget->setGeometry(x, y, w, h);
            success = true;
        }
#else
        // check that we are within bounds
        QScreen* screen = QGuiApplication::screenAt(QPoint(x, y));
        if (screen)
        {
            widget->setGeometry(x, y, w, h);
            success = true;
        }
#endif
    }
    return success;
}


bool switchLanguage(const QString& language)
{
    QApplication::removeTranslator(ttTranslator);
    delete ttTranslator;
    ttTranslator = nullptr;
    if(!language.isEmpty())
    {
        ttTranslator = new QTranslator();
        if(ttTranslator->load(language, TRANSLATE_FOLDER))
            QApplication::installTranslator(ttTranslator);
        else
        {
            delete ttTranslator;
            ttTranslator = nullptr;
            return false;
        }
    }
    return true;
}

QStringList extractLanguages()
{
    QStringList languages;
    QDir dir(TRANSLATE_FOLDER, "*.qm", QDir::Name, QDir::Files);
    for (const auto& lang : dir.entryList())
        languages.append(lang.left(lang.size()-3));
    return languages;
}

QString getLanguageDisplayName(const QString &languageCode)
{
    QLocale locale(languageCode);
    QString languageName = locale.nativeLanguageName();
    if (languageName.isEmpty())
        languageName = languageCode;

    return languageName;
}

QString getFormattedDateTime(QString originalDateTimeString, QString inputFormat)
{
    QDateTime originalDateTime = QDateTime::fromString(originalDateTimeString, inputFormat);

    if (!originalDateTime.isValid()) {
        return QString("Invalid DateTime");
    }

    QLocale userLocale = QLocale::system();
    QString formattedDateTime = userLocale.toString(originalDateTime, getTimestampFormat());

    return formattedDateTime;
}

QString getTimestampFormat()
{
    QLocale userLocale = QLocale::system();
    QString format = ttSettings->value(SETTINGS_DISPLAY_TIMESTAMP_FORMAT).toString().isEmpty()?userLocale.dateTimeFormat(QLocale::ShortFormat):ttSettings->value(SETTINGS_DISPLAY_TIMESTAMP_FORMAT).toString();
    return format;
}

QString getFormattedFileSize(qint64 filesize)
{
    QString formattedFileSize;
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
    if (filesize >= 1024*1024*1024)
        formattedFileSize = QString("%1 G").arg(Filesize/(1024*1024*1024));
    else if (filesize >= 1024*1024)
        formattedFileSize = QString("%1 M").arg(filesize/(1024*1024));
    else if (filesize >= 1024)
        formattedFileSize = QString("%1 K").arg(filesize/1024);
    else
        formattedFileSize = QString("%1").arg(filesize);
#else
    formattedFileSize = QLocale().formattedDataSize(filesize, 1, QLocale::DataSizeSIFormat);
#endif
    return formattedFileSize;
}

QString UtilUI::getDefaultValue(const QString& paramKey)
{
    if (paramKey == SETTINGS_STATUSBARMSG_USER_LOGGEDIN)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_USER_LOGGEDIN_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_USER_LOGGEDOUT)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_USER_LOGGEDOUT_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_USER_JOINED)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_USER_JOINED_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_USER_LEFT)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_USER_LEFT_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_USER_JOINED_SAME)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_USER_JOINED_SAME_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_USER_LEFT_SAME)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_USER_LEFT_SAME_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_SUBCHANGE)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_SUBCHANGE_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_CLASSROOM)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_CLASSROOM_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_FILE_ADDED)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_FILE_ADDED_DEFAULT);
    if (paramKey == SETTINGS_STATUSBARMSG_FILE_REMOVED)
        return QCoreApplication::translate("UtilUI", SETTINGS_STATUSBARMSG_FILE_REMOVED_DEFAULT);
    return QString();
}

QString UtilUI::getStatusBarMessage(const QString& paramKey, const QHash<QString, QString>& variables)
{
    QString messageTemplate = ttSettings->value(paramKey, getDefaultValue(paramKey)).toString();

    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
    {
        messageTemplate.replace(it.key(), it.value());
    }

    return messageTemplate;
}

QString UtilUI::getRawStatusBarMessage(const QString& paramKey)
{
    return ttSettings->value(paramKey, getDefaultValue(paramKey)).toString();
}

LoginInfoDialog::LoginInfoDialog(const QString &title, const QString &desc, const QString &initialUsername, const QString &initialPassword, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setAccessibleDescription(desc);

    QLabel *descLabel = new QLabel(desc);
    QLabel *userLabel = new QLabel(tr("Username:"));
    userEdit = new QLineEdit;
    userEdit->setText(initialUsername);
    userLabel->setBuddy(userEdit);

    QLabel *passLabel = new QLabel(tr("Password:"));
    passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setText(initialPassword);
    passLabel->setBuddy(passEdit);

    QCheckBox *showPasswordCheckBox = new QCheckBox(tr("Show password"));
    connect(showPasswordCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        passEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    QPushButton *okButton = new QPushButton(tr("&OK"));
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"));

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descLabel);
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(userEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passEdit);
    mainLayout->addWidget(showPasswordCheckBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

QString LoginInfoDialog::getUsername() const
{
    return userEdit->text();
}

QString LoginInfoDialog::getPassword() const
{
    return passEdit->text();
}

PasswordDialog::PasswordDialog(const QString &title, const QString &desc, const QString &initialPassword, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setAccessibleDescription(desc);

    QLabel *descLabel = new QLabel(desc);
    QLabel *passLabel = new QLabel(tr("Password"));
    passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setText(initialPassword);
    passLabel->setBuddy(passEdit);

    QCheckBox *showPasswordCheckBox = new QCheckBox(tr("Show password"));
    connect(showPasswordCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        passEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    QPushButton *okButton = new QPushButton(tr("&OK"));
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"));

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descLabel);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passEdit);
    mainLayout->addWidget(showPasswordCheckBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

QString PasswordDialog::getPassword() const
{
    return passEdit->text();
}

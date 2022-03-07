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

#include "utilhotkey.h"
#include "settings.h"

extern QSettings* ttSettings;
extern TTInstance* ttInst;

QString getHotKeyString(HotKeyID keyid)
{
    switch(keyid)
    {
    case HOTKEY_PUSHTOTALK :
        return SETTINGS_GENERAL_PUSHTOTALK_KEY;
    case HOTKEY_VOICEACTIVATION :
        return SETTINGS_SHORTCUTS_VOICEACTIVATION;
    case HOTKEY_INCVOLUME :
        return SETTINGS_SHORTCUTS_INCVOLUME;
    case HOTKEY_DECVOLUME :
        return SETTINGS_SHORTCUTS_DECVOLUME;
    case HOTKEY_MUTEALL :
        return SETTINGS_SHORTCUTS_MUTEALL;
    case HOTKEY_MICROPHONEGAIN_INC :
        return SETTINGS_SHORTCUTS_INCVOICEGAIN;
    case HOTKEY_MICROPHONEGAIN_DEC :
        return SETTINGS_SHORTCUTS_DECVOICEGAIN;
    case HOTKEY_VIDEOTX :
        return SETTINGS_SHORTCUTS_VIDEOTX;
    case HOTKEY_REINITSOUNDDEVS :
        return SETTINGS_SHORTCUTS_RESTARTSOUNDDEVICES;
    case HOTKEY_SHOWHIDE_WINDOW :
        return SETTINGS_SHORTCUTS_SHOWHIDEWINDOW;
    default :
        Q_ASSERT(0); //unknown hotkey id
    }
    return QString();
}

void saveHotKeySettings(HotKeyID hotkeyid, const hotkey_t& hotkey)
{
    QStringList hklst;
    for(std::size_t i=0;i<hotkey.size();i++)
        hklst.push_back(QString::number(hotkey[i]));
    ttSettings->setValue(getHotKeyString(hotkeyid), hklst);
}

bool loadHotKeySettings(HotKeyID hotkeyid, hotkey_t& hotkey)
{
    QStringList hklst = ttSettings->value(getHotKeyString(hotkeyid)).toStringList();
    for(int i=0;i<hklst.size();i++)
        hotkey.push_back(hklst[i].toInt());
    return hklst.size();
}

void deleteHotKeySettings(HotKeyID hotkeyid)
{
    ttSettings->remove(getHotKeyString(hotkeyid));
}

#if defined(Q_OS_DARWIN)
QString QCFStringToQString(CFStringRef str)
{
    if(!str)
        return QString();
    CFIndex length = CFStringGetLength(str);
    const UniChar *chars = CFStringGetCharactersPtr(str);
    if (chars)
        return QString(reinterpret_cast<const QChar *>(chars), length);

    QVector<UniChar> buffer(length);
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
    return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
}

QString TranslateKey(quint8 vk)
{
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
    const CFDataRef layoutData =
        reinterpret_cast<const CFDataRef>(TISGetInputSourceProperty(currentKeyboard,
                                          kTISPropertyUnicodeKeyLayoutData) );

    const UCKeyboardLayout *keyboardLayout =
        reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layoutData));

    UInt32 keysDown = 0;
    UniChar chars[10];
    UniCharCount realLength = 0;

    OSStatus oss = UCKeyTranslate(keyboardLayout,
                                  vk,
                                  kUCKeyActionDown,
                                  0,
                                  LMGetKbdType(),
                                  0,//kUCKeyTranslateNoDeadKeysBit,
                                  &keysDown,
                                  sizeof(chars) / sizeof(chars[0]),
                                  &realLength,
                                  chars);
    Q_ASSERT(oss == 0);

    CFStringRef ptr_str = CFStringCreateWithCharacters(kCFAllocatorDefault,
                                                       chars, (int)realLength);
    QString ss = QCFStringToQString(ptr_str);
    CFRelease(ptr_str);
    CFRelease(currentKeyboard);
    return ss;
}

QString GetMacOSHotKeyText(const hotkey_t& hotkey)
{
    if(hotkey.size() != MAC_HOTKEY_SIZE)
        return QString();

    QString comp;

    if(hotkey[0] != (INT32)MAC_NO_KEY)
    {
        if(hotkey[0] & cmdKey)
            comp += comp.size()? " + Cmd" : "Cmd";
        if(hotkey[0] & shiftKey)
            comp += comp.size()? " + Shift" : "Shift";
        if(hotkey[0] & optionKey)
            comp += comp.size()? " + Option" : "Option";
        if(hotkey[0] & controlKey)
            comp += comp.size()? "+ Ctrl" : "Ctrl";
    }

    QString tmp;
    if(hotkey[1] != (INT32)MAC_NO_KEY)
    {
        quint8 vk = hotkey[1];
        switch(vk)
        {
        case kVK_ANSI_A:
        case kVK_ANSI_S:
        case kVK_ANSI_D:
        case kVK_ANSI_F:
        case kVK_ANSI_H:
        case kVK_ANSI_G:
        case kVK_ANSI_Z:
        case kVK_ANSI_X:
        case kVK_ANSI_C:
        case kVK_ANSI_V:
        case kVK_ANSI_B:
        case kVK_ANSI_Q:
        case kVK_ANSI_W:
        case kVK_ANSI_E:
        case kVK_ANSI_R:
        case kVK_ANSI_Y:
        case kVK_ANSI_T:
        case kVK_ANSI_1:
        case kVK_ANSI_2:
        case kVK_ANSI_3:
        case kVK_ANSI_4:
        case kVK_ANSI_6:
        case kVK_ANSI_5:
        case kVK_ANSI_Equal:
        case kVK_ANSI_9:
        case kVK_ANSI_7:
        case kVK_ANSI_Minus:
        case kVK_ANSI_8:
        case kVK_ANSI_0:
        case kVK_ANSI_RightBracket:
        case kVK_ANSI_O:
        case kVK_ANSI_U:
        case kVK_ANSI_LeftBracket:
        case kVK_ANSI_I:
        case kVK_ANSI_P:
        case kVK_ANSI_L:
        case kVK_ANSI_J:
        case kVK_ANSI_Quote:
        case kVK_ANSI_K:
        case kVK_ANSI_Semicolon:
        case kVK_ANSI_Backslash:
        case kVK_ANSI_Comma:
        case kVK_ANSI_Slash:
        case kVK_ANSI_N:
        case kVK_ANSI_M:
        case kVK_ANSI_Period:
        case kVK_ANSI_Grave:
        case kVK_ANSI_KeypadDecimal:
        case kVK_ANSI_KeypadMultiply:
        case kVK_ANSI_KeypadPlus:
        case kVK_ANSI_KeypadClear:
        case kVK_ANSI_KeypadDivide:
        case kVK_ANSI_KeypadEnter:
        case kVK_ANSI_KeypadMinus:
        case kVK_ANSI_KeypadEquals:
        case kVK_ANSI_Keypad0:
        case kVK_ANSI_Keypad1:
        case kVK_ANSI_Keypad2:
        case kVK_ANSI_Keypad3:
        case kVK_ANSI_Keypad4:
        case kVK_ANSI_Keypad5:
        case kVK_ANSI_Keypad6:
        case kVK_ANSI_Keypad7:
        case kVK_ANSI_Keypad8:
        case kVK_ANSI_Keypad9:
            tmp = TranslateKey(vk);
            break;
        case kVK_Return:
            tmp = "Return";break;
        case kVK_Tab:
            tmp = "Tab";break;
        case kVK_Space:
            tmp = "Space";break;
        case kVK_Delete:
            tmp = "BackSpace";break;
        case kVK_Escape:
            tmp = "Esc";break;
            //case kVK_Command:
            //case kVK_Shift:
        case kVK_CapsLock:
            tmp = "CapsLock";break;
            //case kVK_Option:
            //case kVK_Control:
            //case kVK_RightShift:
            //case kVK_RightOption:
            //case kVK_RightControl:
        case kVK_Function:
            tmp = "Fn";break;
        case kVK_F17:
            tmp = "F17";break;
        case kVK_VolumeUp:
            tmp = "VolUp";break;
        case kVK_VolumeDown:
            tmp = "VolDown";break;
        case kVK_Mute:
            tmp = "Mute";break;
        case kVK_F18:
            tmp = "F18";break;
        case kVK_F19:
            tmp = "F19";break;
        case kVK_F20:
            tmp = "F20";break;
        case kVK_F5:
            tmp = "F5";break;
        case kVK_F6:
            tmp = "F6";break;
        case kVK_F7:
            tmp = "F7";break;
        case kVK_F3:
            tmp = "F3";break;
        case kVK_F8:
            tmp = "F8";break;
        case kVK_F9:
            tmp = "F9";break;
        case kVK_F11:
            tmp = "F11";break;
        case kVK_F13:
            tmp = "F13";break;
        case kVK_F16:
            tmp = "F16";break;
        case kVK_F14:
            tmp = "F14";break;
        case kVK_F10:
            tmp = "F10";break;
        case kVK_F12:
            tmp = "F12";break;
        case kVK_F15:
            tmp = "F15";break;
        case kVK_Help:
            tmp = "Help";break;
        case kVK_Home:
            tmp = "Home";break;
        case kVK_PageUp:
            tmp = "PgUp";break;
        case kVK_ForwardDelete:
            tmp = "Delete";break;
        case kVK_F4:
            tmp = "F4";break;
        case kVK_End:
            tmp = "End";break;
        case kVK_F2:
            tmp = "F2";break;
        case kVK_PageDown:
            tmp = "PgDown";break;
        case kVK_F1:
            tmp = "F1";break;
        case kVK_LeftArrow:
            tmp = "Left";break;
        case kVK_RightArrow:
            tmp = "Right";break;
        case kVK_DownArrow:
            tmp = "Down";break;
        case kVK_UpArrow:
            tmp = "Up";break;
        default:
            tmp += QString::number(vk);
        }
    }
    comp += comp.size()? " + " + tmp : tmp;
    return comp;
}
#endif


QString getHotKeyText(const hotkey_t& hotkey)
{
#ifdef Q_OS_WIN32
    QString key;
    for(std::size_t i=0;i<hotkey.size();i++)
    {
        TTCHAR buff[TT_STRLEN] = {};
        TT_HotKey_GetKeyString(ttInst, hotkey[i], buff);
        key += (i == hotkey.size()-1)? _Q(buff):_Q(buff) + " + ";
    }
    return key;
#elif defined(Q_OS_LINUX)
    int keys[4] = {0, 0, 0, 0};
    for(std::size_t i=0;i<hotkey.size();i++)
        keys[i] = hotkey[i];

    QKeySequence keyseq(keys[0], keys[1], keys[2], keys[3]);
    return keyseq.toString();
#elif defined(Q_OS_DARWIN)
    /*
    QString key;
    if(hotkey.size())
        key = QString::number(hotkey[0]);

    for(int i=1;i<hotkey.size();i++)
        key += " + " + QString::number(hotkey[i]);

    return key;
    */
    return GetMacOSHotKeyText(hotkey);
#else
    return QString("Unknown");
#endif
}


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

#include "stdafx.h"
#include "ClientXML.h"
#include "MyStd.h"
#include <assert.h>
#include <sstream>

using namespace std;
using namespace tinyxml2;

namespace teamtalk {

    bool ClientXML::SaveFile()
    {
        SetFileVersion(TEAMTALK_XML_VERSION);
        return XMLDocument::SaveFile();
    }

    XMLElement* ClientXML::GetRootElement()
    {
        XMLElement* root = m_xmlDocument.RootElement();
        if(!root)
        {
            XMLElement* newroot = m_xmlDocument.NewElement(m_rootname.c_str());
            newroot->SetAttribute("version", m_xmlversion.c_str());
            m_xmlDocument.InsertEndChild(newroot);
            root = m_xmlDocument.RootElement();
        }
        return root;
    }


    XMLElement* ClientXML::GetMainElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* pMain = root->FirstChildElement("main");
            if(!pMain)
            {
                pMain = AppendElement(root, "main");
            }
            return pMain;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetGeneralElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(!child)
            {
                child = AppendElement(root, "general");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetWindowElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("window");
            if(!child)
            {
                child = AppendElement(root, "window");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetClientElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("clientsetup");
            if(!child)
            {
                child = AppendElement(root, "clientsetup");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetSoundSystemElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("soundsystem");
            if(!child)
            {
                child = AppendElement(root, "soundsystem");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetEventsElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("events");
            if(!child)
            {
                child = AppendElement(root, "events");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetVideoElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("videocapture");
            if(!child)
            {
                child = AppendElement(root, "videocapture");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetAdvancedElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("advanced");
            if(!child)
            {
                child = AppendElement(root, "advanced");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetShortCutsElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("shortcuts");
            if(!child)
            {
                child = AppendElement(root, "shortcuts");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetHostManagerElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("hostmanager");
            if(!child)
            {
                child = AppendElement(root, "hostmanager");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetLatestHostsElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("latesthosts");
            if(!child)
            {
                child = AppendElement(root, "latesthosts");
            }
            return child;
        }

        return NULL;
    }

    XMLElement* ClientXML::GetMediaFilesElement()
    {
        XMLElement* root = GetRootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("mediafiles");
            if(!child)
            {
                child = AppendElement(root, "mediafiles");
            }
            return child;
        }
        return NULL;
    }



    /****************************/
    /********* <main> ***********/
    /****************************/

    bool ClientXML::SetWindowPlacement(int x, int y, int cx, int cy)
    {
        XMLElement* pMain = GetMainElement();
        if(pMain)
        {
            XMLElement* element = ReplaceElement(pMain, "windowplacement");
            PutInteger(element, "left", x);
            PutInteger(element, "top", y);
            PutInteger(element, "width", cx);
            PutInteger(element, "height", cy);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetWindowPlacement(int& x, int& y, int& cx, int& cy)
    {
        XMLElement* item=m_xmlDocument.RootElement();
        if(item)
        {
            if((item = item->FirstChildElement("main")))
            {
                XMLElement* target = item->FirstChildElement("windowplacement");
                if(target)
                {
                    GetInteger(target, "left", x);
                    GetInteger(target, "top", y);
                    GetInteger(target, "width", cx);
                    GetInteger(target, "height", cy);
                    return true;
                }
            }
        }
        return false;
    }

    bool ClientXML::SetWindowExtended(bool bExtended)
    {
        XMLElement* pParent = GetMainElement();
        if(pParent)
        {
            PutBoolean(pParent, "extended", bExtended);
            return true;
        }
        return false;
    }

    bool ClientXML::GetWindowExtended()
    {
        XMLElement* pParent = GetMainElement();
        if(pParent)
        {
            bool bEnabled = true;
            GetBoolean(pParent, "extended", bEnabled);
            return bEnabled;
        }
        return true;
    }

    /**************************************/
    /*****************         ************/

    bool ClientXML::AddHostManagerEntry(const HostEntry& entry)
    {
        XMLElement* pParent = GetHostManagerElement();
        if(pParent)
        {
            XMLElement* element = AppendElement(pParent, "host");
            element->SetAttribute("name", entry.szEntryName.c_str());

            PutString(element, "address", entry.szAddress);
            PutInteger(element, "tcpport", entry.nTcpPort);
            PutInteger(element, "udpport", entry.nUdpPort);
            PutBoolean(element, "encrypted", entry.bEncrypted);
            PutString(element, "username", entry.szUsername);
            PutString(element, "password", entry.szPassword);
            PutString(element, "channel", entry.szChannel);
            PutString(element, "cpassword", entry.szChPasswd);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetHostManagerEntry(const std::string& entryname, HostEntry& entry)
    {
        bool found = false;
        for(int i=0;i<GetHostManagerEntryCount() && !found;i++)
        {
            GetHostManagerEntry(i, entry);
            if(entry.szEntryName==entryname)
                found = true;
        }
        return found;
    }

    bool ClientXML::RemoveHostManagerEntry(const string entryname)
    {
        XMLElement* item=m_xmlDocument.RootElement();

        if(item)
        {
            XMLElement* parent = item->FirstChildElement("hostmanager");
            if(parent)
                for(XMLElement* child = parent->FirstChildElement("host");
                    child;
                    child = child->NextSiblingElement("host"))
                {
                    const char* attr = child->Attribute("name");
                    string szName = attr ? attr : "";
                    if(strcmpnocase(szName, entryname))
                    {
                        parent->DeleteChild(child);
                        break;
                    }
                }
                return true;
        }
        return false;
    }

    int ClientXML::GetHostManagerEntryCount()
    {
        XMLElement* item=m_xmlDocument.RootElement();
        int count = 0;

        if(item)
        {
            item = item->FirstChildElement("hostmanager");
            if(item)
                for(XMLElement* child = item->FirstChildElement("host");
                    child;
                    child = child->NextSiblingElement("host"))
                {
                    count++;
                }
        }
        return count;
    }

    bool ClientXML::GetHostManagerEntry(int index, HostEntry& entry)
    {
        XMLElement* item=m_xmlDocument.RootElement();
        bool found = false;
        if(item)
        {
            int i = 0;
            item = item->FirstChildElement("hostmanager");
            if(item)
            {
                XMLElement* child = NULL;
                for(child = item->FirstChildElement("host");
                    child;
                    child = child->NextSiblingElement("host"))
                    if(i==index)
                        break;
                    else
                        i++;

                if(child && i==index)
                {
                    found = true;
                    HostEntry tmp;
                    const char* attr = child->Attribute("name");
                    tmp.szEntryName = attr ? attr : "";
                    found &= GetString(child, "address", tmp.szAddress);
                    found &= GetInteger(child, "tcpport", tmp.nTcpPort);
                    found &= GetInteger(child, "udpport", tmp.nUdpPort);
                    GetBoolean(child, "encrypted", tmp.bEncrypted);
                    found &= GetString(child, "username", tmp.szUsername);
                    found &= GetString(child, "password", tmp.szPassword);
                    found &= GetString(child, "channel", tmp.szChannel);
                    found &= GetString(child, "cpassword", tmp.szChPasswd);
                    if(found)
                        entry = tmp;
                }
                else found = false;
            }

        }

        return found;
    }

    /************** <general> ****************/

    bool ClientXML::SetNickname(const std::string& szNickname)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutString(pParent, "nickname", szNickname);
            return true;
        }
        else
            return false;
    }

    string ClientXML::GetNickname(std::string def_nickname/* = std::string()*/)
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
                GetString(child, "nickname", def_nickname);
        }
        return def_nickname;
    }

    void ClientXML::SetStatusMessage(const std::string& szStatusMsg)
    {
        SetValue("general/status-message", szStatusMsg);
    }

    std::string ClientXML::GetStatusMessage(std::string def_statusmsg/* = std::string()*/)
    {
        return GetValue(true, "general/status-message", def_statusmsg);
    }

    void ClientXML::SetBearWareLogin(const std::string& szUsername, const std::string& szToken)
    {
        SetValue("general/bearwareid/username", szUsername);
        SetValue("general/bearwareid/token", szToken);
    }

    void ClientXML::SetRestoreUserFromWebLogin(bool bEnable)
    {
        SetValue("general/restore-user-weblogin", bEnable);
    }
    
    bool ClientXML::GetRestoreUserFromWebLogin(bool bDefEnable)
    {
        return GetValue(true, "general/restore-user-weblogin", bDefEnable);
    }

    bool ClientXML::GetBearWareLogin(std::string& szUsername, std::string& szToken)
    {
        szUsername = GetValue(true, "general/bearwareid/username", "");
        szToken = GetValue(true, "general/bearwareid/token", "");
        return !szUsername.empty();
    }

    bool ClientXML::SetProfileName(const std::string& szProfilename)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutString(pParent, "profile-name", szProfilename);
            return true;
        }
        else
            return false;
    }
    
    std::string ClientXML::GetProfileName()
    {
        return GetValue(true, "/general/profile-name", "");
    }

    bool ClientXML::SetGender(int nGender)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutInteger(pParent, "gender", nGender);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetGender(int nDefGender /*= DEFAULT_GENDER*/)
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
            {
                GetInteger(child, "gender", nDefGender);
            }
        }
        return nDefGender;
    }

    bool ClientXML::SetStartMinimized(bool bEnable)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutBoolean(pParent, "start-minimized", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetStartMinimized()
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
            {
                bool bEnabled = false;
                GetBoolean(child, "start-minimized", bEnabled);
                return bEnabled;
            }
        }
        return false;
    }

    bool ClientXML::SetMinimizeToTray(bool bEnable)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutBoolean(pParent, "minimize-to-tray", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetMinimizeToTray()
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
            {
                bool bEnabled = false;
                GetBoolean(child, "minimize-to-tray", bEnabled);
                return bEnabled;
            }
        }
        return false;
    }

    bool ClientXML::SetPushToTalk(bool bEnable)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutBoolean(pParent, "push-to-talk", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetPushToTalk(bool bDefEnable /*= DEFAULT_PUSHTOTALK*/)
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
                GetBoolean(child, "push-to-talk", bDefEnable);
        }
        return bDefEnable;
    }

    bool ClientXML::SetPushToTalkKey(const HotKey& hotkey)
    {
        XMLElement* general = GetGeneralElement();
        if(general)
        {
            XMLElement* newhotkey = ReplaceElement(general, "hotkey");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetPushToTalkKey(HotKey& hotkey)
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* general = root->FirstChildElement("general");
            if(general)
            {
                XMLElement* hkElement = general->FirstChildElement("hotkey");
                if(hkElement)
                {
                    GetHotKey(hkElement, hotkey);
                    return true;
                }
            }
        }
        return false;
    }

    bool ClientXML::SetVoiceActivated(bool bEnable)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutBoolean(pParent, "voice-activated", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetVoiceActivated(bool bDefVoiceAct /*= DEFAULT_VOICEACTIVATED*/)
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
            {
                GetBoolean(child, "voice-activated", bDefVoiceAct);
            }
        }
        return bDefVoiceAct;
    }

    bool ClientXML::SetVoiceActivationLevel(int nLevel)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutInteger(pParent, "voice-activation-level", nLevel);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetVoiceActivationLevel(int nDefVoxLevel/* = DEFAULT_VOICEACTIVATE_LEVEL*/)
    {
        XMLElement* root=m_xmlDocument.RootElement();
        if(root)
        {
            XMLElement* child = root->FirstChildElement("general");
            if(child)
                GetInteger(child, "voice-activation-level", nDefVoxLevel);
        }
        return nDefVoxLevel;
    }

    int ClientXML::SetInactivityDelay(int nDelay)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutInteger(pParent, "inactivity-delay", nDelay);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetInactivityDelay()
    {
        int nValue = 0;
        XMLElement* pGeneral = GetGeneralElement();
        if(pGeneral)
        {
            GetInteger(pGeneral, "inactivity-delay", nValue);
        }
        return nValue;
    }

    bool ClientXML::SetDisableVadOnIdle(bool bEnable)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutBoolean(pParent, "idle-disable-vad", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetDisableVadOnIdle()
    {
        bool bEnabled = false;
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
            GetBoolean(pParent, "idle-disable-vad", bEnabled);
        return bEnabled;
    }

    bool ClientXML::SetDesktopShareMode(int nMode)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutInteger(pParent, "desktop-share-mode", nMode);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetDesktopShareMode()
    {
        int nValue = UNDEFINED;
        XMLElement* pGeneral = GetGeneralElement();
        if(pGeneral)
        {
            GetInteger(pGeneral, "desktop-share-mode", nValue);
        }
        return nValue;
    }

    bool ClientXML::SetDesktopShareRgbMode(int nMode)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutInteger(pParent, "desktop-share-rgbmode", nMode);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetDesktopShareRgbMode()
    {
        int nValue = UNDEFINED;
        XMLElement* pGeneral = GetGeneralElement();
        if(pGeneral)
        {
            GetInteger(pGeneral, "desktop-share-rgbmode", nValue);
        }
        return nValue;
    }

    bool ClientXML::SetDesktopShareUpdateInterval(int nInterval)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutInteger(pParent, "desktop-share-interval", nInterval);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetDesktopShareUpdateInterval()
    {
        int nValue = UNDEFINED;
        XMLElement* pGeneral = GetGeneralElement();
        if(pGeneral)
        {
            GetInteger(pGeneral, "desktop-share-interval", nValue);
        }
        return nValue;
    }

    bool ClientXML::SetFirewallInstall(bool bAsked)
    {
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
        {
            PutBoolean(pParent, "firewall-install", bAsked);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetFirewallInstall(bool bDefValue)
    {
        bool bEnabled = bDefValue;
        XMLElement* pParent = GetGeneralElement();
        if(pParent)
            GetBoolean(pParent, "firewall-install", bEnabled);
        return bEnabled;
    }


    /*************************************************/
    /***************** <window> **********************/
    /*************************************************/
    bool ClientXML::SetFont(const std::string& szFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
    {
        XMLElement* window = GetWindowElement();
        if(window)
        {
            XMLElement* element = ReplaceElement(window, "font");
            PutString(element, "name", szFontName);
            PutInteger(element, "size", nSize);
            PutBoolean(element, "bold", bBold);
            PutBoolean(element, "underline", bUnderline);
            PutBoolean(element, "italic", bItalic);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetFont(string& szFontName, int& nSize, bool& bBold, bool& bUnderline, bool& bItalic)
    {
        XMLElement* window=GetWindowElement();
        if(window)
        {
            XMLElement* font = window->FirstChildElement("font");
            if(font)
            {
                GetString(font, "name", szFontName);
                GetInteger(font, "size", nSize);
                GetBoolean(font, "bold", bBold);
                GetBoolean(font, "underline", bUnderline);
                GetBoolean(font, "italic", bItalic);
                return true;
            }
        }
        return false;
    }

    bool ClientXML::SetPopupOnMessage(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "message-popup", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetPopupOnMessage()
    {
        XMLElement* child = GetWindowElement();
        if(child)
        {
            bool bEnabled = true;
            GetBoolean(child, "message-popup", bEnabled);
            return bEnabled;
        }
        return true;
    }

    bool ClientXML::SetAlwaysOnTop(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "always-on-top", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetAlwaysOnTop()
    {
        XMLElement* child = GetWindowElement();
        if(child)
        {
            bool bEnabled = false;
            GetBoolean(child, "always-on-top", bEnabled);
            return bEnabled;
        }
        return false;
    }

    bool ClientXML::SetShowUserCount(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "show-user-count", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetShowUserCount()
    {
        bool bEnabled = true;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "show-user-count", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetJoinDoubleClick(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "join-dbclick", bEnable);
            return true;
        }
        else
            return false;
    }
    bool ClientXML::GetJoinDoubleClick()
    {
        bool bEnabled = true;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "join-dbclick", bEnabled);

        return bEnabled;
    }
    bool ClientXML::SetDisplayServerName(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "display-servname", bEnable);
            return true;
        }
        else
            return false;
    }
    bool ClientXML::GetDisplayServerName()
    {
        bool bEnabled = true;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "display-servname", bEnabled);

        return bEnabled;
    }
    bool ClientXML::SetQuitClearChannels(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "quit-clear-channels", bEnable);
            return true;
        }
        else
            return false;
    }
    bool ClientXML::GetQuitClearChannels()
    {
        bool bEnabled = true;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "quit-clear-channels", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetMessageTimeStamp(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "message-timestamp", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetMessageTimeStamp()
    {
        bool bEnabled = false;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "message-timestamp", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetLanguageFile(const std::string& szLanguageFile)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutString(pParent, "language-file", szLanguageFile);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetLanguageFile()
    {
        std::string szLanguageFile;
        XMLElement* child = GetWindowElement();
        if(child)
            GetString(child, "language-file", szLanguageFile);

        return szLanguageFile;
    }

    bool ClientXML::SetCloseTransferDialog(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "close-transferdlg", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetCloseTransferDialog()
    {
        bool bEnabled = false;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "close-transferdlg", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetVuMeterUpdate(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "update-vu-meter", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetVuMeterUpdate()
    {
        bool bEnabled = false;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "update-vu-meter", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetCheckApplicationUpdates(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "check-for-updates", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetCheckApplicationUpdates()
    {
        bool bEnabled = true;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "check-for-updates", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetShowUsernames(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "show-usernames", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetShowUsernames()
    {
        bool bEnabled = false;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "show-usernames", bEnabled);

        return bEnabled;
    }
    bool ClientXML::SetMaxTextLength(int nLength)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutInteger(pParent, "max-text-length", nLength);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetMaxTextLength(int nDefault)
    {
        XMLElement* child = GetWindowElement();
        if(child)
            GetInteger(child, "max-text-length", nDefault);

        return nDefault;
    }

    bool ClientXML::SetShowPublicServers(bool bEnable)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutBoolean(pParent, "show-public-servers", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetShowPublicServers()
    {
        bool bEnabled = true;
        XMLElement* child = GetWindowElement();
        if(child)
            GetBoolean(child, "show-public-servers", bEnabled);

        return bEnabled;
    }

    bool ClientXML::SetSortOrder(int nSorting)
    {
        XMLElement* pParent = GetWindowElement();
        if(pParent)
        {
            PutInteger(pParent, "sort-channels", nSorting);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetSortOrder()
    {
        return GetValue(true, "window/sort-channels", 0);
    }

    void ClientXML::SetShowEmojis(bool bEnable)
    {
        SetValueBool("window/show-emojis", bEnable);
    }

    bool ClientXML::GetShowEmojis(bool bDefault)
    {
        return GetValueBool(true, "window/show-emojis", bDefault);
    }

    void ClientXML::SetShowLoggedInOut(bool bEnable)
    {
        SetValueBool("window/show-logged-in-out", bEnable);
    }

    bool ClientXML::GetShowLoggedInOut(bool bDefault)
    {
        return GetValueBool(true, "window/show-logged-in-out", bDefault);
    }

    /***********************************/
    /*********** <client> **************/
    /***********************************/
    bool ClientXML::SetClientTcpPort(int nPort)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutInteger(pParent, "tcpport", nPort);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetClientTcpPort(int nDefPort)
    {
        XMLElement* child = GetClientElement();
        if(child)
            GetInteger(child, "tcpport", nDefPort);
        return nDefPort;
    }

    bool ClientXML::SetClientUdpPort(int nPort)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutInteger(pParent, "udpport", nPort);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetClientUdpPort(int nDefPort)
    {
        XMLElement* child = GetClientElement();
        if(child)
            GetInteger(child, "udpport", nDefPort);
        return nDefPort;
    }

    bool ClientXML::SetAutoConnectToLastest(bool bEnable)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutBoolean(pParent, "auto-connect", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetAutoConnectToLastest()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            bool bEnabled = false;
            GetBoolean(child, "auto-connect", bEnabled);
            return bEnabled;
        }
        return false;
    }

    bool ClientXML::SetReconnectOnDropped(bool bEnable)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutBoolean(pParent, "reconnect", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetReconnectOnDropped()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            bool bEnabled = true;
            GetBoolean(child, "reconnect", bEnabled);
            return bEnabled;
        }
        return true;
    }

    bool ClientXML::SetAutoJoinRootChannel(bool bEnable)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutBoolean(pParent, "auto-join-root", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetAutoJoinRootChannel()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            bool bEnabled = true;
            GetBoolean(child, "auto-join-root", bEnabled);
            return bEnabled;
        }
        return false;
    }

    bool ClientXML::SetAudioLogStorageMode(int mode)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutInteger(pParent, "audio-mode", mode);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetAudioLogStorageMode()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            int nValue = 0;
            GetInteger(child, "audio-mode", nValue);
            return nValue;
        }
        return 0;
    }

    void ClientXML::SetAudioLogStreamTypes(StreamTypes uStreamTypes)
    {
        SetValue("clientsetup/audio-stream-types", uStreamTypes);
    }

    StreamTypes ClientXML::GetAudioLogStreamTypes(StreamTypes uDefaultStreamTypes)
    {
        return GetValue(true, "clientsetup/audio-stream-types", uDefaultStreamTypes);
    }

    bool ClientXML::SetAudioLogStorageFormat(int aff)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutInteger(pParent, "audio-format", aff);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetAudioLogStorageFormat()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            int nValue = 0;
            GetInteger(child, "audio-format", nValue);
            return nValue;
        }
        return 0;
    }

    bool ClientXML::SetAudioLogStorage(const std::string& folder)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutString(pParent, "audio-folder", folder);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetAudioLogStorage()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            string s;
            GetString(child, "audio-folder", s);
            return s;
        }
        return "";
    }

    bool ClientXML::SetChanTextLogStorage(const std::string& folder)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutString(pParent, "channel-text-folder", folder);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetChanTextLogStorage()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            string s;
            GetString(child, "channel-text-folder", s);
            return s;
        }
        return "";
    }

    bool ClientXML::SetUserTextLogStorage(const std::string& folder)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutString(pParent, "user-text-folder", folder);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetUserTextLogStorage()
    {
        XMLElement* child = GetClientElement();
        if(child)
        {
            string s;
            GetString(child, "user-text-folder", s);
            return s;
        }
        return "";
    }


    bool ClientXML::SetDefaultSubscriptions(int subs)
    {
        XMLElement* pParent = GetClientElement();
        if(pParent)
        {
            PutInteger(pParent, "default-subscriptions", subs);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetDefaultSubscriptions()
    {
        XMLElement* child = GetClientElement();
        int nValue = UNDEFINED;
        if(child)
        {
            GetInteger(child, "default-subscriptions", nValue);
        }
        return nValue;
    }

    /****************************************/
    /************** <soundsystem> ***********/
    /****************************************/
    bool ClientXML::SetSoundInputDevice(int nDevice)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutInteger(pParent, "input-driver", nDevice);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetSoundInputDevice(int nDefDeviceId)
    {
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetInteger(child, "input-driver", nDefDeviceId);
        return nDefDeviceId;
    }

    bool ClientXML::SetSoundInputDevice(const std::string& devid)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutString(pParent, "input-device-id", devid);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetSoundInputDevice()
    {
        std::string devid;
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetString(child, "input-device-id", devid);
        return devid;
    }

    bool ClientXML::SetSoundOutputDevice(int nDevice)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutInteger(pParent, "output-driver", nDevice);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetSoundOutputDevice(int nDefDeviceId)
    {
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetInteger(child, "output-driver", nDefDeviceId);
        return nDefDeviceId;
    }

    bool ClientXML::SetSoundOutputDevice(const std::string& devid)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutString(pParent, "output-device-id", devid);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetSoundOutputDevice()
    {
        std::string devid;
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetString(child, "output-device-id", devid);
        return devid;
    }

    bool ClientXML::SetSoundOutputVolume(int nVolume)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutInteger(pParent, "volume", nVolume);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetSoundOutputVolume(int def_vol)
    {
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetInteger(child, "volume", def_vol);
        return def_vol;
    }

    bool ClientXML::SetMediaStreamVsVoice(int nPercent)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutInteger(pParent, "media-vs-voice", nPercent);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetMediaStreamVsVoice(int nDefPercent)
    {
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetInteger(child, "media-vs-voice", nDefPercent);
        return nDefPercent;
    }

    bool ClientXML::SetAutoPositioning(bool bEnable)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutBoolean(pParent, "auto-positioning", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetAutoPositioning()
    {
        bool bEnable = false;
        XMLElement* child = GetSoundSystemElement();
        if(child)
        {
            GetBoolean(child, "auto-positioning", bEnable);
        }
        return bEnable;
    }

    bool ClientXML::SetAGC(bool bEnable)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutBoolean(pParent, "agc", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetAGC(bool bDefValue)
    {
        XMLElement* child = GetSoundSystemElement();
        bool bEnabled = bDefValue;
        if(child)
            GetBoolean(child, "agc", bEnabled);
        return bEnabled;
    }

    bool ClientXML::SetDenoise(bool bEnable)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutBoolean(pParent, "denoise", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetDenoise(bool bDefValue)
    {
        XMLElement* child = GetSoundSystemElement();
        bool bEnabled = bDefValue;
        if(child)
            GetBoolean(child, "denoise", bEnabled);
        return bEnabled;
    }

    bool ClientXML::SetVoiceGain(bool bEnable)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutBoolean(pParent, "voice-gain", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetVoiceGain()
    {
        XMLElement* child = GetSoundSystemElement();
        if(child)
        {
            bool bEnabled = false;
            GetBoolean(child, "voice-gain", bEnabled);
            return bEnabled;
        }
        return false;
    }

    bool ClientXML::SetVoiceGainLevel(int nGain)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutInteger(pParent, "gain-level", nGain);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetVoiceGainLevel(int nDefGain)
    {
        XMLElement* child = GetSoundSystemElement();
        if(child)
            GetInteger(child, "gain-level", nDefGain);
        return nDefGain;
    }

    bool ClientXML::SetEchoCancel(bool bEnable)
    {
        XMLElement* pParent = GetSoundSystemElement();
        if(pParent)
        {
            PutBoolean(pParent, "echo-cancel", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetEchoCancel(bool bDefValue)
    {
        XMLElement* child = GetSoundSystemElement();
        bool bEnabled = bDefValue;
        if(child)
            GetBoolean(child, "echo-cancel", bEnabled);
        return bEnabled;
    }

    /**************** <events> ****************/

    void ClientXML::SetEventSoundsEnabled(unsigned uSoundEvents)
    {
        SetValue("events/enabled-soundevents", int(uSoundEvents));
    }

    unsigned ClientXML::GetEventSoundsEnabled(unsigned uDefEvents)
    {
        uDefEvents = GetValue(true, "events/enabled-soundevents", int(uDefEvents));
        return uDefEvents;
    }

    void ClientXML::SetEventNewUser(const std::string& szPath)
    {
        SetValue("events/newuser", szPath);
    }

    string ClientXML::GetEventNewUser(std::string szDefPath)
    {
        return GetValue(true, "events/newuser", szDefPath);
    }

    void ClientXML::SetEventNewMessage(const std::string& szPath)
    {
        SetValue("events/newmessage", szPath);
    }

    string ClientXML::GetEventNewMessage(std::string szDefPath)
    {
        return GetValue(true, "events/newmessage", szDefPath);
    }

    void ClientXML::SetEventNewMessageSent(const std::string& szPath)
    {
        SetValue("events/newmessagesent", szPath);
    }

    string ClientXML::GetEventNewMessageSent(std::string szDefPath)
    {
        return GetValue(true, "events/newmessagesent", szDefPath);
    }

    void ClientXML::SetEventRemovedUser(const std::string& szPath)
    {
        SetValue("events/removeduser", szPath);
    }

    string ClientXML::GetEventRemovedUser(std::string szDefPath)
    {
        return GetValue(true, "events/removeduser", szDefPath);
    }

    void ClientXML::SetEventUserLoggedIn(const std::string& szPath)
    {
        SetValue("events/userloggedin", szPath);
    }

    string ClientXML::GetEventUserLoggedIn(std::string szDefPath)
    {
        return GetValue(true, "events/userloggedin", szDefPath);
    }

    void ClientXML::SetEventUserLoggedOut(const std::string& szPath)
    {
        SetValue("events/userloggedout", szPath);
    }

    string ClientXML::GetEventUserLoggedOut(std::string szDefPath)
    {
        return GetValue(true, "events/userloggedout", szDefPath);
    }

    void ClientXML::SetEventServerLost(const std::string& szPath)
    {
        SetValue("events/serverlost", szPath);
    }

    string ClientXML::GetEventServerLost(std::string szDefPath)
    {
        return GetValue(true, "events/serverlost", szDefPath);
    }

    void ClientXML::SetEventChannelSilent(const std::string& szPath)
    {
        SetValue("events/userstoppedtalking", szPath);
    }

    std::string ClientXML::GetEventChannelSilent(std::string szDefPath)
    {
        return GetValue(true, "events/userstoppedtalking", szDefPath);
    }

    void ClientXML::SetEventHotKey(const std::string& szPath)
    {
        SetValue("events/hotkeypressed", szPath);
    }

    string ClientXML::GetEventHotKey(std::string szDefPath)
    {
        return GetValue(true, "events/hotkeypressed", szDefPath);
    }

    void ClientXML::SetEventChannelMsg(const std::string& szPath)
    {
        SetValue("events/channelmessage", szPath);
    }

    string ClientXML::GetEventChannelMsg(std::string szDefPath)
    {
        return GetValue(true, "events/channelmessage", szDefPath);
    }

    void ClientXML::SetEventChannelMsgSent(const std::string& szPath)
    {
        SetValue("events/channelmessagesent", szPath);
    }

    string ClientXML::GetEventChannelMsgSent(std::string szDefPath)
    {
        return GetValue(true, "events/channelmessagesent", szDefPath);
    }

    void ClientXML::SetEventBroadcastMsg(const std::string& szPath)
    {
        SetValue("events/broadcastmessage", szPath);
    }

    std::string ClientXML::GetEventBroadcastMsg(std::string szDefPath)
    {
        return GetValue(true, "events/broadcastmessage", szDefPath);
    }

    void ClientXML::SetEventFilesUpd(const std::string& szPath)
    {
        SetValue("events/filesupdates", szPath);
    }

    string ClientXML::GetEventFilesUpd(std::string szDefPath)
    {
        return GetValue(true, "events/filesupdates", szDefPath);
    }

    void ClientXML::SetEventTransferEnd(const std::string& szPath)
    {
        SetValue("events/transferend", szPath);
    }

    string ClientXML::GetEventTransferEnd(std::string szDefPath)
    {
        return GetValue(true, "events/transferend", szDefPath);
    }

    void ClientXML::SetEventVideoSession(const std::string& szPath)
    {
        SetValue("events/new-video-session", szPath);
    }

    std::string ClientXML::GetEventVideoSession(std::string szDefPath)
    {
        return GetValue(true, "events/new-video-session", szDefPath);
    }

    void ClientXML::SetEventDesktopSession(const std::string& szPath)
    {
        SetValue("events/new-desktop-session", szPath);
    }

    std::string ClientXML::GetEventDesktopSession(std::string szDefPath)
    {
        return GetValue(true, "events/new-desktop-session", szDefPath);
    }

    void ClientXML::SetEventQuestionMode(const std::string& szPath)
    {
        SetValue("events/question-mode", szPath);
    }
    
    std::string ClientXML::GetEventQuestionMode(std::string szDefPath)
    {
        return GetValue(true, "events/question-mode", szDefPath);
    }

    void ClientXML::SetEventDesktopAccessReq(const std::string& szPath)
    {
        SetValue("events/desktop-access-req", szPath);
    }

    std::string ClientXML::GetEventDesktopAccessReq(std::string szDefPath)
    {
        return GetValue(true, "events/desktop-access-req", szDefPath);
    }

    void ClientXML::SetEventVoiceActivated(const std::string& szPath)
    {
        SetValue("events/voice-activated", szPath);
    }

    std::string ClientXML::GetEventVoiceActivated(std::string szDefPath)
    {
        return GetValue(true, "events/voice-activated", szDefPath);
    }

    void ClientXML::SetEventVoiceDeactivated(const std::string& szPath)
    {
        SetValue("events/voice-deactivated", szPath);
    }

    std::string ClientXML::GetEventVoiceDeactivated(std::string szDefPath)
    {
        return GetValue(true, "events/voice-deactivated", szDefPath);
    }

    void ClientXML::SetEventEnableVoiceActivation(const std::string& szPath)
    {
        SetValue("events/enable-voice-activation", szPath);
    }

    std::string ClientXML::GetEventEnableVoiceActivation(std::string szDefPath)
    {
        return GetValue(true, "events/enable-voice-activation", szDefPath);
    }

    void ClientXML::SetEventDisableVoiceActivation(const std::string& szPath)
    {
        SetValue("events/disable-voice-activation", szPath);
    }

    std::string ClientXML::GetEventDisableVoiceActivation(std::string szDefPath)
    {
        return GetValue(true, "events/disable-voice-activation", szDefPath);
    }

    void ClientXML::SetEventMeEnableVoiceActivation(const std::string& szPath)
    {
        SetValue("events/me-enable-voice-activation", szPath);
    }

    std::string ClientXML::GetEventMeEnableVoiceActivation(std::string szDefPath)
    {
        return GetValue(true, "events/me-enable-voice-activation", szDefPath);
    }

    void ClientXML::SetEventMeDisableVoiceActivation(const std::string& szPath)
    {
        SetValue("events/me-disable-voice-activation", szPath);
    }

    std::string ClientXML::GetEventMeDisableVoiceActivation(std::string szDefPath)
    {
        return GetValue(true, "events/me-disable-voice-activation", szDefPath);
    }

    void ClientXML::SetEventEnableMuteAll(const std::string& szPath)
    {
        SetValue("events/enable-mute-all", szPath);
    }

    std::string ClientXML::GetEventEnableMuteAll(std::string szDefPath)
    {
        return GetValue(true, "events/enable-mute-all", szDefPath);
    }

    void ClientXML::SetEventDisableMuteAll(const std::string& szPath)
    {
        SetValue("events/disable-mute-all", szPath);
    }

    std::string ClientXML::GetEventDisableMuteAll(std::string szDefPath)
    {
        return GetValue(true, "events/disable-mute-all", szDefPath);
    }

    void ClientXML::SetEventTransmitQueueHead(std::string szPath)
    {
        SetValue("events/transmit-queue-front", szPath);
    }
    
    std::string ClientXML::GetEventTransmitQueueHead(std::string szDefPath)
    {
        return GetValue(true, "events/transmit-queue-front", szDefPath);
    }

    void ClientXML::SetEventTransmitQueueStop(std::string szPath)
    {
        SetValue("events/transmit-queue-stop", szPath);
    }

    std::string ClientXML::GetEventTransmitQueueStop(std::string szDefPath)
    {
        return GetValue(true, "events/transmit-queue-stop", szDefPath);
    }

    void ClientXML::SetEventTTSEvents(TTSEvents uEvents)
    {
        SetValue("events/text-to-speech-events", int(uEvents));
    }
    
    TTSEvents ClientXML::GetEventTTSEvents()
    {
        return TTSEvents(GetValue(true, "events/text-to-speech-events", TTS_ALL));
    }

    void ClientXML::SetClientSoundsVsVoice(int nPercent)
    {
        SetValue("events/client-sounds-vs-voice", nPercent);
    }

    int ClientXML::GetClientSoundsVsVoice(int nDefPercent)
    {
        return GetValue(true, "events/client-sounds-vs-voice", nDefPercent);
    }

    void ClientXML::SetSoundPlaybackMode(int pbmode)
    {
        SetValue("events/playback-mode", pbmode);
    }

    int ClientXML::GetSoundPlaybackMode(int nDefPbMode)
    {
        return GetValue(true, "events/playback-mode", nDefPbMode);
    }

    /********* <advanced> ************/

    bool ClientXML::SetLowLevelMouseHook(bool bEnable)
    {
        XMLElement* pParent = GetAdvancedElement();
        if(pParent)
        {
            PutBoolean(pParent, "lowlevel-mousehook", bEnable);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetLowLevelMouseHook()
    {
        XMLElement* child = GetAdvancedElement();
        if(child)
        {
            bool bEnabled = false;
            GetBoolean(child, "lowlevel-mousehook", bEnabled);
            return bEnabled;
        }
        return false;
    }

    /*********** </advanced> ***************/

    /********* <shortcuts> **********/
    bool ClientXML::SetHotKeyVoiceAct(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "voice-act");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyVoiceAct(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("voice-act")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    bool ClientXML::SetHotKeyVolumePlus(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "increase-volume");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyVolumePlus(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("increase-volume")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    bool ClientXML::SetHotKeyVolumeMinus(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "lower-volume");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyVolumeMinus(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("lower-volume")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    bool ClientXML::SetHotKeyMuteAll(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "mute-all");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyMuteAll(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("mute-all")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    bool ClientXML::SetHotKeyVoiceGainPlus(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "increase-gain");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyVoiceGainPlus(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("increase-gain")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    bool ClientXML::SetHotKeyVoiceGainMinus(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "lower-gain");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyVoiceGainMinus(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("lower-gain")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    bool ClientXML::SetHotKeyMinRestore(const HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child)
        {
            XMLElement* newhotkey = ReplaceElement(child, "min-restore");
            PutHotKey(newhotkey, hotkey);
            return true;
        }
        return false;
    }

    bool ClientXML::GetHotKeyMinRestore(HotKey& hotkey)
    {
        XMLElement* child = GetShortCutsElement();
        if(child && (child = child->FirstChildElement("min-restore")))
        {
            return GetHotKey(child, hotkey);
        }
        return false;
    }

    /********* </shortcuts> *********/

    /********* <videocapture> *********/
    bool ClientXML::SetVideoCaptureDevice(const std::string& viddev)
    {
        XMLElement* pParent = GetVideoElement();
        if(pParent)
        {
            PutString(pParent, "videodevice", viddev);
            return true;
        }
        else
            return false;
    }

    std::string ClientXML::GetVideoCaptureDevice()
    {
        XMLElement* child = GetVideoElement();
        string s;
        if(child)
            GetString(child, "videodevice", s);
        return s;
    }

    bool ClientXML::SetVideoCaptureFormat(int index)
    {
        XMLElement* pParent = GetVideoElement();
        if(pParent)
        {
            PutInteger(pParent, "capture-format", index);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetVideoCaptureFormat(int nDefIndex)
    {
        XMLElement* child = GetVideoElement();
        if(child)
            GetInteger(child, "capture-format", nDefIndex);
        return nDefIndex;
    }

    bool ClientXML::SetVideoCaptureFormat(const VideoFormat& capformat)
    {
        XMLElement* child = GetVideoElement();
        if(child && capformat.nWidth)
        {
            XMLElement* capfmt = ReplaceElement(child, "videoformat");
            PutInteger(capfmt, "fourcc", capformat.picFourCC);
            PutInteger(capfmt, "width", capformat.nWidth);
            PutInteger(capfmt, "height", capformat.nHeight);
            PutInteger(capfmt, "fps-numerator", capformat.nFPS_Numerator);
            PutInteger(capfmt, "fps-denominator", capformat.nFPS_Denominator);
            return true;
        }
        return false;
    }

    bool ClientXML::GetVideoCaptureFormat(VideoFormat& capformat)
    {
        XMLElement* child = GetVideoElement();
        XMLElement* capElem = child->FirstChildElement("videoformat");
        if(capElem)
        {
            int c = 0;
            GetInteger(capElem, "fourcc", c);
            capformat.picFourCC = (FourCC)c;
            GetInteger(capElem, "width", capformat.nWidth);
            GetInteger(capElem, "height", capformat.nHeight);
            GetInteger(capElem, "fps-numerator", capformat.nFPS_Numerator);
            GetInteger(capElem, "fps-denominator", capformat.nFPS_Denominator);
            return true;
        }
        return false;
    }

    bool ClientXML::SetVideoCodecBitrate(int bitrate)
    {
        XMLElement* pParent = GetVideoElement();
        if(pParent)
        {
            PutInteger(pParent, "webm-vp8-bitrate", bitrate);
            return true;
        }
        else
            return false;
    }

    int ClientXML::GetVideoCodecBitrate(int nDefBitrate)
    {
        XMLElement* child = GetVideoElement();
        if(child)
            GetInteger(child, "webm-vp8-bitrate", nDefBitrate);
        return nDefBitrate;
    }

    bool ClientXML::SetVideoCaptureEnabled(bool enabled)
    {
        XMLElement* pParent = GetVideoElement();
        if(pParent)
        {
            PutBoolean(pParent, "video-capture-enabled", enabled);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::GetVideoCaptureEnabled(bool def_value)
    {
        XMLElement* child = GetVideoElement();
        if(child)
            GetBoolean(child, "video-capture-enabled", def_value);
        return def_value;
    }

    /********* </videocapture> *********/


    /******** <latesthosts> ************/
    bool ClientXML::AddLatestHostEntry(const HostEntry& entry)
    {
        XMLElement* latest = GetLatestHostsElement();
        if(latest)
        {
            XMLElement* element = AppendElement(latest, "host");
            PutString(element, "address", entry.szAddress);
            PutInteger(element, "tcpport", entry.nTcpPort);
            PutInteger(element, "udpport", entry.nUdpPort);
            PutBoolean(element, "encrypted", entry.bEncrypted);
            PutString(element, "username", entry.szUsername);
            PutString(element, "password", entry.szPassword);
            PutString(element, "channel", entry.szChannel);
            PutString(element, "cpassword", entry.szChPasswd);
            return true;
        }
        else
            return false;
    }

    bool ClientXML::RemoveLatestHostEntry(const HostEntry& entry)
    {
        XMLElement* parent = GetLatestHostsElement();
        if(parent)
        {
            for(XMLElement* child = parent->FirstChildElement("host");
                child;
                child = child->NextSiblingElement("host"))
            {
                string address, srvpasswd, username, password, channel, chpasswd;
                int soundport, port;
                bool encrypted;
                GetString(child, "address", address);
                //GetString(child, "password", password);
                GetInteger(child, "tcpport", port);
                GetInteger(child, "udpport", soundport);
                GetBoolean(child, "encrypted", encrypted);
                GetString(child, "username", username);
                GetString(child, "password", password);
                GetString(child, "channel", channel);
                GetString(child, "cpassword", chpasswd);
                if(address == entry.szAddress &&
                    //password == entry.szPassword &&
                    port == entry.nTcpPort &&
                    soundport == entry.nUdpPort &&
                    encrypted == entry.bEncrypted &&
                    username == entry.szUsername &&
                    password == entry.szPassword &&
                    channel == entry.szChannel &&
                    chpasswd == entry.szChPasswd )
                {
                    parent->DeleteChild(child);
                    break;
                }
            }
            return true;
        }
        return false;
    }

    int ClientXML::GetLatestHostEntryCount()
    {
        int count = 0;
        XMLElement* item = GetLatestHostsElement();
        if(item)
            for(XMLElement* child = item->FirstChildElement("host");
                child;
                child = child->NextSiblingElement("host"))
            {
                count++;
            }

            return count;
    }

    bool ClientXML::GetLatestHostEntry(int index, HostEntry& entry)
    {
        bool found = false;
        int i = 0;
        XMLElement* item = GetLatestHostsElement();
        if(item)
        {
            XMLElement* child = NULL;
            for(child = item->FirstChildElement("host");
                child;
                child = child->NextSiblingElement("host"))
                if(i==index)
                    break;
                else
                    i++;

            if(child && i==index)
            {
                found = true;
                HostEntry tmp;
                found &= GetString(child, "address", tmp.szAddress);
                found &= GetInteger(child, "tcpport", tmp.nTcpPort);
                found &= GetInteger(child, "udpport", tmp.nUdpPort);
                GetBoolean(child, "encrypted", tmp.bEncrypted);
                found &= GetString(child, "username", tmp.szUsername);
                found &= GetString(child, "password", tmp.szPassword);
                found &= GetString(child, "channel", tmp.szChannel);
                found &= GetString(child, "cpassword", tmp.szChPasswd);

                if(found)
                    entry = tmp;
            }
            else found = false;
        }

        return found;
    }

    /**** </latest-hosts> *****/

    /********** <mediafiles> *********/
    bool ClientXML::SetLastMediaFiles(const std::vector<std::string>& filenames)
    {
        XMLElement* parent = GetMediaFilesElement();
        if(parent)
        {
            XMLElement* child = parent->FirstChildElement();
            while (child)
            {
                parent->DeleteChild(child);
                child = parent->FirstChildElement();
            }

            for (auto s : filenames)
            {
                XMLElement* element = AppendElement(parent, "last-media-file");
                PutElementText(element, s);
            }
            return true;
        }
        else
            return false;
    }

    std::vector<std::string> ClientXML::GetLastMediaFiles()
    {
        std::vector<std::string> result;

        XMLElement* parent = GetMediaFilesElement();
        if (!parent)
            return result;

        XMLElement* child = parent->FirstChildElement();
        while (child)
        {
            std::string s;
            GetElementText(child, s);
            if (s.size())
                result.push_back(s);
            child = child->NextSiblingElement("last-media-file");
        }
        return result;
    }
    /********** </mediafiles> *********/

    void ClientXML::SetAudioPreprocessor(AudioPreprocessorType preproc)
    {
        SetValue("streammedia/audiopreprocessor", preproc);
    }
    
    AudioPreprocessorType ClientXML::GetAudioPreprocessor(AudioPreprocessorType defaultvalue)
    {
        return AudioPreprocessorType(GetValue(true, "streammedia/audiopreprocessor", int(defaultvalue)));
    }

    void ClientXML::SetTTAudioPreprocessor(const TTAudioPreprocessor& ttaud)
    {
        SetValue("streammedia/ttaudiopreprocessor/gain-level", ttaud.nGainLevel);
        SetValueBool("streammedia/ttaudiopreprocessor/mute-left", ttaud.bMuteLeftSpeaker);
        SetValueBool("streammedia/ttaudiopreprocessor/mute-right", ttaud.bMuteRightSpeaker);
    }
    
    TTAudioPreprocessor ClientXML::GetTTAudioPreprocessor()
    {
        TTAudioPreprocessor ttaud = {};
        ttaud.nGainLevel = GetValue(true, "streammedia/ttaudiopreprocessor/gain-level", SOUND_GAIN_DEFAULT);
        ttaud.bMuteLeftSpeaker = GetValueBool(true, "streammedia/ttaudiopreprocessor/mute-left", false);
        ttaud.bMuteRightSpeaker = GetValueBool(true, "streammedia/ttaudiopreprocessor/mute-right", false);
        return ttaud;
    }

    void ClientXML::SetSpeexDSPAudioPreprocessor(const SpeexDSP& spxdsp)
    {
        SetValueBool("streammedia/speexdspaudiopreprocessor/agc", spxdsp.bEnableAGC);
        SetValue("streammedia/speexdspaudiopreprocessor/gain-level", spxdsp.nGainLevel);
        SetValue("streammedia/speexdspaudiopreprocessor/gain-max", spxdsp.nMaxGainDB);
        SetValue("streammedia/speexdspaudiopreprocessor/gain-inc-sec", spxdsp.nMaxIncDBSec);
        SetValue("streammedia/speexdspaudiopreprocessor/gain-dec-sec", spxdsp.nMaxDecDBSec);
        SetValueBool("streammedia/speexdspaudiopreprocessor/denoise", spxdsp.bEnableDenoise);
        SetValue("streammedia/speexdspaudiopreprocessor/denoise-max", spxdsp.nMaxNoiseSuppressDB);
    }
    
    SpeexDSP ClientXML::GetSpeexDSPAudioPreprocessor()
    {
        SpeexDSP dsp = {};
        dsp.bEnableAGC = GetValueBool(true, "streammedia/speexdspaudiopreprocessor/agc", DEFAULT_SPEEXDSP_AGC_ENABLE);
        dsp.nGainLevel = GetValue(true, "streammedia/speexdspaudiopreprocessor/gain-level", DEFAULT_SPEEXDSP_AGC_GAINLEVEL);
        dsp.nMaxGainDB = GetValue(true, "streammedia/speexdspaudiopreprocessor/gain-max", DEFAULT_SPEEXDSP_AGC_GAINMAXDB);
        dsp.nMaxIncDBSec = GetValue(true, "streammedia/speexdspaudiopreprocessor/gain-inc-sec", DEFAULT_SPEEXDSP_AGC_INC_MAXDB);
        dsp.nMaxDecDBSec = GetValue(true, "streammedia/speexdspaudiopreprocessor/gain-dec-sec", DEFAULT_SPEEXDSP_AGC_DEC_MAXDB);
        dsp.bEnableDenoise = GetValueBool(true, "streammedia/speexdspaudiopreprocessor/denoise", DEFAULT_SPEEXDSP_DENOISE_ENABLE);
        dsp.nMaxNoiseSuppressDB = GetValue(true, "streammedia/speexdspaudiopreprocessor/denoise-max", DEFAULT_SPEEXDSP_DENOISE_SUPPRESS);
        return dsp;
    }

    void ClientXML::SetMediaFilePlayback(const MediaFilePlayback& mfp)
    {
        SetValue("streammedia/mediafileplayback/offset", mfp.uOffsetMSec);
        SetAudioPreprocessor(mfp.audioPreprocessor.nPreprocessor);
        switch(mfp.audioPreprocessor.nPreprocessor)
        {
        case SPEEXDSP_AUDIOPREPROCESSOR :
            SetSpeexDSPAudioPreprocessor(mfp.audioPreprocessor.speexdsp);
            break;
        case TEAMTALK_AUDIOPREPROCESSOR :
            SetTTAudioPreprocessor(mfp.audioPreprocessor.ttpreprocessor);
            break;
        case NO_AUDIOPREPROCESSOR : break;
        }
    }

    MediaFilePlayback ClientXML::GetMediaFilePlayback()
    {
        MediaFilePlayback mfp = {};
        mfp.bPaused = FALSE;
        mfp.uOffsetMSec = GetValue(true, "streammedia/mediafileplayback/offset", TT_MEDIAPLAYBACK_OFFSET_IGNORE);
        mfp.audioPreprocessor.nPreprocessor = GetAudioPreprocessor(NO_AUDIOPREPROCESSOR);
        switch(mfp.audioPreprocessor.nPreprocessor)
        {
        case SPEEXDSP_AUDIOPREPROCESSOR :
            mfp.audioPreprocessor.speexdsp = GetSpeexDSPAudioPreprocessor();
            break;
        case TEAMTALK_AUDIOPREPROCESSOR :
            mfp.audioPreprocessor.ttpreprocessor = GetTTAudioPreprocessor();
            break;
        case NO_AUDIOPREPROCESSOR : break;
        }
        return mfp;
    }

    void ClientXML::SetVideoCodec(const VideoCodec& codec)
    {
        SetValue("streammedia/videocodec", codec.nCodec);
        switch (codec.nCodec)
        {
        case WEBM_VP8_CODEC :
            SetValue("streammedia/webmvp8/encdeadline", codec.webm_vp8.nEncodeDeadline);
            SetValue("streammedia/webmvp8/targetbitrate", codec.webm_vp8.nRcTargetBitrate);
            break;
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
        case NO_CODEC :
            break;
        }
    }
    
    VideoCodec ClientXML::GetVideoCodec()
    {
        VideoCodec codec = {};
        codec.nCodec = Codec(GetValue(true, "streammedia/videocodec", NO_CODEC));
        switch (codec.nCodec)
        {
        case WEBM_VP8_CODEC :
            codec.webm_vp8.nEncodeDeadline = GetValue(true, "streammedia/webmvp8/encdeadline", 0);
            codec.webm_vp8.nRcTargetBitrate = GetValue(true, "streammedia/webmvp8/targetbitrate", 0);
            break;
        case SPEEX_CODEC:
        case SPEEX_VBR_CODEC:
        case OPUS_CODEC:
        case NO_CODEC:
            break;
        }
        return codec;
    }

    void ClientXML::SetMediaFileRepeat(bool repeat)
    {
        SetValueBool("streammedia/repeat", repeat);
    }

    bool ClientXML::GetMediaFileRepeat(bool defaultvalue)
    {
        return GetValueBool(true, "streammedia/repeat", defaultvalue);
    }

    void ClientXML::PutHotKey(XMLElement* parent, const HotKey& hotkey)
    {
        for(size_t i=0;i<hotkey.size();i++)
            PutInteger(parent, string("key") + i2str((int)i), hotkey[i]);
    }

    bool ClientXML::GetHotKey(const XMLElement* parent, HotKey& hotkey)
    {
        assert(hotkey.empty());
        bool b = true;
        int i = 0;
        int key = 0;
        while(GetInteger(parent, string("key") + i2str(i++), key))
            hotkey.push_back(key);
        return hotkey.size()?true:false;
    }



    /************************/
    /* Parsing of .tt files */
    /************************/
    XMLElement* TTFile::GetRootElement()
    {
        return m_xmlDocument.RootElement();
    }


    void TTFile::SetHostEntry(const HostEntry& entry)
    {
        XMLElement* item=m_xmlDocument.RootElement();
        if(!item)
            return;

        XMLElement* hostElement = m_xmlDocument.NewElement("host");
        item->InsertEndChild(hostElement);

        PutString(hostElement, "name", entry.szEntryName);
        PutString(hostElement, "address", entry.szAddress);
        PutInteger(hostElement, "tcpport", entry.nTcpPort);
        PutInteger(hostElement, "udpport", entry.nUdpPort);
        PutBoolean(hostElement, "encrypted", entry.bEncrypted);

        XMLElement* auth = AppendElement(hostElement, "auth");
        PutString(auth, "username", entry.szUsername);
        PutString(auth, "password", entry.szPassword);

        XMLElement* join = AppendElement(hostElement, "join");
        PutString(join, "channel", entry.szChannel);
        PutString(join, "password", entry.szChPasswd);

        bool hasClientSetup = entry.szNickname.size() || entry.nGender ||
                              entry.hotkey.size() || entry.capformat.nWidth ||
                              entry.vidcodec.nCodec != NO_CODEC;
        if(hasClientSetup)
        {
            XMLElement* client = AppendElement(hostElement, "clientsetup");
            if(entry.szNickname.size())
                PutString(client, "nickname", entry.szNickname);
            if(entry.nGender)
                PutInteger(client, "gender", entry.nGender);
            if(entry.hotkey.size())
            {
                XMLElement* hotkey = AppendElement(client, "win-hotkey");
                for(size_t i=0;i<entry.hotkey.size();i++)
                {
                    XMLElement* newelement = m_xmlDocument.NewElement("key");
                    XMLText* text = m_xmlDocument.NewText(i2str(entry.hotkey[i]).c_str());
                    newelement->InsertEndChild(text);
                    hotkey->InsertEndChild(newelement);
                }
            }
            PutInteger(client, "voice-activated", entry.nVoiceAct);

            if(entry.capformat.nWidth)
            {
                XMLElement* capfmt = AppendElement(client, "videoformat");
                PutInteger(capfmt, "fourcc", entry.capformat.picFourCC);
                PutInteger(capfmt, "width", entry.capformat.nWidth);
                PutInteger(capfmt, "height", entry.capformat.nHeight);
                PutInteger(capfmt, "fps-numerator", entry.capformat.nFPS_Numerator);
                PutInteger(capfmt, "fps-denominator", entry.capformat.nFPS_Denominator);
            }

            if(entry.vidcodec.nCodec != NO_CODEC)
            {
                XMLElement* vidcodec = AppendElement(client, "videocodec");
                PutInteger(vidcodec, "codec", entry.vidcodec.nCodec);
                PutInteger(vidcodec, "webm-vp8-bitrate", entry.vidcodec.webm_vp8.nRcTargetBitrate);
            }
        }
    }

    bool TTFile::GetHostEntry(HostEntry& entry, int i)
    {
        XMLElement* item=m_xmlDocument.RootElement();
        bool found = false;
        if(item)
        {
            item = item->FirstChildElement("host");
            while(item && i >= 0)
            {
                bool ok = true;
                ok &= GetString(item, "name", entry.szEntryName);
                ok &= GetString(item, "address", entry.szAddress);
                ok &= GetInteger(item, "tcpport", entry.nTcpPort);
                ok &= GetInteger(item, "udpport", entry.nUdpPort);
                GetBoolean(item, "encrypted", entry.bEncrypted);

                //check auth settings
                XMLElement* auth = item->FirstChildElement("auth");
                if(auth)
                {
                    GetString(auth, "username", entry.szUsername);
                    GetString(auth, "password", entry.szPassword);
                }
                XMLElement* join = item->FirstChildElement("join");
                if(join)
                {
                    GetString(join, "channel", entry.szChannel);
                    GetString(join, "password", entry.szChPasswd);
                }
                XMLElement* client = item->FirstChildElement("clientsetup");
                if(client)
                {

                    GetString(client, "nickname", entry.szNickname);
                    GetInteger(client, "gender", entry.nGender);

                    XMLElement* hotkey = client->FirstChildElement("win-hotkey");
                    if(hotkey)
                    {
                        XMLElement* key = hotkey->FirstChildElement("key");
                        while(key)
                        {
                            string v;
                            GetElementText(key, v);
                            entry.hotkey.push_back(str2i(v));
                            key = key->NextSiblingElement("key");
                        }
                    }
                    GetInteger(client, "voice-activated", entry.nVoiceAct);

                    XMLElement* capformat = client->FirstChildElement("videoformat");
                    if(capformat)
                    {
                        int c = 0;
                        GetInteger(capformat, "fourcc", c);
                        entry.capformat.picFourCC = (FourCC)c;

                        GetInteger(capformat, "width", entry.capformat.nWidth);
                        GetInteger(capformat, "height", entry.capformat.nHeight);
                        GetInteger(capformat, "fps-numerator", entry.capformat.nFPS_Numerator);
                        GetInteger(capformat, "fps-denominator", entry.capformat.nFPS_Denominator);
                    }

                    XMLElement* vidcodec = client->FirstChildElement("videocodec");
                    if(vidcodec)
                    {
                        int c = NO_CODEC;
                        GetInteger(vidcodec, "codec", c);
                        entry.vidcodec.nCodec = (Codec)c;
                        switch(entry.vidcodec.nCodec)
                        {
                        case WEBM_VP8_CODEC :
                            GetInteger(vidcodec, "webm-vp8-bitrate", entry.vidcodec.webm_vp8.nRcTargetBitrate);
                            entry.vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
                            break;
                        }
                    }
                }
                
                found = ok && i == 0;
                item = item->NextSiblingElement("host");
                i--;
            }
        }
        return found;
    }
    
    bool TTFile::HasClientSetup()    {
        XMLElement* item=m_xmlDocument.RootElement();
        if(item)
        {
            item = item->FirstChildElement("host");
            XMLElement* client = item->FirstChildElement("clientsetup");
            return client != NULL;
        }
        return false;
    }

}

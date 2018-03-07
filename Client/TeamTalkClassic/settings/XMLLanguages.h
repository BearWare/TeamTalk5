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

#pragma once

#include <settings/Settings.h>

#define XML_LANGUAGE_VERSION "1.0"

class XMLLanguages : public teamtalk::XMLDocument
{
public:
    XMLLanguages();
    ~XMLLanguages();

    bool CreateFile(const std::string& szFileName);

    void AddItem(int id, std::string szText);
    void RemoveItem(int id);

    std::string GetItemText(int id);

    int GetFirstItem();
    int GetNextItem(int curid);
    int GetPrevItem(int curid);

    void SortItems();

protected:
  virtual TiXmlElement* GetRootElement();
private:
    TiXmlElement* GetItem(int id);
};

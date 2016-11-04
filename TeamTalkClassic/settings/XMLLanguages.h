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
 * This source code is part of the TeamTalk 5 Conferencing System
 * owned by BearWare.dk. Unauthorized use is prohibited. No changes
 * are allowed to this file without the copyright owner's written
 * consent. Using this file requires a source code license issued by
 * BearWare.dk.
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

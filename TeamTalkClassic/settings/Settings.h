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
 * This source code is part of the TeamTalk Conferencing System
 * owned by BearWare.dk. Unauthorized use is prohibited. No changes
 * are allowed to this file without the copyright owner's written
 * consent. Using this file requires a source code license issued by
 * BearWare.dk.
 *
 */

#if !defined(SETTINGS_H)
#define SETTINGS_H

#include <tinyxml.h>
#include <string>
#include <vector>
#include <map>

#include <time.h>


#if defined(_MSC_VER)
#define _INT64 __int64
#else
#include <stdint.h>
#define _INT64 int64_t
#endif

#ifndef UNDEFINED
#define UNDEFINED -1
#endif

#if defined(CreateFile)
#undef CreateFile
#endif

/* 
* Host Manager elements
* <hostmanager>
*  <host name="blah">
*   <address/>
*   <password/>
*   <tcpport/>
*   <udpport/>
*  </host>
* </hostmanager>
*/

namespace teamtalk {

    class XMLDocument  
    {
    public:
        XMLDocument(const std::string& rootname, const std::string& version);
        virtual ~XMLDocument();

        virtual bool CreateFile(const std::string& filename);
        bool LoadFile(const std::string& filename);
        bool SaveFile();
        bool HasErrors();
        std::string GetError();
        bool Parse(const std::string& xml);

        bool SetFileVersion(const std::string& version);
        std::string GetFileVersion();

        const std::string& GetFileName() const { return m_filename; }

        std::string GetValue(const std::string& path);
    protected:
        TiXmlDocument m_xmlDocument;
        void PutElementText(TiXmlElement& element, const std::string& value);
        void GetElementText(const TiXmlElement& element, std::string& value) const;

        void PutBoolean(TiXmlElement& parent, const std::string& szName, bool bValue);
        void PutString(TiXmlElement& parent, const std::string& szName, const std::string& szValue);
        void PutInteger(TiXmlElement& parent, const std::string& szName, int nValue);
        void PutInteger(TiXmlElement& parent, const std::string& szName, _INT64 nValue);

        bool GetBoolean(const TiXmlElement& parent, const std::string& szName, bool& bValue) const;
        bool GetString(const TiXmlElement& parent, const std::string& szName, std::string& szValue) const;
        bool GetInteger(const TiXmlElement& parent, const std::string& szName, int& nValue) const;
        bool GetInteger(const TiXmlElement& parent, const std::string& szName, _INT64& nValue) const;

        TiXmlElement* AppendElement(TiXmlElement& parent, const TiXmlElement& newElement);
        TiXmlElement* ReplaceElement(TiXmlElement& target, const TiXmlElement& element);
        virtual TiXmlElement* GetRootElement();
        std::string m_rootname, m_filename, m_xmlversion;
    };

}

#endif // !defined(SETTINGS_H)

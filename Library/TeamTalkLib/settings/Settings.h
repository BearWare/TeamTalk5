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
        virtual bool SaveFile();
        bool HasErrors();
        std::string GetError();
        bool Parse(const std::string& xml);

        bool SetFileVersion(const std::string& version);
        std::string GetFileVersion();

        const std::string& GetFileName() const { return m_filename; }

        void SetValue(const std::string& path, const std::string& value);
        std::string GetValue(bool prefixRoot, const std::string& path, const std::string& defaultvalue);

        void SetValue(const std::string& path, int value);
        int GetValue(bool prefixRoot, const std::string& path, int defaultvalue);

        void SetValueBool(const std::string& path, bool value);
        bool GetValueBool(bool prefixRoot, const std::string& path, bool defaultvalue);

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

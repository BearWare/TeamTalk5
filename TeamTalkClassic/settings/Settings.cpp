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

#include "Settings.h"
#include "StringPrintf.h"
#include "MyStd.h"

using namespace std;

namespace teamtalk {

    XMLDocument::XMLDocument(const std::string& rootname, const std::string& version)
        : m_rootname(rootname)
        , m_xmlversion(version)
    {
    }

    XMLDocument::~XMLDocument()
    {
    }

    bool XMLDocument::HasErrors()
    {
        return m_xmlDocument.Error();
    }

    string XMLDocument::GetError()
    {
        string error;
        StringPrintf(error, "%s. Line %d column %d.", m_xmlDocument.ErrorDesc(), m_xmlDocument.ErrorCol(), m_xmlDocument.ErrorRow());
        return error;
    }

    bool XMLDocument::Parse(const std::string& xml)
    {
        m_xmlDocument.Clear();
        return m_xmlDocument.Parse(xml.c_str(), 0, TIXML_ENCODING_UNKNOWN) == NULL;
    }


    bool XMLDocument::CreateFile(const std::string& filename)
    {
        m_xmlDocument.Clear();
        string szXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<" + m_rootname + " version=\"" + m_xmlversion + "\">"
            "</" + m_rootname + ">";

        m_xmlDocument.Parse(szXml.c_str(), 0, TIXML_ENCODING_UTF8);
        return m_xmlDocument.SaveFile(filename.c_str()) && LoadFile(filename.c_str());
    }

    bool XMLDocument::SetFileVersion(const std::string& version)
    {
        TiXmlElement* item=GetRootElement();

        if(item)
        {
            item->SetAttribute("version", version.c_str());
            return true;
        }
        return false;
    }

    string XMLDocument::GetFileVersion()
    {
        TiXmlElement* item=GetRootElement();

        string version;
        if(item)
            version = item->Attribute("version");

        return version;
    }

    std::string XMLDocument::GetValue(const std::string& path)
    {
        TiXmlElement* item = GetRootElement();
        stdstrings_t tokens = stdtokenize(path, "/");
        if(item && tokens.size())
        {
            if(item->Value() != tokens[0])
                tokens.clear();
            else
                tokens.erase(tokens.begin());
        }

        while(item && tokens.size())
        {
            item = item->FirstChildElement(tokens[0].c_str());
            tokens.erase(tokens.begin());
        }
        string value;
        if(tokens.empty() && item)
            GetElementText(*item, value);
        return value;
    }


    bool XMLDocument::LoadFile(const std::string& filename)
    {
        if(m_xmlDocument.LoadFile(filename.c_str()))
        {
            m_filename = filename;
            return true;
        }
        return false;
    }

    bool XMLDocument::SaveFile()
    {
        return m_xmlDocument.SaveFile();
    }

    void XMLDocument::PutElementText(TiXmlElement& element, const std::string& value)
    {
        TiXmlText text(value.c_str());
        element.InsertEndChild(text);
    }

    void XMLDocument::GetElementText(const TiXmlElement& element, string& value) const
    {
        //if string == "" text is null apparently
        if(element.FirstChild())
        {
            const TiXmlText* text = element.FirstChild()->ToText();
            if(text)
                value = text->Value();
            else
                value = "";
        }
        else
            value = "";
    }

    void XMLDocument::PutBoolean(TiXmlElement& parent, const string& szName, bool bValue)
    {
        TiXmlElement newelement(szName.c_str());
        TiXmlText text(bValue? "true" : "false");
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    void XMLDocument::PutString(TiXmlElement& parent, const string& szName, const string& szValue)
    {
        TiXmlElement newelement(szName.c_str());
        TiXmlText text(szValue.c_str());
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    void XMLDocument::PutInteger(TiXmlElement& parent, const string& szName, int nValue)
    {
        TiXmlElement newelement(szName.c_str());
        string s = i2str(nValue);

        TiXmlText text(s.c_str());
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    void XMLDocument::PutInteger(TiXmlElement& parent, const string& szName, _INT64 nValue)
    {
        TiXmlElement newelement(szName.c_str());
        string s = i2str(nValue);

        TiXmlText text(s.c_str());
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    TiXmlElement* XMLDocument::AppendElement(TiXmlElement& parent, const TiXmlElement& newElement)
    {
        TiXmlElement* pElement = parent.InsertEndChild(newElement)->ToElement();
        return pElement;
    }

    bool XMLDocument::GetBoolean(const TiXmlElement& parent, const string& szName, bool& bValue) const
    {
        const TiXmlElement* item = parent.FirstChildElement(szName.c_str());
        if(item)
        {
            string s;
            GetElementText(*item, s);

            if(strcmpnocase(s, "true"))
            {
                bValue = true;
                return true;
            }
            else
                if(strcmpnocase(s,"false"))
                {
                    bValue = false;
                    return true;
                }
        }

        return false;
    }

    bool XMLDocument::GetString(const TiXmlElement& parent, const string& szName, string& szValue) const
    {
        const TiXmlElement* item = parent.FirstChildElement(szName.c_str());
        if(item)
        {
            GetElementText(*item, szValue);
            return true;
        }

        return false;
    }

    bool XMLDocument::GetInteger(const TiXmlElement& parent, const string& szName, int& nValue) const
    {
        const TiXmlElement* item = parent.FirstChildElement(szName.c_str());

        if(item)
        {
            string s;
            GetElementText(*item, s);
            nValue = str2i(s);
            return true;
        }
        return false;
    }

    bool XMLDocument::GetInteger(const TiXmlElement& parent, const string& szName, _INT64& nValue) const
    {
        const TiXmlElement* item = parent.FirstChildElement(szName.c_str());

        if(item)
        {
            string s;
            GetElementText(*item, s);
            nValue = str2i64(s);
            return true;
        }
        return false;
    }

    TiXmlElement* XMLDocument::ReplaceElement(TiXmlElement& target, const TiXmlElement& element)
    {
        TiXmlElement* pResult = NULL;
        TiXmlElement* existing = target.FirstChildElement(element.Value());
        if(existing && existing->FirstAttribute())
        {
            bool found = false;
            for(;existing && !found;existing=existing->NextSiblingElement())
            {
                bool match = true;
                const TiXmlAttribute* attr = element.FirstAttribute();
                for(;attr && match;attr=attr->Next())
                {
                    const char* val1 = existing->Attribute(attr->Name());
                    const char* val2 = attr->Value();
                    if(val1 == val2)
                        continue;
                    match &= val1 && val2 && strcmp(val1, val2) == 0;
                }
                if(match)
                    break;
            }
        }

        if(existing)
        {
            pResult = target.ReplaceChild(existing, element)->ToElement();
        }
        else
            pResult = target.InsertEndChild(element)->ToElement();

        return pResult;
    }

    TiXmlElement* XMLDocument::GetRootElement()
    {
        return m_xmlDocument.RootElement();
    }
}


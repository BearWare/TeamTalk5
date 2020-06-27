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

#include "Settings.h"
#include <mystd/MyStd.h>
#include <sstream>

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
        std::ostringstream os;
        os << "File: " << this->GetFileName() << ". ";
        os << m_xmlDocument.ErrorDesc() << ". ";
        os << "Line " << m_xmlDocument.ErrorRow() << " column " << m_xmlDocument.ErrorCol() << ".";
        return os.str();
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

    void XMLDocument::SetValue(const std::string& path, const std::string& value)
    {
        TiXmlElement* item = GetRootElement();
        stdstrings_t tokens = stdtokenize(path, "/");
        assert(tokens.size());
        if (tokens.empty() || !item)
            return;

        std::string name = tokens.back();
        tokens.erase(tokens.end()-1);

        while (tokens.size())
        {
            auto child = item->FirstChildElement(tokens[0].c_str());
            if (!child)
            {
                TiXmlElement newelement(tokens[0].c_str());
                child = AppendElement(*item, newelement);
            }
            item = child;
            tokens.erase(tokens.begin());
        }
        PutString(*item, name, value);
    }

    std::string XMLDocument::GetValue(bool prefixRoot, const std::string& path, const std::string& defaultvalue)
    {
        TiXmlElement* item = GetRootElement();
        if (!item)
            return defaultvalue;

        stdstrings_t tokens = stdtokenize(path, "/");
        if (prefixRoot)
            tokens.insert(tokens.begin(), m_rootname);

        // handle root item
        if (tokens.size())
        {
            if(item->Value() != tokens[0])
            {
                tokens.clear();
                item = nullptr;
            }
            else
                tokens.erase(tokens.begin());
        }

        // handle child items
        while(item && tokens.size())
        {
            item = item->FirstChildElement(tokens[0].c_str());
            tokens.erase(tokens.begin());
        }
        string value = defaultvalue;
        if(tokens.empty() && item)
            GetElementText(*item, value);
        return value;
    }

    void XMLDocument::SetValue(const std::string& path, int value)
    {
        SetValue(path, i2str(value));
    }
    
    int XMLDocument::GetValue(bool prefixRoot, const std::string& path, int defaultvalue)
    {
        return str2i(GetValue(prefixRoot, path, i2str(defaultvalue)));
    }

    void XMLDocument::SetValueBool(const std::string& path, bool value)
    {
        SetValue(path, std::string(value?"true":"false"));
    }
    
    bool XMLDocument::GetValueBool(bool prefixRoot, const std::string& path, bool defaultvalue)
    {
        return GetValue(prefixRoot, path, std::string(defaultvalue?"true":"false")) == "true";
    }

    bool XMLDocument::LoadFile(const std::string& filename)
    {
        if(m_xmlDocument.LoadFile(filename.c_str()))
        {
            m_filename = filename;
            m_rootname = GetRootElement()? GetRootElement()->Value() : "";
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
            nValue = int(str2i(s));
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
            nValue = str2i(s);
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


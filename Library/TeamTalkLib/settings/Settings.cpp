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

#include "mystd/MyStd.h"

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>

using namespace std;

namespace teamtalk {

    XMLDocument::XMLDocument(std::string  rootname, std::string  version)
        : m_rootname(std::move(rootname))
        , m_xmlversion(std::move(version))
    {
    }

    XMLDocument::~XMLDocument()
    = default;

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
        return m_xmlDocument.Parse(xml.c_str(), nullptr, TIXML_ENCODING_UNKNOWN) == nullptr;
    }


    bool XMLDocument::CreateFile(const std::string& filename)
    {
        m_xmlDocument.Clear();
        string const szXml = 
            R"(<?xml version="1.0" encoding="UTF-8"?><)" + m_rootname + " version=\"" + m_xmlversion + "\">"
            "</" + m_rootname + ">";

        m_xmlDocument.Parse(szXml.c_str(), nullptr, TIXML_ENCODING_UTF8);
        return m_xmlDocument.SaveFile(filename.c_str()) && LoadFile(filename);
    }

    bool XMLDocument::SetFileVersion(const std::string& version)
    {
        TiXmlElement* item=GetRootElement();

        if(item != nullptr)
        {
            item->SetAttribute("version", version.c_str());
            return true;
        }
        return false;
    }

    string XMLDocument::GetFileVersion() const
    {
        const TiXmlElement* item=GetRootElement();

        string version;
        if(item != nullptr)
            version = item->Attribute("version");

        return version;
    }

    void XMLDocument::SetValue(const std::string& path, const std::string& value)
    {
        TiXmlElement* item = GetRootElement();
        stdstrings_t tokens = StringTokenize(path, "/");
        assert(!tokens.empty());
        if (tokens.empty() || (item == nullptr))
            return;

        std::string const name = tokens.back();
        tokens.erase(tokens.end()-1);

        while (!tokens.empty())
        {
            auto *child = item->FirstChildElement(tokens[0].c_str());
            if (child == nullptr)
            {
                TiXmlElement const newelement(tokens[0].c_str());
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
        if (item == nullptr)
            return defaultvalue;

        stdstrings_t tokens = StringTokenize(path, "/");
        if (prefixRoot)
            tokens.insert(tokens.begin(), m_rootname);

        // handle root item
        if (!tokens.empty())
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
        while((item != nullptr) && (!tokens.empty()))
        {
            item = item->FirstChildElement(tokens[0].c_str());
            tokens.erase(tokens.begin());
        }
        string value = defaultvalue;
        if(tokens.empty() && (item != nullptr))
            GetElementText(*item, value);
        return value;
    }

    void XMLDocument::SetValue(const std::string& path, int value)
    {
        SetValue(path, std::to_string(value));
    }
    
    int XMLDocument::GetValue(bool prefixRoot, const std::string& path, int defaultvalue)
    {
        return std::stoi(GetValue(prefixRoot, path, std::to_string(defaultvalue)));
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
            m_rootname = (GetRootElement() != nullptr)? GetRootElement()->Value() : "";
            return UpdateFile();
        }
        return false;
    }

    bool XMLDocument::SaveFile()
    {
        return m_xmlDocument.SaveFile();
    }

    bool XMLDocument::UpdateFile()
    {
        return true;
    }

    void XMLDocument::PutElementText(TiXmlElement& element, const std::string& value)
    {
        TiXmlText const text(value.c_str());
        element.InsertEndChild(text);
    }

    void XMLDocument::GetElementText(const TiXmlElement& element, string& value) 
    {
        //if string == "" text is null apparently
        if(element.FirstChild() != nullptr)
        {
            const TiXmlText* text = element.FirstChild()->ToText();
            if(text != nullptr)
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
        TiXmlText const text(bValue? "true" : "false");
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing != nullptr)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    void XMLDocument::PutString(TiXmlElement& parent, const string& szName, const string& szValue)
    {
        TiXmlElement newelement(szName.c_str());
        TiXmlText const text(szValue.c_str());
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing != nullptr)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    void XMLDocument::PutInteger(TiXmlElement& parent, const string& szName, int nValue)
    {
        TiXmlElement newelement(szName.c_str());
        string const s = std::to_string(nValue);

        TiXmlText const text(s.c_str());
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing != nullptr)
            parent.ReplaceChild(existing, newelement);
        else
            parent.InsertEndChild(newelement);
    }

    void XMLDocument::PutInteger(TiXmlElement& parent, const string& szName, int64_t nValue)
    {
        TiXmlElement newelement(szName.c_str());
        string const s = std::to_string(nValue);

        TiXmlText const text(s.c_str());
        newelement.InsertEndChild(text);

        TiXmlElement* existing = parent.FirstChildElement(szName.c_str());
        if(existing != nullptr)
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
        if(item != nullptr)
        {
            string s;
            GetElementText(*item, s);

            if (StringCmpNoCase(s, "true"))
            {
                bValue = true;
                return true;
            }
            if(StringCmpNoCase(s, "false"))
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
        if(item != nullptr)
        {
            GetElementText(*item, szValue);
            return true;
        }

        return false;
    }

    bool XMLDocument::GetInteger(const TiXmlElement& parent, const string& szName, int& nValue) const
    {
        const TiXmlElement* item = parent.FirstChildElement(szName.c_str());

        if(item != nullptr)
        {
            string s;
            GetElementText(*item, s);
            nValue = std::stoi(s);
            return true;
        }
        return false;
    }

    bool XMLDocument::GetInteger(const TiXmlElement& parent, const string& szName, int64_t& nValue) const
    {
        const TiXmlElement* item = parent.FirstChildElement(szName.c_str());

        if(item != nullptr)
        {
            string s;
            GetElementText(*item, s);
            nValue = stoll(s);
            return true;
        }
        return false;
    }

    TiXmlElement* XMLDocument::ReplaceElement(TiXmlElement& target, const TiXmlElement& element)
    {
        TiXmlElement* pResult = nullptr;
        TiXmlElement* existing = target.FirstChildElement(element.Value());
        if((existing != nullptr) && (existing->FirstAttribute() != nullptr))
        {
            bool const found = false;
            for(;(existing != nullptr) && !found;existing=existing->NextSiblingElement())
            {
                bool match = true;
                const TiXmlAttribute* attr = element.FirstAttribute();
                for(;(attr != nullptr) && match;attr=attr->Next())
                {
                    const char* val1 = existing->Attribute(attr->Name());
                    const char* val2 = attr->Value();
                    if(val1 == val2)
                        continue;
                    match &= (val1 != nullptr) && (val2 != nullptr) && strcmp(val1, val2) == 0;
                }
                if(match)
                    break;
            }
        }

        if(existing != nullptr)
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

    const TiXmlElement* XMLDocument::GetRootElement() const
    {
        return m_xmlDocument.RootElement();
    }
} // namespace teamtalk


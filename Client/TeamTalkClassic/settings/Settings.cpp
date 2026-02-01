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
#include "Settings.h"
#include "MyStd.h"
#include <cassert>
#include <sstream>

using namespace std;
using namespace tinyxml2;

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
        os << m_xmlDocument.ErrorStr() << ". ";
        os << "Line " << m_xmlDocument.ErrorLineNum() << ".";
        return os.str();
    }

    bool XMLDocument::Parse(const std::string& xml)
    {
        m_xmlDocument.Clear();
        return m_xmlDocument.Parse(xml.c_str()) == XML_SUCCESS;
    }


    bool XMLDocument::CreateFile(const std::string& filename)
    {
        m_xmlDocument.Clear();
        string szXml =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<" + m_rootname + " version=\"" + m_xmlversion + "\">"
            "</" + m_rootname + ">";

        m_xmlDocument.Parse(szXml.c_str());
        return m_xmlDocument.SaveFile(filename.c_str()) == XML_SUCCESS && LoadFile(filename.c_str());
    }

    bool XMLDocument::SetFileVersion(const std::string& version)
    {
        tinyxml2::XMLElement* item=GetRootElement();

        if(item)
        {
            item->SetAttribute("version", version.c_str());
            return true;
        }
        return false;
    }

    string XMLDocument::GetFileVersion()
    {
        tinyxml2::XMLElement* item=GetRootElement();

        string version;
        if(item)
        {
            const char* attr = item->Attribute("version");
            if (attr)
                version = attr;
        }

        return version;
    }

    void XMLDocument::SetValue(const std::string& path, const std::string& value)
    {
        tinyxml2::XMLElement* item = GetRootElement();
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
                child = AppendElement(item, tokens[0].c_str());
            }
            item = child;
            tokens.erase(tokens.begin());
        }
        PutString(item, name, value);
    }

    std::string XMLDocument::GetValue(bool prefixRoot, const std::string& path, const std::string& defaultvalue)
    {
        tinyxml2::XMLElement* item = GetRootElement();
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
            GetElementText(item, value);
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
        if(m_xmlDocument.LoadFile(filename.c_str()) == XML_SUCCESS)
        {
            m_filename = filename;
            m_rootname = GetRootElement()? GetRootElement()->Value() : "";
            return true;
        }
        return false;
    }

    bool XMLDocument::SaveFile()
    {
        return m_xmlDocument.SaveFile(m_filename.c_str()) == XML_SUCCESS;
    }

    void XMLDocument::PutElementText(tinyxml2::XMLElement* element, const std::string& value)
    {
        tinyxml2::XMLText* text = m_xmlDocument.NewText(value.c_str());
        element->InsertEndChild(text);
    }

    void XMLDocument::GetElementText(const tinyxml2::XMLElement* element, string& value)
    {
        //if string == "" text is null apparently
        if(element->FirstChild())
        {
            const tinyxml2::XMLText* text = element->FirstChild()->ToText();
            if(text)
                value = text->Value();
            else
                value = "";
        }
        else
            value = "";
    }

    void XMLDocument::PutBoolean(tinyxml2::XMLElement* parent, const string& szName, bool bValue)
    {
        tinyxml2::XMLElement* existing = parent->FirstChildElement(szName.c_str());
        if(existing)
            parent->DeleteChild(existing);

        tinyxml2::XMLElement* newelement = m_xmlDocument.NewElement(szName.c_str());
        tinyxml2::XMLText* text = m_xmlDocument.NewText(bValue ? "true" : "false");
        newelement->InsertEndChild(text);
        parent->InsertEndChild(newelement);
    }

    void XMLDocument::PutString(tinyxml2::XMLElement* parent, const string& szName, const string& szValue)
    {
        tinyxml2::XMLElement* existing = parent->FirstChildElement(szName.c_str());
        if(existing)
            parent->DeleteChild(existing);

        tinyxml2::XMLElement* newelement = m_xmlDocument.NewElement(szName.c_str());
        tinyxml2::XMLText* text = m_xmlDocument.NewText(szValue.c_str());
        newelement->InsertEndChild(text);
        parent->InsertEndChild(newelement);
    }

    void XMLDocument::PutInteger(tinyxml2::XMLElement* parent, const string& szName, int nValue)
    {
        tinyxml2::XMLElement* existing = parent->FirstChildElement(szName.c_str());
        if(existing)
            parent->DeleteChild(existing);

        tinyxml2::XMLElement* newelement = m_xmlDocument.NewElement(szName.c_str());
        string s = i2str(nValue);
        tinyxml2::XMLText* text = m_xmlDocument.NewText(s.c_str());
        newelement->InsertEndChild(text);
        parent->InsertEndChild(newelement);
    }

    void XMLDocument::PutInteger(tinyxml2::XMLElement* parent, const string& szName, int64_t nValue)
    {
        tinyxml2::XMLElement* existing = parent->FirstChildElement(szName.c_str());
        if(existing)
            parent->DeleteChild(existing);

        tinyxml2::XMLElement* newelement = m_xmlDocument.NewElement(szName.c_str());
        string s = i2str(nValue);
        tinyxml2::XMLText* text = m_xmlDocument.NewText(s.c_str());
        newelement->InsertEndChild(text);
        parent->InsertEndChild(newelement);
    }

    tinyxml2::XMLElement* XMLDocument::AppendElement(tinyxml2::XMLElement* parent, const char* name)
    {
        tinyxml2::XMLElement* newelement = m_xmlDocument.NewElement(name);
        return parent->InsertEndChild(newelement)->ToElement();
    }

    bool XMLDocument::GetBoolean(const tinyxml2::XMLElement* parent, const string& szName, bool& bValue) const
    {
        const tinyxml2::XMLElement* item = parent->FirstChildElement(szName.c_str());
        if(item)
        {
            string s;
            GetElementText(item, s);

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

    bool XMLDocument::GetString(const tinyxml2::XMLElement* parent, const string& szName, string& szValue) const
    {
        const tinyxml2::XMLElement* item = parent->FirstChildElement(szName.c_str());
        if(item)
        {
            GetElementText(item, szValue);
            return true;
        }

        return false;
    }

    bool XMLDocument::GetInteger(const tinyxml2::XMLElement* parent, const string& szName, int& nValue) const
    {
        const tinyxml2::XMLElement* item = parent->FirstChildElement(szName.c_str());

        if(item)
        {
            string s;
            GetElementText(item, s);
            nValue = str2i(s);
            return true;
        }
        return false;
    }

    bool XMLDocument::GetInteger(const tinyxml2::XMLElement* parent, const string& szName, int64_t& nValue) const
    {
        const tinyxml2::XMLElement* item = parent->FirstChildElement(szName.c_str());

        if(item)
        {
            string s;
            GetElementText(item, s);
            nValue = str2i64(s);
            return true;
        }
        return false;
    }

    tinyxml2::XMLElement* XMLDocument::ReplaceElement(tinyxml2::XMLElement* target, const char* name)
    {
        tinyxml2::XMLElement* existing = target->FirstChildElement(name);
        if(existing)
            target->DeleteChild(existing);

        tinyxml2::XMLElement* newelement = m_xmlDocument.NewElement(name);
        return target->InsertEndChild(newelement)->ToElement();
    }

    tinyxml2::XMLElement* XMLDocument::GetRootElement()
    {
        return m_xmlDocument.RootElement();
    }
}

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
#include "XMLLanguages.h"
#include "MyStd.h"

#include <vector>
#include <algorithm>
#include <set>
using namespace std;
using namespace tinyxml2;

XMLLanguages::XMLLanguages()
: XMLDocument("language", XML_LANGUAGE_VERSION)
{
}

XMLLanguages::~XMLLanguages()
{
}

tinyxml2::XMLElement* XMLLanguages::GetRootElement()
{
    tinyxml2::XMLElement* root = m_xmlDocument.RootElement();
    if(!root)
    {
        tinyxml2::XMLElement* newroot = m_xmlDocument.NewElement("languages");
        newroot->SetAttribute("version", m_xmlversion.c_str());
        m_xmlDocument.InsertEndChild(newroot);
        root = m_xmlDocument.RootElement();
    }
    return root;
}

bool XMLLanguages::CreateFile(const std::string& szFileName)
{
    m_xmlDocument.Clear();
    string szXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<language version=\"" XML_LANGUAGE_VERSION "\">"
        "</language>";

  m_xmlDocument.Parse(szXml.c_str());
  return m_xmlDocument.SaveFile(szFileName.c_str()) == XML_SUCCESS && LoadFile(szFileName.c_str());
}

void XMLLanguages::AddItem(int id, std::string szText)
{
    tinyxml2::XMLElement* root = GetRootElement();
    if(root)
    {
        //delete item if it already exists
        RemoveItem(id);

        tinyxml2::XMLElement* newitem = m_xmlDocument.NewElement("item");
        newitem->SetAttribute("id", i2str(id).c_str());
        tinyxml2::XMLText* text = m_xmlDocument.NewText(szText.c_str());
        newitem->InsertEndChild(text);
        root->InsertEndChild(newitem);
    }
}

void XMLLanguages::RemoveItem(int id)
{
    tinyxml2::XMLElement* item = GetItem(id);
    tinyxml2::XMLNode* parent;
    if(item && (parent = item->Parent()))
    {
        parent->DeleteChild(item);
    }
}

tinyxml2::XMLElement* XMLLanguages::GetItem(int id)
{
    tinyxml2::XMLElement* root = m_xmlDocument.RootElement();

    if(root)
    {
        tinyxml2::XMLElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0 && i2str(id) == child->Attribute("id"))
                return child;
        }
    }
    return NULL;
}

std::string XMLLanguages::GetItemText(int id)
{
    std::string text;
    tinyxml2::XMLElement* item = GetItem(id);
    if(item)
    {
        GetElementText(item, text);
    }
    else
    {
    }

    return text;
}

int XMLLanguages::GetFirstItem()
{
    tinyxml2::XMLElement* root = GetRootElement();
    if(root)
    {
        tinyxml2::XMLElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0)
            {
                return str2i(child->Attribute("id"));
            }
        }
    }
    return -1;
}

int XMLLanguages::GetNextItem(int curid)
{
    tinyxml2::XMLElement* root = m_xmlDocument.RootElement();
    int id = -1;
    if(root)
    {
        tinyxml2::XMLElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0 && i2str(curid) == child->Attribute("id") && child->NextSiblingElement())
            {
                child = child->NextSiblingElement();
                if(child)
                    return str2i(child->Attribute("id"));
                else
                    break;
            }
        }
    }
    return id;
}

int XMLLanguages::GetPrevItem(int curid)
{
    tinyxml2::XMLElement* root = m_xmlDocument.RootElement();
    int id = -1;
    if(root)
    {
        tinyxml2::XMLElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0 && i2str(curid) == child->Attribute("id") && child->PreviousSibling())
            {
                child = child->PreviousSibling()->ToElement();
                if(child)
                    return str2i(child->Attribute("id"));
                else
                    break;
            }
        }
    }
    return id;
}

void XMLLanguages::SortItems()
{
    std::set<int> items;
    tinyxml2::XMLElement* root = m_xmlDocument.RootElement();
    if(root)
    {
        tinyxml2::XMLElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0)
                items.insert(str2i(child->Attribute("id")));
        }
    }

    for(std::set<int>::const_iterator i=items.begin();i!=items.end();i++)
    {
        string text = GetItemText(*i);
        RemoveItem(*i);
        AddItem(*i, text);
    }
}


/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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

#include "stdafx.h"
#include "XMLLanguages.h"
#include "MyStd.h"

#include <vector>
#include <algorithm>
using namespace std;

XMLLanguages::XMLLanguages()
: XMLDocument("language")
{
}

XMLLanguages::~XMLLanguages()
{
}

TiXmlElement* XMLLanguages::GetRootElement()
{
    TiXmlElement* root = m_xmlDocument.RootElement();
    if(!root)
    {
        TiXmlElement newroot("languages");
        newroot.SetAttribute("version", XML_LANGUAGE_VERSION);
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
  return m_xmlDocument.SaveFile(szFileName.c_str()) && LoadFile(szFileName.c_str());
}

void XMLLanguages::AddItem(int id, std::string szText)
{
    TiXmlElement* root = GetRootElement();
    if(root)
    {
        //delete item if it already exists
        RemoveItem(id);

        TiXmlElement newitem("item");
        newitem.SetAttribute("id", i2str(id).c_str());
        TiXmlText text(szText.c_str());
        newitem.InsertEndChild(text);
        root->InsertEndChild(newitem);
    }
}

void XMLLanguages::RemoveItem(int id)
{
    TiXmlElement* item = GetItem(id);
    TiXmlNode* parent;
    if(item && (parent = item->Parent()))
    {
        parent->RemoveChild(item);
    }
}

TiXmlElement* XMLLanguages::GetItem(int id)
{
    TiXmlElement* root = m_xmlDocument.RootElement();

    if(root)
    {
        TiXmlElement* child;
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
    TiXmlElement* item = GetItem(id);
    if(item)
        GetElementText(*item, text);
    return text;
}

int XMLLanguages::GetFirstItem()
{
    TiXmlElement* root = GetRootElement();
    if(root)
    {
        TiXmlElement* child;
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
    TiXmlElement* root = m_xmlDocument.RootElement();
    int id = -1;
    if(root)
    {
        TiXmlElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0 && i2str(curid) == child->Attribute("id"))
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
    TiXmlElement* root = m_xmlDocument.RootElement();
    int id = -1;
    if(root)
    {
        TiXmlElement* child;
        for(child=root->FirstChildElement();child;child=child->NextSiblingElement())
        {
            if(strcmp(child->Value(), "item") == 0 && i2str(curid) == child->Attribute("id"))
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
    std::vector<int> items;
    int id = GetFirstItem();
    while(id != -1)
    {
        items.push_back(id);
        id = GetNextItem(id);
    }
    std::sort(items.begin(), items.end());
    for(size_t i=0;i<items.size();i++)
    {
        string text = GetItemText(items[i]);
        RemoveItem(items[i]);
        AddItem(items[i], text);
    }
}


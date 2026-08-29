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

#include "BadWords.h"

#include "myace/MyACE.h"
#include "teamtalk/Log.h"

#include <ace/Dirent_Selector.h>
#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdio.h>
#include <ace/OS_NS_string.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>

namespace spambot {

    const std::vector<std::string> BadWords::EMPTY;

    static std::string ToLower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    static bool IsWordChar(unsigned char c)
    {
        return std::isalnum(c) != 0 || c == '_' || (c & 0x80) != 0;
    }

    bool BadWords::LoadFile(const std::string& language, const std::string& filename)
    {
        std::vector<std::string> words;

        std::ifstream f(filename);
        if (!f.is_open())
        {
            ACE_TCHAR msg[512];
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Failed to load badwords file %C."), filename.c_str());
            TT_SYSLOG(msg);
            m_lang_badwords[language] = words;
            return false;
        }

        std::string line;
        while (std::getline(f, line))
        {
            std::istringstream iss(line);
            std::string token;
            while (std::getline(iss, token, ','))
            {
                if (!token.empty())
                    words.push_back(ToLower(std::move(token)));
            }
        }

        m_lang_badwords[language] = std::move(words);
        return true;
    }

    bool BadWords::Contains(const std::string& language, const std::string& sentence) const
    {
        std::string s = ToLower(sentence);
        std::replace(s.begin(), s.end(), '_', ' ');

        std::string word;
        word.reserve(32);
        for (unsigned char c : s)
        {
            if (IsWordChar(c) && c != '_')
            {
                word.push_back(static_cast<char>(c));
            }
            else
            {
                if (!word.empty())
                {
                    if (ContainsWord(language, word))
                        return true;
                    word.clear();
                }
            }
        }
        if (!word.empty() && ContainsWord(language, word))
            return true;

        return false;
    }

    bool BadWords::ContainsWord(const std::string& language, const std::string& wordLowerCase) const
    {
        auto it = m_lang_badwords.find(language);
        if (it == m_lang_badwords.end())
            return false;
        return std::find(it->second.begin(), it->second.end(), wordLowerCase) != it->second.end();
    }

    bool BadWords::ContainsAny(const std::string& sentence) const
    {
        for (const auto& kv : m_lang_badwords)
        {
            if (Contains(kv.first, sentence))
                return true;
        }
        return false;
    }

    const std::vector<std::string>& BadWords::GetBadWords(const std::string& language) const
    {
        auto it = m_lang_badwords.find(language);
        return it != m_lang_badwords.end() ? it->second : EMPTY;
    }

    namespace {
        std::string DirentNameToUtf8(const ACE_DIRENT* entry)
        {
            ACE_TString const tname(entry->d_name);
            return UnicodeToUtf8(tname).c_str();
        }

        int MatchesBadwordsName(const ACE_DIRENT* entry)
        {
            std::string const name = DirentNameToUtf8(entry);
            if (name.compare(0, 8, "badwords") != 0)
                return 0;
            return (name.size() >= 12 && name.compare(name.size() - 4, 4, ".txt") == 0) ? 1 : 0;
        }
    }

    int BadWords::LoadDirectory(const std::string& dir)
    {
        std::string path = dir.empty() ? std::string(".") : dir;
        ACE_TString const tpath = Utf8ToUnicode(path.c_str());
        ACE_Dirent_Selector ds;
        if (ds.open(tpath.c_str(), MatchesBadwordsName) == -1)
        {
            ACE_TCHAR msg[512];
            ACE_OS::snprintf(msg, 512, ACE_TEXT("Failed to scan badwords directory %C."), path.c_str());
            TT_SYSLOG(msg);
            return 0;
        }

        int loaded = 0;
        for (int i = 0; i < ds.length(); ++i)
        {
            std::string const fname = DirentNameToUtf8(ds[i]);
            std::string filename = path + "/" + fname;

            std::string base = fname.substr(0, fname.size() - 4);

            std::string language;
            if (base == "badwords")
                language = "";
            else if (base.size() > 9 && base[8] == '_')
                language = base.substr(9);
            else
                continue;

            if (LoadFile(language, filename))
                ++loaded;
        }
        ds.close();
        return loaded;
    }

} // namespace spambot

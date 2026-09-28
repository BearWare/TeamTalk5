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

#ifndef SPAMBOT_BADWORDS_H
#define SPAMBOT_BADWORDS_H

#include <map>
#include <string>
#include <vector>

namespace spambot {

    class BadWords
    {
    public:
        bool LoadFile(const std::string& language, const std::string& filename);
        int  LoadDirectory(const std::string& dir);

        bool Contains(const std::string& language, const std::string& sentence) const;
        bool ContainsWord(const std::string& language, const std::string& wordLowerCase) const;
        bool ContainsAny(const std::string& sentence) const;

        const std::vector<std::string>& GetBadWords(const std::string& language) const;

    private:
        std::map<std::string, std::vector<std::string>> m_lang_badwords;
        static const std::vector<std::string> EMPTY;
    };

} // namespace spambot

#endif // SPAMBOT_BADWORDS_H

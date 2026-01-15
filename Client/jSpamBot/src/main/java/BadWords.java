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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;
import java.util.logging.Logger;

public class BadWords {
    Map<String, Vector<String>> lang_badwords = new HashMap<>();
    Logger logger;

    public BadWords(Logger log) {
        this.logger = log;
    }

    boolean loadFile(String language, String filename) {
        Vector<String> badwords = new Vector<>();
        File file = new File(filename);
        if (file.exists()) {
            try (BufferedReader br = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = br.readLine()) != null) {
                    badwords.addAll(Arrays.asList(line.split(",")));
                }

                while (badwords.remove(""));
            }
            catch (IOException e) {
                logger.severe("Failed to load badwords for " + filename);
                return false;
            }
        }

        Vector<String> badwords_lower = new Vector<>();
        for (String w : badwords)
            badwords_lower.add(w.toLowerCase());

        lang_badwords.put(language, badwords_lower);
        return true;
    }

    boolean contains(String language, String sentence) {
        sentence = sentence.toLowerCase().replace('_', ' ');

        String[] words = sentence.split("\\W");
        for (String word : words) {
            if (word.isEmpty())
                continue;
            if (containsWord(language, word)) {
                return true;
            }
        }
        return false;
    }

    boolean containsWord(String language, String wordLowerCase) {
        var badwords = lang_badwords.get(language);
        return badwords != null ? badwords.contains(wordLowerCase) : false;
    }

    Vector<String> getBadWords(String language) {
        return lang_badwords.get(language) != null ? lang_badwords.get(language) : new Vector<>();
    }
}

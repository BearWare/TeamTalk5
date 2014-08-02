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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.gui;

import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Vector;

import com.google.gson.Gson;

import dk.bearware.Channel;
import dk.bearware.User;
import dk.bearware.data.ServerEntry;
import android.content.Intent;
import android.preference.EditTextPreference;
import android.preference.Preference;

public class Utils {

    public static void setEditTextPreference(Preference preference, String text, String summary) {
        EditTextPreference textpref = (EditTextPreference) preference;
        textpref.setText(text);
        if (summary.length() > 0)
            textpref.setSummary(summary);
    }

    public static String getEditTextPreference(Preference preference) {
        EditTextPreference textpref = (EditTextPreference) preference;
        return textpref.getText();
    }
    
    public static Intent putServerEntry(Intent intent, ServerEntry entry) {
        return intent.putExtra(ServerEntry.class.getName(), new Gson().toJson(entry));
    }
    
    public static ServerEntry getServerEntry(Intent intent) {
        if (intent.hasExtra(ServerEntry.class.getName())) {
            return new Gson().fromJson(intent.getExtras().getString(ServerEntry.class.getName()),  ServerEntry.class);
        }
        return null;
    }
    
    public static Vector<Channel> getSubChannels(int chanid, Map<Integer, Channel> channels) {
        Vector<Channel> result = new Vector<Channel>();
        
        Iterator<Entry<Integer, Channel>> it = channels.entrySet().iterator();

        while (it.hasNext()) {
            Channel chan = it.next().getValue();
            if (chan.nParentID == chanid)
                result.add(chan);
        }
        return result;
    }
    
    public static Vector<User> getUsers(int chanid, Map<Integer, User> users) {
        Vector<User> result = new Vector<User>();
        Iterator<Entry<Integer, User>> it = users.entrySet().iterator();

        while (it.hasNext()) {
            User user = it.next().getValue();
            if (user.nChannelID == chanid)
                result.add(user);
        }
        return result;
    }
}

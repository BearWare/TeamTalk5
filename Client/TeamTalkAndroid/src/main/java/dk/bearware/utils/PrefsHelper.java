/*
 * Copyright (c) 2005-2025, BearWare.dk
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
package dk.bearware.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import java.util.Map;
import java.util.Set;

public class PrefsHelper {
    private final SharedPreferences prefs;

    public PrefsHelper(Context ctx) {
        prefs = PreferenceManager.getDefaultSharedPreferences(ctx);
    }

    public void put(String key, Object value) {
        SharedPreferences.Editor editor = prefs.edit();
        putValue(editor, key, value);
        editor.apply();
    }

    public void putAll(Map<String, Object> values) {
        SharedPreferences.Editor editor = prefs.edit();
        for (Map.Entry<String, Object> entry : values.entrySet()) {
            putValue(editor, entry.getKey(), entry.getValue());
        }
        editor.apply();
    }

    private void putValue(SharedPreferences.Editor editor, String key, Object value) {
        if (value instanceof String) {
            editor.putString(key, (String) value);
        } else if (value instanceof Integer) {
            editor.putInt(key, (Integer) value);
        } else if (value instanceof Boolean) {
            editor.putBoolean(key, (Boolean) value);
        } else if (value instanceof Float) {
            editor.putFloat(key, (Float) value);
        } else if (value instanceof Long) {
            editor.putLong(key, (Long) value);
        } else if (value instanceof Set) {
            //noinspection unchecked
            editor.putStringSet(key, (Set<String>) value);
        } else {
            throw new IllegalArgumentException("Unsupported preference type: " + value.getClass());
        }
    }

    @SuppressWarnings("unchecked")
    public <T> T get(String key, T defaultValue) {
        if (defaultValue instanceof String) {
            return (T) prefs.getString(key, (String) defaultValue);
        } else if (defaultValue instanceof Integer) {
            return (T) (Integer) prefs.getInt(key, (Integer) defaultValue);
        } else if (defaultValue instanceof Boolean) {
            return (T) (Boolean) prefs.getBoolean(key, (Boolean) defaultValue);
        } else if (defaultValue instanceof Float) {
            return (T) (Float) prefs.getFloat(key, (Float) defaultValue);
        } else if (defaultValue instanceof Long) {
            return (T) (Long) prefs.getLong(key, (Long) defaultValue);
        } else if (defaultValue instanceof Set) {
            return (T) prefs.getStringSet(key, (Set<String>) defaultValue);
        } else {
            throw new IllegalArgumentException("Unsupported preference type: " + defaultValue.getClass());
        }
    }
}

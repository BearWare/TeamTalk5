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

package dk.bearware.data;

import java.lang.reflect.Constructor;

import android.content.Context;

public abstract class TTSWrapper {
    @SuppressWarnings({"unchecked", "rawtypes"})
    public static TTSWrapper getInstance(Context context) {
        try {
            @SuppressWarnings("unused")
            Class tts = Class.forName("android.speech.tts.TextToSpeech");
            Class impl = Class.forName("dk.bearware.data.TTSWrapperImpl");
            Constructor c = impl.getConstructor(Context.class);
            return (TTSWrapper) c.newInstance(context);
        } catch (Exception e) {
            return null;
        }
    }

    public abstract void shutdown();

    public abstract void speak(String message);
}

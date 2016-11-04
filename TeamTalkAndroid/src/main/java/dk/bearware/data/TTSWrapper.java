/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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

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

import android.content.Context;
import android.os.Build;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.EngineInfo;
import android.util.Log;
import java.util.Collections;
import java.util.List;

public class TTSWrapper {
    private static final String TAG = "bearware";
    private static TextToSpeech tts;
    public static final String defaultEngineName = "com.google.android.tts"; // should be got from getDefaultEngine method.
    private Context mContext;
    private String mCurrentEngineName = defaultEngineName;

    public TTSWrapper(Context context) {
        init(context);
        tts = new TextToSpeech(context, null);
    }

    public TTSWrapper(Context context, String engineName) {
        init(context);
        tts = new TextToSpeech(context, null, engineName);
        this.mCurrentEngineName = engineName;
    }

    private void init(Context context) {
        this.mContext = context;
    }

    public void shutdown() {
        if (tts == null) {
            tts.shutdown();
        }
    }

    public void speak(String text) {
        if (tts != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                tts.speak(text, TextToSpeech.QUEUE_ADD, null, null);
            } else {
                tts.speak(text, TextToSpeech.QUEUE_ADD, null);
            }
        }
    }

    public static List<EngineInfo> getEngines() {
        if (tts != null) {
            List<EngineInfo> spEngines = tts.getEngines();
            return spEngines;
        } else {
            return Collections.emptyList();
        }
    }

    public TTSWrapper switchEngine(String engineName) {
        if (tts != null) {
            return engineName.equals(this.mCurrentEngineName) ? this : new TTSWrapper(this.mContext, engineName);
        } else {
            return null;
        }
    }

}

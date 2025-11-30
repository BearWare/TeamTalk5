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
import android.media.AudioAttributes;
import android.os.Build;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.EngineInfo;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import java.util.Collections;
import java.util.List;

public class TTSWrapper {
    private static final String TAG = "bearware";
    private TextToSpeech tts;
    public static final String defaultEngineName = "com.google.android.tts"; // should be got from getDefaultEngine method.
    private final Context mContext;
    public Boolean useAnnouncements;
    private String mCurrentEngineName = defaultEngineName;

    public TTSWrapper(Context context) {
        this.mContext = context;
        tts = new TextToSpeech(context, null);
    }

    public TTSWrapper(Context context, String engineName) {
        this(context);
        tts = new TextToSpeech(context, null, engineName);
        this.mCurrentEngineName = engineName;
    }

    public void shutdown() {
        tts.shutdown();
    }

    public void setAccessibilityStream(boolean bEnable) {
        tts.setAudioAttributes( new AudioAttributes.Builder()
        .setContentType(bEnable ? AudioAttributes.CONTENT_TYPE_SPEECH : AudioAttributes.CONTENT_TYPE_UNKNOWN)
        .setUsage(bEnable ? AudioAttributes.USAGE_ASSISTANCE_ACCESSIBILITY : AudioAttributes.USAGE_UNKNOWN)
        .build());
    }

    public void speak(String text) {
        if (this.useAnnouncements) {
            AccessibilityManager manager = (AccessibilityManager) mContext.getSystemService(Context.ACCESSIBILITY_SERVICE);
            if ((manager != null) && manager.isEnabled()) {
                AccessibilityEvent e = AccessibilityEvent.obtain();
                e.setEventType(AccessibilityEvent.TYPE_ANNOUNCEMENT);
                e.getText().add(text);
                manager.sendAccessibilityEvent(e);
            }
        } else {
            tts.speak(text, TextToSpeech.QUEUE_ADD, null, null);
        }
    }

    public List<EngineInfo> getEngines() {
        List<EngineInfo> spEngines = tts.getEngines();
        return spEngines;
    }

    public TTSWrapper switchEngine(String engineName) {
        return engineName.equals(this.mCurrentEngineName) ? this : new TTSWrapper(this.mContext, engineName);
    }

}

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

package dk.bearware.data;

import android.content.Context;
import android.speech.tts.TextToSpeech;

public class TTSWrapperImpl extends TTSWrapper{
	private static TextToSpeech ttsInstance = null;
	
	public TTSWrapperImpl(Context context) {
		ttsInstance = new TextToSpeech(context, null);
	}

	public void shutdown() {
		ttsInstance.shutdown();
	}

	public void speak(String message) {
		ttsInstance.speak(message, TextToSpeech.QUEUE_ADD, null);	
	}
}

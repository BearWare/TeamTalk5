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

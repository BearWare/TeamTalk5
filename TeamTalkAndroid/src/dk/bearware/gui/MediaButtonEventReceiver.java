package dk.bearware.gui;

import dk.bearware.TeamTalkBase;
import dk.bearware.backend.TeamTalkService;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;

public class MediaButtonEventReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_MEDIA_BUTTON.equals(intent.getAction())) {
            TeamTalkService ttService = ((TeamTalkService.LocalBinder)peekService(context, new Intent(context, TeamTalkService.class))).getService();
            TeamTalkBase ttClient = ttService.getTTInstance();
            KeyEvent keyEvent = (KeyEvent)intent.getExtras().get(Intent.EXTRA_KEY_EVENT);
            int keyAction = keyEvent.getAction();
            int keyCode = keyEvent.getKeyCode();
            switch (keyCode) {
            case KeyEvent.KEYCODE_MEDIA_PLAY:
            case KeyEvent.KEYCODE_MEDIA_PAUSE:
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
            case KeyEvent.KEYCODE_HEADSETHOOK:
                switch (keyAction) {
                case KeyEvent.ACTION_UP:
                    ttClient.enableVoiceTransmission(!ttService.isVoiceTransmissionEnabled());
                    break;
                default:
                    break;
                }
                abortBroadcast();
                break;
            default:
                break;
            }
        }
    }

}

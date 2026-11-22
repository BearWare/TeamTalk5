package dk.bearware.data;

import dk.bearware.ClientEvent;
import dk.bearware.StreamType;
import dk.bearware.TeamTalkBase;
import dk.bearware.User;
import dk.bearware.UserState;

public class UserCached {
    int subscriptions;
    boolean voiceMute, mediaMute;
    int voiceVolume, mediaVolume;
    boolean voiceLeftSpeaker, voiceRightSpeaker,
            mediaLeftSpeaker, mediaRightSpeaker;

    public static String getCacheID(User user) {
        if (user.szUsername.endsWith(AppInfo.WEBLOGIN_BEARWARE_USERNAMEPOSTFIX))
            return user.szUsername + "|" + user.szClientName;
        return "";
    }

    public UserCached(User user) {
        this.subscriptions = user.uLocalSubscriptions;
        voiceMute = (user.uUserState & UserState.USERSTATE_MUTE_VOICE) != UserState.USERSTATE_NONE;
        mediaMute = (user.uUserState & UserState.USERSTATE_MUTE_MEDIAFILE) != UserState.USERSTATE_NONE;
        voiceVolume = user.nVolumeVoice;
        mediaVolume = user.nVolumeMediaFile;
        voiceLeftSpeaker = user.stereoPlaybackVoice[0];
        voiceRightSpeaker = user.stereoPlaybackVoice[1];
        mediaLeftSpeaker = user.stereoPlaybackMediaFile[0];
        mediaRightSpeaker = user.stereoPlaybackMediaFile[1];
    }

    public void sync(TeamTalkBase ttclient, User user) {
        ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_VOICE, voiceMute);
        ttclient.setUserMute(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, mediaMute);
        ttclient.setUserVolume(user.nUserID, StreamType.STREAMTYPE_VOICE, voiceVolume);
        ttclient.setUserVolume(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, mediaVolume);
        ttclient.setUserStereo(user.nUserID, StreamType.STREAMTYPE_VOICE, voiceLeftSpeaker, voiceRightSpeaker);
        ttclient.setUserStereo(user.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, mediaLeftSpeaker, mediaRightSpeaker);
        if (subscriptions != user.uLocalSubscriptions)
        {
            ttclient.doUnsubscribe(user.nUserID, user.uLocalSubscriptions ^ subscriptions);
            ttclient.doSubscribe(user.nUserID, subscriptions);
        }
        ttclient.pumpMessage(ClientEvent.CLIENTEVENT_USER_STATECHANGE, user.nUserID);
    }
}

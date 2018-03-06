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

import java.nio.ByteBuffer;
import java.util.Vector;

import dk.bearware.DesktopInput;
import dk.bearware.MediaFileInfo;
import dk.bearware.User;
import dk.bearware.UserState;
import dk.bearware.VideoFrame;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.UserListener;
import dk.bearware.gui.Utils;
import android.content.Context;
import android.graphics.Bitmap;
import android.view.LayoutInflater;

public class WebcamAdapter extends MediaAdapter implements UserListener {

    public static final String TAG = "bearware";

    private LayoutInflater inflater;
    
    public WebcamAdapter(Context context) {
        super(context);
        inflater = LayoutInflater.from(context);
    }
    
    public void setTeamTalkService(TeamTalkService service) {
        super.setTeamTalkService(service);

        service.registerUserListener(this);

        Vector<User> vecusers = Utils.getUsers(service.getUsers());
        for(User user : vecusers) {
            if((user.uUserState & UserState.USERSTATE_VIDEOCAPTURE) == UserState.USERSTATE_VIDEOCAPTURE)
                display_users.put(user.nUserID, user);
        }
    }

    public void clearTeamTalkService(TeamTalkService service) {
        super.clearTeamTalkService(service);
        service.unregisterUserListener(this);
    }

    @Override
    public Bitmap extractUserBitmap(int userid, Bitmap prev_bmp) {
        VideoFrame wnd = ttservice.getTTInstance().acquireUserVideoCaptureFrame(userid);
        
        if(wnd == null) {
            return null;
        }
        
        if(prev_bmp != null) {
            // create new bitmap if size 
            if(prev_bmp.getWidth() != wnd.nWidth || prev_bmp.getHeight() != wnd.nHeight)
                prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
        }
        else {
            prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
        }
        
        prev_bmp.copyPixelsFromBuffer(ByteBuffer.wrap(wnd.frameBuffer));
        
        return prev_bmp;
    }

    @Override
    public void onUserStateChange(User user) {
//        this.updateUserStreamState(user, UserState.USERSTATE_VIDEOCAPTURE);
    }

    @Override
    public void onUserVideoCapture(int nUserID, int nStreamID) {
//        Log.d(TAG, "New webcam frame from #" + nUserID + " stream " + nStreamID + " " + Integer.toHexString(this.hashCode()));
        // only update if user is expanded (bitmap is being displayed)
        if (media_sessions.indexOfKey(nUserID) >= 0)
            updateUserBitmap(nUserID);
    }

    @Override
    public void onUserMediaFileVideo(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopWindow(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput) {
    }

    @Override
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
    }

    @Override
    public void onUserAudioBlock(int nUserID, int nStreamType) {
    }
}

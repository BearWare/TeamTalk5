/*
 * Copyright (c) 2005-2016, BearWare.dk
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

import java.nio.ByteBuffer;
import java.util.Vector;

import dk.bearware.BitmapFormat;
import dk.bearware.DesktopInput;
import dk.bearware.DesktopWindow;
import dk.bearware.MediaFileInfo;
import dk.bearware.User;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.UserListener;
import dk.bearware.gui.Utils;
import android.content.Context;
import android.graphics.Bitmap;
import android.view.LayoutInflater;

public class DesktopAdapter
extends MediaAdapter
implements UserListener
{
    public static final String TAG = "bearware";

    private LayoutInflater inflater;
    
    public DesktopAdapter(Context context) {
        super(context);
        inflater = LayoutInflater.from(context);
    }
    
    public void setTeamTalkService(TeamTalkService service) {
        super.setTeamTalkService(service);
        service.registerUserListener(this);

        Vector<User> vecusers = Utils.getUsers(ttservice.getUsers());
        for(User user : vecusers) {
            if((user.uUserState & UserState.USERSTATE_DESKTOP) == UserState.USERSTATE_DESKTOP)
                display_users.put(user.nUserID, user);
        }
    }
    
    public void clearTeamTalkService(TeamTalkService service) {
        super.clearTeamTalkService(service);
        service.unregisterUserListener(this);
    }
    
    public Bitmap extractUserBitmap(int userid, Bitmap prev_bmp) {

        DesktopWindow wnd = ttservice.getTTInstance().acquireUserDesktopWindowEx(userid,
                                                                                 BitmapFormat.BMP_RGB32);
        // TODO: only RGB32 support for now 
        if(wnd == null || wnd.bmpFormat != BitmapFormat.BMP_RGB32)
            return null;
        
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
        
//        this.updateUserStreamState(user, UserState.USERSTATE_DESKTOP);
    }

    @Override
    public void onUserVideoCapture(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserMediaFileVideo(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopWindow(int nUserID, int nStreamID) {
        //only update if user is expanded (bitmap is being displayed)
        if(media_sessions.indexOfKey(nUserID) >= 0)
            updateUserBitmap(nUserID);
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

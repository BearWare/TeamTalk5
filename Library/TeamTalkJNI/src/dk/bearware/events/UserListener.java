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

package dk.bearware.events;

import dk.bearware.DesktopInput;
import dk.bearware.MediaFileInfo;
import dk.bearware.User;

public interface UserListener {

    public void onUserStateChange(User user);
    public void onUserVideoCapture(int nUserID, int nStreamID);
    public void onUserMediaFileVideo(int nUserID, int nStreamID);
    public void onUserDesktopWindow(int nUserID, int nStreamID);
    public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput);
    public void onUserDesktopInput(int nUserID, DesktopInput desktopinput);
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo);
    public void onUserAudioBlock(int nUserID, int nStreamType);
}

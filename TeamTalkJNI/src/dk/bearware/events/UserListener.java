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
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo);
    //TODO: AudioBlock-struct
    //public void onUserAudioBlock();
}

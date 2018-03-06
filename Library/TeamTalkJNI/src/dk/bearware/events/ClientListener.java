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

import dk.bearware.ClientErrorMsg;
import dk.bearware.FileTransfer;
import dk.bearware.MediaFileInfo;

public interface ClientListener {

    public void onInternalError(ClientErrorMsg clienterrormsg);
    public void onVoiceActivation(boolean bVoiceActive);
    public void onHotKeyToggle(int nHotKeyID, boolean bActive);
    public void onHotKeyTest(int nVkCode, boolean bActive);
    public void onFileTransfer(FileTransfer filetransfer);
    public void onDesktopWindowTransfer(int nSessionID, int nTransferRemaining);
    public void onStreamMediaFile(MediaFileInfo mediafileinfo);
}

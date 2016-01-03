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

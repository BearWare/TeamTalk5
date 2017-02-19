/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware;

public class ServerCallback {
    
    public void userLogin(ClientErrorMsg lpClientErrorMsg,
                          User lpUser, UserAccount lpUserAccount) {
    }

    public void userChangeNickname(ClientErrorMsg lpClientErrorMsg,
                                   User lpUser, String szNewNickname) {
    }

    public void userChangeStatus(ClientErrorMsg lpClientErrorMsg,
                                 User lpUser, int nNewStatusMode, String szNewStatusMsg) {
    }

    public void userCreateUserAccount(ClientErrorMsg lpClientErrorMsg,
                                      User lpUser, UserAccount lpUserAccount) {
    }

    public void userDeleteUserAccount(ClientErrorMsg lpClientErrorMsg,
                                      User lpUser, String szUsername) {
    }

    public void userAddServerBan(ClientErrorMsg lpClientErrorMsg,
                                 User lpBanner, User lpBanee) {
    }
    
    public void userAddServerBanIPAddress(ClientErrorMsg lpClientErrorMsg,
                                          User lpBanner, String szIPAddress) {
    }

    public void userDeleteServerBan(ClientErrorMsg lpClientErrorMsg,
                                    User lpUser, String szIPAddress) {
    }
}

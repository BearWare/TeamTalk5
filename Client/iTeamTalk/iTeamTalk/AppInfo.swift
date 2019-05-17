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

import UIKit

class AppInfo {

    static let TTLINK_PREFIX = "tt://"

    static let OSTYPE = "iOS"

    static let DEFAULT_TCPPORT = 10333
    static let DEFAULT_UDPPORT = 10333
    
    static let WEBLOGIN_FACEBOOK = "facebook"
    static let WEBLOGIN_FACEBOOK_PASSWDPREFIX = "token="
    
    enum BundleInfo {
        case name, version_NO
    }
    
    static func getBundleInfo(_ b: BundleInfo) -> String {
        let bundle = Bundle.main
        let dict = bundle.infoDictionary

        switch b {
        case .name :
            if let info = dict?["CFBundleName"] {
                return info as! String
            }
            return "Unknown"
        case .version_NO :
            if let info = dict?["CFBundleShortVersionString"] {
                return info as! String
            }
            return "0.1"
        }
    }
    
    static func getAppName() -> String {
        return getBundleInfo(.name)
    }
    static func getAppVersion() -> String {
        return getBundleInfo(.version_NO)
    }
    
    static func getDefaultUrlArgs() -> String {
        return "client=" + getAppName() + "&version=" + getAppVersion() + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE
    }
    
    static func getServersURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5servers.php?" + getDefaultUrlArgs()
    }

    static func getUpdateURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5update.php?" + getDefaultUrlArgs()
    }
    
    static func getWebLoginURL(username : String, passwd : String) -> String {
        
        let escUsername = username.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? username
        let escPasswd = passwd.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? passwd
        
        return "https://www.bearware.dk/test/weblogin.php?" + getDefaultUrlArgs() +
            "&service=bearware" + "&action=auth&username=" + escUsername + "&password=" + escPasswd
    }
}

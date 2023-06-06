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
    
    static let WEBLOGIN_BEARWARE_USERNAME = "bearware"
    static let WEBLOGIN_BEARWARE_USERNAMEPOSTFIX = "@bearware.dk"
    static let WEBLOGIN_BEARWARE_PASSWDPREFIX = "token="
    
    static func isBearWareWebLogin(_ username: String) -> Bool {
        	return username == AppInfo.WEBLOGIN_BEARWARE_USERNAME ||
                username.hasSuffix(AppInfo.WEBLOGIN_BEARWARE_USERNAMEPOSTFIX)
    }
    
    enum BundleInfo {
        case name, version_NO, build_NO
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
        case .build_NO :
            if let info = dict?["CFBundleVersion"] {
                return info as! String
            }
            return "0"
        }
    }
    
    static func getAppName() -> String {
        return getBundleInfo(.name)
    }
    static func getAppVersion() -> String {
        return getBundleInfo(.version_NO)
    }
    static func getAppVersionLong() -> String {
        return getAppVersion() + " Build " + getBundleInfo(.build_NO)
    }

    static func getDefaultUrlArgs() -> String {
        return "client=" + getAppName() + "&version=" + getAppVersion() + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE
    }
    
    static func getServersURL(officialservers: Bool, unofficialservers: Bool) -> String {
        let officialstr = officialservers ? "1" : "0"
        let unofficialstr = unofficialservers ? "1" : "0"
        return "http://www.bearware.dk/teamtalk/tt5servers.php?official=\(officialstr)&unofficial=\(unofficialstr)&" + getDefaultUrlArgs()
    }

    static func getUpdateURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5update.php?" + getDefaultUrlArgs()
    }

    static var BEARWARE_REGISTRATION_WEBSITE = "http://www.bearware.dk"

    static func getBearWareTokenURL(username : String, passwd : String) -> String {
        
        let escUsername = username.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? username
        let escPasswd = passwd.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? passwd
        
        return "https://www.bearware.dk/teamtalk/weblogin.php?" + getDefaultUrlArgs() +
            "&service=bearware" + "&action=auth&username=" + escUsername + "&password=" + escPasswd
    }
    
    static func getBearWareServerTokenURL(username : String, token : String, accesstoken : String) -> String {
        
        let escUsername = username.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? username
        let escToken = token.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? token
        let escAccessToken = accesstoken.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? accesstoken
        
        return "https://www.bearware.dk/teamtalk/weblogin.php?" + getDefaultUrlArgs() +
            "&service=bearware" + "&action=clientauth&username=" + escUsername +
            "&token=" + escToken + "&accesstoken=" + escAccessToken
    }
}

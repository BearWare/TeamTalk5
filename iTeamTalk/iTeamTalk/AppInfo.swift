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

import UIKit

class AppInfo {

    static let OSTYPE = "iOS"

    enum BundleInfo {
        case NAME, VERSION_NO
    }
    
    static func getBundleInfo(b: BundleInfo) -> String {
        let bundle = NSBundle.mainBundle()
        let dict = bundle.infoDictionary

        switch b {
        case .NAME :
            if let info = dict?["CFBundleName"] {
                return info as! String
            }
            return "Unknown"
        case .VERSION_NO :
            if let info = dict?["CFBundleShortVersionString"] {
                return info as! String
            }
            return "0.1"
        }
    }
    
    static func getAppName() -> String {
        return getBundleInfo(.NAME)
    }
    static func getAppVersion() -> String {
        return getBundleInfo(.VERSION_NO)
    }
    
    static func getServersURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5servers.php?client=" + getAppName() +
            "&version=" + getAppVersion() +
            "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE

    }

    static func getUpdateURL() -> String {
        return "http://www.bearware.dk/teamtalk/tt5update.php?client=" + getAppName() +
            "&version=" + getAppVersion() + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE

    }

}

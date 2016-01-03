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


class AppInfo {

    static let TEAMTALK_VERSION = "5.1"
    static let APPNAME = "iTeamTalk"
    static let APPVERSION_SHORT = "5.1"
    static let APPVERSION = "5.1 - Unreleased"
    static let APPTITLE = "iTeamTalk v. " + APPVERSION
    static let APPNAME_SHORT = "iTeamTalk5"
    static let OSTYPE = "iOS"

    static let URL_FREESERVER  =
        "http://www.bearware.dk/teamtalk/tt5servers.php?client=" + APPNAME_SHORT +
        "&version=" + APPVERSION_SHORT +
        "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE
    
    static let URL_APPUPDATE =
        "http://www.bearware.dk/teamtalk/tt5update.php?client=" + APPNAME_SHORT +
        "&version=" + APPVERSION_SHORT + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE

}
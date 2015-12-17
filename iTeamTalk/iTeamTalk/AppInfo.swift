//
//  AppInfo.swift
//  iTeamTalk
//
//  Created by Bjoern Rasmussen on 3-09-15.
//  Copyright (c) 2015 BearWare.dk. All rights reserved.
//


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
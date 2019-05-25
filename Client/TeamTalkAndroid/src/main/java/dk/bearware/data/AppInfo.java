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

package dk.bearware.data;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;

import dk.bearware.TeamTalkBase;

public class AppInfo {
    public static final String TAG = "bearware";

    public static final String APPNAME_SHORT = "TeamTalk5Droid";
    public static final String APPVERSION_POSTFIX = "";
    public static final String OSTYPE = "Android";

    public static final String WEBLOGIN_FACEBOOK_USERNAME = "facebook";
    public static final String WEBLOGIN_FACEBOOK_PASSWDPREFIX = "token=";

    public static final String WEBLOGIN_BEARWARE_USERNAME = "bearware";
    public static final String WEBLOGIN_BEARWARE_USERNAMEPOSTFIX = "@bearware.dk";

    // getUpdateUrl() response can override this
    public static String BEARWARE_REGISTRATION_WEBSITE = "http://www.bearware.dk";

    public static String getVersion(Context context) {
        String version = "";
        try {
            version = context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionName;
        }
        catch(NameNotFoundException e) {
            Log.e(TAG, "Unable to get version information");
        }
        return version;
    }

    public static String getDefautlUrlArgs(Context context) {
        final String TEAMTALK_VERSION = TeamTalkBase.getVersion();
        String appversion = getVersion(context);
        return "client=" + APPNAME_SHORT + "&version="
                + appversion + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE;
    }

    public static String getServerListURL(Context context) {
        String urlToRead = "http://www.bearware.dk/teamtalk/tt5servers.php?" + getDefautlUrlArgs(context);
        return urlToRead;
    }

    public static String getUpdateURL(Context context) {
        String urlToRead = "http://www.bearware.dk/teamtalk/tt5update.php?" + getDefautlUrlArgs(context);
        return urlToRead;
    }

    public static String getBearWareTokenUrl(Context context, String username, String password) {

        try {
            username = URLEncoder.encode(username, "utf-8");
            password = URLEncoder.encode(password, "utf-8");
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Unable to encode username/password: " + e.toString());
        }
        String urlToRead = "https://www.bearware.dk/teamtalk/weblogin.php?" + getDefautlUrlArgs(context) +
                "&service=bearware&action=auth&username=" + username + "&password=" + password;
        return urlToRead;
    }

    public static String getBearWareAccessTokenUrl(Context context, String username, String token, String accesstoken) {

        try {
            username = URLEncoder.encode(username, "utf-8");
            token = URLEncoder.encode(token, "utf-8");
            accesstoken = URLEncoder.encode(accesstoken, "utf-8");
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Unable to encode username/token: " + e.toString());
        }
        String urlToRead = "https://www.bearware.dk/teamtalk/weblogin.php?" + getDefautlUrlArgs(context) +
                "&service=bearware&action=clientauth&username=" + username + "&token=" + token +
                "&accesstoken=" + accesstoken;
        return urlToRead;
    }

}

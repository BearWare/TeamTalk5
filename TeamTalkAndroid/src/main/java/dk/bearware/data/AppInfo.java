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

package dk.bearware.data;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;
import dk.bearware.TeamTalkBase;

public class AppInfo {
    public static final String TAG = "bearware";

    public static final String APPNAME_SHORT = "TeamTalk5Droid";
    public static final String APPVERSION_POSTFIX = " - Unreleased";
    public static final String OSTYPE = "Android";

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
    public static String getServerListURL(Context context) {
        final String TEAMTALK_VERSION = TeamTalkBase.getVersion();
        String appversion = getVersion(context);
        String urlToRead = "http://www.bearware.dk/teamtalk/tt5servers.php?client=" + APPNAME_SHORT + "&version="
            + appversion + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE;
        return urlToRead;
    }

    public static String getUpdateURL(Context context) {
        final String TEAMTALK_VERSION = TeamTalkBase.getVersion();
        String appversion = getVersion(context);
        String urlToRead = "http://www.bearware.dk/teamtalk/tt5update.php?client=" + APPNAME_SHORT + "&version="
            + appversion + "&dllversion=" + TEAMTALK_VERSION + "&os=" + OSTYPE;
        return urlToRead;
    }
    
}

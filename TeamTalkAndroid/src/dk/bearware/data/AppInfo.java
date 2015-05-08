package dk.bearware.data;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;
import dk.bearware.TeamTalkBase;

public class AppInfo {
    public static final String TAG = "bearware";

    public static final String APPNAME_SHORT = "TeamTalk5";
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

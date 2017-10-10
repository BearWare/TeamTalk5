/*
 * Copyright (c) 2005-2017, BearWare.dk
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

package dk.bearware.data;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.widget.Toast;

import dk.bearware.gui.R;

public class Permissions {

    public static final int MY_PERMISSIONS_REQUEST_RECORD_AUDIO = 1,
    MY_PERMISSIONS_REQUEST_MODIFY_AUDIO_SETTINGS = 2,
    MY_PERMISSIONS_REQUEST_INTERNET = 3,
    MY_PERMISSIONS_REQUEST_VIBRATE = 4,
    MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE = 5,
    MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE = 6,
    MY_PERMISSIONS_REQUEST_WAKE_LOCK = 7;

    public static boolean setupPermission(Context context, Activity activity, int permission) {
        String stringPermission;
        String errormessage;

        switch(permission) {
            case MY_PERMISSIONS_REQUEST_RECORD_AUDIO :
                stringPermission = Manifest.permission.RECORD_AUDIO;
                errormessage = context.getString(R.string.permission_audioinput);
                break;
            case MY_PERMISSIONS_REQUEST_MODIFY_AUDIO_SETTINGS :
                stringPermission = Manifest.permission.MODIFY_AUDIO_SETTINGS;
                errormessage = context.getString(R.string.permission_audiomodify);
                break;
            case MY_PERMISSIONS_REQUEST_INTERNET :
                stringPermission = Manifest.permission.INTERNET;
                errormessage = context.getString(R.string.permission_internet);
                break;
            case MY_PERMISSIONS_REQUEST_VIBRATE :
                stringPermission = Manifest.permission.VIBRATE;
                errormessage = context.getString(R.string.permission_vibrate);
                break;
            case MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE :
                stringPermission = Manifest.permission.READ_EXTERNAL_STORAGE;
                errormessage = context.getString(R.string.permission_filetx);
                break;
            case MY_PERMISSIONS_REQUEST_WRITE_EXTERNAL_STORAGE :
                stringPermission = Manifest.permission.WRITE_EXTERNAL_STORAGE;
                errormessage = context.getString(R.string.permission_filerx);
                break;
            case MY_PERMISSIONS_REQUEST_WAKE_LOCK:
                stringPermission = Manifest.permission.WAKE_LOCK;
                errormessage = context.getString(R.string.permission_wake_lock);
                break;
            default :
                Log.e(AppInfo.TAG, String.format("Unknown permission %d", permission));
                return false;
        }

        int permissionCheck = ContextCompat.checkSelfPermission(context,
                stringPermission);

        if (permissionCheck != PackageManager.PERMISSION_GRANTED) {

            Toast.makeText(context, errormessage, Toast.LENGTH_LONG).show();

            if (ActivityCompat.shouldShowRequestPermissionRationale(activity,
                    stringPermission)) {

            }
            else {
                ActivityCompat.requestPermissions(activity,
                        new String[]{stringPermission},
                        permission);
            }
            return false;
        }
        return true;
    }
}

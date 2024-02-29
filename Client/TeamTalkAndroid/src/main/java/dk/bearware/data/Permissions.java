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

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import dk.bearware.gui.R;

@SuppressLint("InlinedApi")
public enum Permissions {

    RECORD_AUDIO(Manifest.permission.RECORD_AUDIO, R.string.permission_audioinput),
    MODIFY_AUDIO_SETTINGS(Manifest.permission.MODIFY_AUDIO_SETTINGS, R.string.permission_audiomodify),
    INTERNET(Manifest.permission.INTERNET, R.string.permission_internet),
    VIBRATE(Manifest.permission.VIBRATE, R.string.permission_vibrate),
    READ_EXTERNAL_STORAGE(Manifest.permission.READ_EXTERNAL_STORAGE, R.string.permission_filetx),
    WRITE_EXTERNAL_STORAGE(Manifest.permission.WRITE_EXTERNAL_STORAGE, R.string.permission_filerx),
    WAKE_LOCK(Manifest.permission.WAKE_LOCK, R.string.permission_wake_lock),
    READ_PHONE_STATE(Manifest.permission.READ_PHONE_STATE, R.string.permission_read_phone_state),
    BLUETOOTH((Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) ? Manifest.permission.BLUETOOTH_CONNECT : Manifest.permission.BLUETOOTH, R.string.permission_bluetooth),
    POST_NOTIFICATIONS(Manifest.permission.POST_NOTIFICATIONS, R.string.permission_post_notifications);

    private final String id;
    private final int msgResId;

    Permissions(String id, int msgResId) {
        this.id = id;
        this.msgResId = msgResId;
    }

    public boolean isGranted(@NonNull Context context) {
        return ContextCompat.checkSelfPermission(context, id) == PackageManager.PERMISSION_GRANTED;
    }

    public boolean request(@NonNull Activity activity) {
        boolean state = isGranted(activity.getBaseContext());
        if (!state) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(activity, id)) {
                Toast.makeText(activity.getBaseContext(), msgResId, Toast.LENGTH_LONG).show();
            } else {
                ActivityCompat.requestPermissions(activity, new String[]{id}, ordinal() + 1);
            }
        }
        return state;
    }

    @Nullable
    public static Permissions onRequestResult(@NonNull Activity activity, int requestCode, @NonNull int[] grantResults) {
        Permissions permission = ((requestCode > 0) && (requestCode <= values().length)) ? values()[requestCode - 1] : null;
        boolean granted = (grantResults.length > 0) && (grantResults[0] == PackageManager.PERMISSION_GRANTED);
        if ((permission != null) && !granted) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(activity, permission.id)) {
                Toast.makeText(activity.getBaseContext(), permission.msgResId, Toast.LENGTH_LONG).show();
            }
            return null;
        }
        return permission;
    }

}

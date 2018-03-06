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

package dk.bearware.backend;

import dk.bearware.TeamTalkBase;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

public class TeamTalkConnection implements ServiceConnection {
    
    public String TAG = "bearware";
    
    TeamTalkConnectionListener ttlistener;
    TeamTalkService ttservice;
    
    public TeamTalkConnection(TeamTalkConnectionListener listener) {
        ttlistener = listener;
    }
    
    @Override
    public void onServiceConnected(ComponentName className, IBinder service) {
        // We've bound to LocalService, cast the IBinder and get LocalService instance
        TeamTalkService.LocalBinder binder = (TeamTalkService.LocalBinder) service;

        ttservice = binder.getService();
        TeamTalkBase ttclient = ttservice.getTTInstance();
        
        String s = new String();
        s = "TeamTalk instance 0x" +
            Integer.toHexString(ttclient.hashCode() & 0xFFFFFFFF) + 
            " running v. " + TeamTalkBase.getVersion() + " connected";
        Log.i(TAG, s);

        setBound(true);
        ttlistener.onServiceConnected(ttservice);
    }

    @Override
    public void onServiceDisconnected(ComponentName arg0) {
        ttlistener.onServiceDisconnected(ttservice);
        setBound(false);

        TeamTalkBase ttclient = ttservice.getTTInstance();
        
        String s = new String();
        s = "TeamTalk instance 0x" +
            Integer.toHexString(ttclient.hashCode() & 0xFFFFFFFF) + 
            " disconnected";
        Log.i(TAG, s);
        
        ttservice = null;
    }
    
    boolean bound = false;
    public void setBound(boolean bound) {
        this.bound = bound;
    }
    public boolean isBound() {
        return bound;
    }
}

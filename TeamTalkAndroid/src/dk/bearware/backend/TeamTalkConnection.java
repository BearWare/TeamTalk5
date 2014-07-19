package dk.bearware.backend;

import dk.bearware.TeamTalkBase;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

public class TeamTalkConnection implements ServiceConnection {
    
    public String tag = "bearware";
    
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
            " running v. " + ttclient.GetVersion() + " connected";
        Log.i(tag, s);
        
        ttlistener.onServiceConnected(ttservice);
    }

    @Override
    public void onServiceDisconnected(ComponentName arg0) {
        ttlistener.onServiceDisconnected(ttservice);
        
        TeamTalkBase ttclient = ttservice.getTTInstance();
        
        String s = new String();
        s = "TeamTalk instance 0x" +
            Integer.toHexString(ttclient.hashCode() & 0xFFFFFFFF) + 
            " disconnected";
        Log.i(tag, s);
        
        ttservice = null;
    }
}

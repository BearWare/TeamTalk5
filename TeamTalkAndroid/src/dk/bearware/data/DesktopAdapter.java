package dk.bearware.data;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import dk.bearware.BitmapFormat;
import dk.bearware.DesktopInput;
import dk.bearware.DesktopWindow;
import dk.bearware.MediaFileInfo;
import dk.bearware.User;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.UserListener;
import dk.bearware.gui.R;
import dk.bearware.gui.Utils;
import android.content.Context;
import android.graphics.Bitmap;
import android.os.CountDownTimer;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class DesktopAdapter
extends BaseExpandableListAdapter
implements UserListener
{
    private LayoutInflater inflater;
    
    TeamTalkService ttservice;
    SparseArray<User> display_users = new SparseArray<User>();
    SparseArray<Bitmap> bitmap_users = new SparseArray<Bitmap>();
    
    public DesktopAdapter(Context context) {
        inflater = LayoutInflater.from(context);
    }
    
    public void setTeamTalkService(TeamTalkService service) {
        ttservice = service;
        service.registerUserListener(this);

        display_users.clear();
        bitmap_users.clear();

        Vector<User> vecusers = Utils.getUsers(ttservice.getUsers());
        for(User user : vecusers) {
            if((user.uUserState & UserState.USERSTATE_DESKTOP) == UserState.USERSTATE_DESKTOP)
                display_users.put(user.nUserID, user);
        }
    }
    
    @Override
    public void onGroupCollapsed(int groupPosition) {
        int key = (int)getGroupId(groupPosition);
        bitmap_users.delete(key);
        int size = bitmap_users.size();
        super.onGroupCollapsed(groupPosition);
    }

    @Override
    public void onGroupExpanded(int groupPosition) {
        updateUserBitmap((int)getGroupId(groupPosition));
        super.onGroupExpanded(groupPosition);
    }
    
    @Override
    public Object getChild(int groupPosition, int childPosition) {
        return (User) getGroup(groupPosition);
    }

    @Override
    public long getChildId(int groupPosition, int childPosition) {
        return getGroupId(groupPosition);
    }

    @Override
    public View getChildView(int groupPosition, int childPosition,
                             boolean isLastChild, View convertView, ViewGroup parent) {
        User user = (User) getChild(groupPosition, childPosition);
        if(convertView == null)
            convertView = inflater.inflate(R.layout.item_desktop, null);
        
        Bitmap bmp = bitmap_users.get(user.nUserID);
        if(bmp != null)  {
            ImageView desktop = (ImageView) convertView.findViewById(R.id.user_desktop_image);
            desktop.setImageBitmap(bmp);
        }
        return convertView;
    }

    @Override
    public int getChildrenCount(int groupPosition) {
        return 1;
    }

    @Override
    public Object getGroup(int groupPosition) {
        return display_users.get((int)getGroupId(groupPosition));
    }

    @Override
    public int getGroupCount() {
        return display_users.size();
    }

    @Override
    public long getGroupId(int groupPosition) {
        return display_users.keyAt(groupPosition);
    }

    @Override
    public View getGroupView(int groupPosition, boolean isExpanded,
                             View convertView, ViewGroup parent) {
        User user = (User) getGroup(groupPosition);
        
        if(convertView == null)
            convertView = inflater.inflate(R.layout.item_desktop_user, null);
        
        TextView nickname = (TextView) convertView.findViewById(R.id.desktop_nickname_textview);
        TextView wndinfo = (TextView) convertView.findViewById(R.id.desktopinfo_textview);
        nickname.setText(user.szNickname);

        Bitmap bmp = bitmap_users.get(user.nUserID); 
        if(bmp != null) {
            wndinfo.setText(String.format("%1$dx%2$d %3$d-bit", bmp.getWidth(), 
                            bmp.getHeight(), (bmp.getConfig() == Bitmap.Config.ARGB_8888)? 32 : 0));
        }
        else {
            wndinfo.setText("");
        }
        
        return convertView;
    }

    @Override
    public boolean hasStableIds() {
        return true;
    }

    @Override
    public boolean isChildSelectable(int groupPosition, int childPosition) {
        return false;
    }
    
    CountDownTimer timer;
    
    @Override
    public void notifyDataSetChanged() {
        //ensure we don't do too many updates
        if(timer == null)
            super.notifyDataSetChanged();
        else {
            timer = new CountDownTimer(100, 100) {
                public void onTick(long millisUntilFinished) {
                }

                public void onFinish() {
                    timer = null;
                    notifyDataSetChanged();
                }
            }.start();
        }
    }

    @Override
    public void onUserStateChange(User user) {

        if(display_users.get(user.nUserID) != null &&
           (user.uUserState & UserState.USERSTATE_DESKTOP) == 0) {
            display_users.delete(user.nUserID);
            bitmap_users.delete(user.nUserID);
            
            notifyDataSetChanged();
        }
        else if(display_users.get(user.nUserID) == null &&
                (user.uUserState & UserState.USERSTATE_DESKTOP) == UserState.USERSTATE_DESKTOP) {
            display_users.put(user.nUserID, user);
            
            notifyDataSetChanged();
        }
    }

    @Override
    public void onUserVideoCapture(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserMediaFileVideo(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopWindow(int nUserID, int nStreamID) {
        //only update if user's bitmap is being displayed
        if(bitmap_users.get(nUserID) != null && updateUserBitmap(nUserID))
            notifyDataSetChanged();
    }
    
    boolean updateUserBitmap(int userid) {

        DesktopWindow wnd = ttservice.getTTInstance().acquireUserDesktopWindow(userid);
        // TODO: only RGB32 support for now 
        if(wnd == null || wnd.bmpFormat != BitmapFormat.BMP_RGB32)
            return false;
        
        Bitmap bmp = bitmap_users.get(userid); 
        if(bmp != null) {
            // create new bitmap if size 
            if(bmp.getWidth() != wnd.nWidth || bmp.getHeight() != wnd.nHeight)
                bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
        }
        else {
            bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
        }
        
        bmp.copyPixelsFromBuffer(ByteBuffer.wrap(wnd.frameBuffer));
        bitmap_users.put(userid, bmp);
        
        return true;
    }

    @Override
    public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput) {
    }

    @Override
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
    }
}

/*
 * Copyright (c) 2005-2014, BearWare.dk
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

import java.util.LinkedList;

import dk.bearware.User;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.gui.R;
import dk.bearware.gui.Utils;
import android.content.Context;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.util.Log;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public abstract class ImageAdapter extends BaseExpandableListAdapter {
    public static final String TAG = "bearware";

    private LayoutInflater inflater;

    TeamTalkService ttservice;
    Context context;
    SparseArray<User> display_users = new SparseArray<User>();
    SparseArray<Bitmap> bitmap_users = new SparseArray<Bitmap>();

    public ImageAdapter(Context context) {
        this.context = context;
        inflater = LayoutInflater.from(context);
    }

    public void setTeamTalkService(TeamTalkService service) {
    	display_users.clear();
    	bitmap_users.clear();
    	ttservice = service;
    }

    public void clearTeamTalkService(TeamTalkService service) {
    	display_users.clear();
    	bitmap_users.clear();
    	
    	synchronized (updatequeue) {
    		updatequeue.clear();
    		updatequeue.notify();
    	}
    }

    public void updateUserStreamState(User user, int uUserState) {
        if (display_users.get(user.nUserID) != null
                && (user.uUserState & uUserState) == 0) {
            display_users.delete(user.nUserID);
            bitmap_users.delete(user.nUserID);

            notifyDataSetChanged();
        } else if (display_users.get(user.nUserID) == null
                && (user.uUserState & uUserState) == uUserState) {
            display_users.put(user.nUserID, user);

            notifyDataSetChanged();
        }
    }

    @Override
    public void onGroupCollapsed(int groupPosition) {
        int userid = (int) getGroupId(groupPosition);
        bitmap_users.delete(userid);

        synchronized (updatequeue) {
            updatequeue.removeFirstOccurrence(userid);
        }
        super.onGroupCollapsed(groupPosition);
    }

    @Override
    public void onGroupExpanded(int groupPosition) {
        int userid = (int) getGroupId(groupPosition);
        // Bitmap bmp = extractUserBitmap(userid, bitmap_users.get(userid));
        String text = context.getResources()
                .getString(R.string.text_waitstream);
        Bitmap bmp = Utils.drawTextToBitmap(context, 100, 20, text);
        bitmap_users.put(userid, bmp);
        updateUserBitmap(userid);
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
        if (convertView == null)
            convertView = inflater.inflate(R.layout.item_desktop, null);

        Bitmap bmp = bitmap_users.get(user.nUserID);
        if (bmp != null) {
            ImageView desktop = (ImageView) convertView
                    .findViewById(R.id.user_desktop_image);
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
        return display_users.get((int) getGroupId(groupPosition));
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

        if (convertView == null)
            convertView = inflater.inflate(R.layout.item_desktop_user, null);

        TextView nickname = (TextView) convertView
                .findViewById(R.id.desktop_nickname_textview);
        TextView wndinfo = (TextView) convertView
                .findViewById(R.id.desktopinfo_textview);
        nickname.setText(user.szNickname);

        Bitmap bmp = bitmap_users.get(user.nUserID);
        if (bmp != null) {
            wndinfo.setText(String.format("%1$dx%2$d %3$d-bit", bmp.getWidth(),
                    bmp.getHeight(),
                    (bmp.getConfig() == Bitmap.Config.ARGB_8888) ? 32 : 0));
        } else {
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

    LinkedList<Integer> updatequeue = new LinkedList<Integer>();
    ImageUpdateAsyncTask updatetask;

    public void updateUserBitmap(int userid) {

        synchronized (updatequeue) {

            if (!updatequeue.contains(userid)) {
                updatequeue.add(userid);
            } else {
                // Log.d(TAG, "Skipped update for #"+ userid);
            }

            if (updatetask == null) {
                updatetask = new ImageUpdateAsyncTask();
                updatetask.execute();
            }

            updatequeue.notify();
        }
    }

    class ImageUpdateAsyncTask extends AsyncTask<Void, Integer, Void> {

        SparseArray<Bitmap> ready_users;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            ready_users = bitmap_users.clone();
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);

            int userid = values[0];
            Bitmap bmp;
            synchronized (ready_users) {
                bmp = ready_users.get(userid);
                ready_users.remove(userid);
            }

            // only place new bitmap if user is being displayed
            if (bitmap_users.indexOfKey(userid) >= 0)
                bitmap_users.put(userid, bmp);

            ImageAdapter.this.notifyDataSetChanged();
        }

        @Override
        protected void onPostExecute(Void result) {
            updatetask = null;
        }

        @Override
        protected Void doInBackground(Void... params) {

            int userid;
            do {

                userid = 0;

                synchronized (updatequeue) {
                    if (updatequeue.isEmpty()) {
                        try {
                            updatequeue.wait(10000);
                        } catch (InterruptedException e) {
                            Log.d(TAG,
                                    "Interrupted exception for image retrieve");
                        }
                    }

                    if (updatequeue.size() > 0)
                        userid = updatequeue.remove();
                }

                if (userid > 0) {
                    Bitmap bmp;
                    synchronized (ready_users) {
                        bmp = ready_users.get(userid);
                    }
                    bmp = extractUserBitmap(userid, bmp);
                    if (bmp != null) {
                        // Log.d(TAG,
                        // "Received "+bmp.getWidth()+"x"+bmp.getHeight()+" bitmap from #"
                        // + userid);
                        synchronized (ready_users) {
                            ready_users.put(userid, bmp);
                        }
                        this.publishProgress(userid);
                    } else {
                        // Log.d(TAG, "Bitmap is null from #" + userid);
                    }
                }
            } while (userid != 0);

            return null;
        }
    }

    public abstract Bitmap extractUserBitmap(int userid, Bitmap prev_bmp);

}

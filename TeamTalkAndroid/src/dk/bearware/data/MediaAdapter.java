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

import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.Vector;

import dk.bearware.BitmapFormat;
import dk.bearware.DesktopInput;
import dk.bearware.DesktopWindow;
import dk.bearware.MediaFileInfo;
import dk.bearware.User;
import dk.bearware.UserState;
import dk.bearware.VideoFrame;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.UserListener;
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

public class MediaAdapter
extends BaseExpandableListAdapter
implements UserListener {
	
    public static final String TAG = "bearware";

    private LayoutInflater inflater;

    TeamTalkService ttservice;
    Context context;
    static final int DESKTOP_SESSION   	= 0x00001000,
    				 WEBCAM_SESSION    	= 0x00002000,
    				 MEDIAFILE_SESSION 	= 0x00004000,
    				 
    				 USERID_MASK 		= 0xFFF;
    
    SparseArray<User> display_users = new SparseArray<User>();
    SparseArray<Bitmap> media_sessions = new SparseArray<Bitmap>();

    public MediaAdapter(Context context) {
        this.context = context;
        inflater = LayoutInflater.from(context);
    }

    public void setTeamTalkService(TeamTalkService service) {
        display_users.clear();
        media_sessions.clear();
        ttservice = service;
        
        service.registerUserListener(this);
        
        Vector<User> vecusers = Utils.getUsers(ttservice.getUsers());
        for(User user : vecusers) {
            if((user.uUserState & UserState.USERSTATE_DESKTOP) == UserState.USERSTATE_DESKTOP)
                display_users.put(user.nUserID | DESKTOP_SESSION, user);
            if((user.uUserState & UserState.USERSTATE_MEDIAFILE_VIDEO) == UserState.USERSTATE_MEDIAFILE_VIDEO)
                display_users.put(user.nUserID | MEDIAFILE_SESSION, user);
            if((user.uUserState & UserState.USERSTATE_VIDEOCAPTURE) == UserState.USERSTATE_VIDEOCAPTURE)
                display_users.put(user.nUserID | WEBCAM_SESSION, user);
        }
    }

    public void clearTeamTalkService(TeamTalkService service) {
        display_users.clear();
        media_sessions.clear();
        
        synchronized (updatequeue) {
            updatequeue.clear();
            updatequeue.notify();
        }
    }

    @Override
    public void onGroupCollapsed(int groupPosition) {
        int userid = (int) getGroupId(groupPosition);
        media_sessions.delete(userid);

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
        Bitmap bmp = Utils.drawTextToBitmap(context, 300, 20, text);
        media_sessions.put(userid, bmp);
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
    	int userid = (int) getGroupId(groupPosition);
        if (convertView == null)
            convertView = inflater.inflate(R.layout.item_media, null);

        Bitmap bmp = media_sessions.get(userid);
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
        int userid = (int) getGroupId(groupPosition);

        if (convertView == null)
            convertView = inflater.inflate(R.layout.item_media_user, null);

        TextView nickname = (TextView) convertView
                .findViewById(R.id.media_nickname_textview);
        TextView wndinfo = (TextView) convertView
                .findViewById(R.id.mediainfo_textview);
        nickname.setText(user.szNickname);
        ImageView img = (ImageView) convertView.findViewById(R.id.mediaicon);
        int img_resource;
        switch(userid & ~USERID_MASK) {
        default :
        case DESKTOP_SESSION :
        	img_resource = R.drawable.board_blue;
        	img.setContentDescription(context.getString(R.string.text_desktop_session));
        	break;
        case WEBCAM_SESSION :
        	img_resource = R.drawable.webcam_pink;
        	img.setContentDescription(context.getString(R.string.text_webcam_session));
        	break;
        case MEDIAFILE_SESSION :
        	img_resource = R.drawable.camera_blue;
        	img.setContentDescription(context.getString(R.string.text_mediafile_session));
        	break;
        }
        img.setImageResource(img_resource);

        Bitmap bmp = media_sessions.get(userid);
        if (bmp != null) {
            wndinfo.setText(String.format("%1$dx%2$d %3$d-bit", bmp.getWidth(),
                    bmp.getHeight(),
                    (bmp.getConfig() == Bitmap.Config.ARGB_8888) ? 32 : 0));
        } else {
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

            ready_users = media_sessions.clone();
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);

            int userid = values[0];
            Bitmap bmp;
            synchronized (ready_users) {
                bmp = ready_users.get(userid);
            }

            // only place new bitmap if user is being displayed
            if (media_sessions.indexOfKey(userid) >= 0)
            	media_sessions.put(userid, bmp);
            
            //update currently active sessions (get rid of obsolete sessions)
            ready_users = media_sessions.clone();
            
            MediaAdapter.this.notifyDataSetChanged();
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

    public Bitmap extractUserBitmap(int userid, Bitmap prev_bmp) {
    	
    	switch(userid & ~USERID_MASK) {
    	case DESKTOP_SESSION : {
    		userid &= USERID_MASK;
    		
    		DesktopWindow wnd = ttservice.getTTInstance().acquireUserDesktopWindowEx(userid,
    				BitmapFormat.BMP_RGB32);
    		// TODO: only RGB32 support for now 
    		if(wnd == null || wnd.bmpFormat != BitmapFormat.BMP_RGB32)
    			return null;

    		if(prev_bmp != null) {
    			// create new bitmap if size 
    			if(prev_bmp.getWidth() != wnd.nWidth || prev_bmp.getHeight() != wnd.nHeight)
    				prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
    		}
    		else {
    			prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
    		}

    		prev_bmp.copyPixelsFromBuffer(ByteBuffer.wrap(wnd.frameBuffer));

    		return prev_bmp;
    	}
    	case MEDIAFILE_SESSION : {
    		userid &= USERID_MASK;
    		
            VideoFrame wnd = ttservice.getTTInstance().acquireUserMediaVideoFrame(userid);
            
            if(wnd == null) {
                return null;
            }
            
            if(prev_bmp != null) {
                // create new bitmap if size 
                if(prev_bmp.getWidth() != wnd.nWidth || prev_bmp.getHeight() != wnd.nHeight)
                    prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
            }
            else {
                prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
            }
            
            prev_bmp.copyPixelsFromBuffer(ByteBuffer.wrap(wnd.frameBuffer));
            
            return prev_bmp;
    	}
    	case WEBCAM_SESSION : {
    		userid &= USERID_MASK;
    		
            VideoFrame wnd = ttservice.getTTInstance().acquireUserVideoCaptureFrame(userid);
            
            if(wnd == null) {
                return null;
            }
            
            if(prev_bmp != null) {
                // create new bitmap if size 
                if(prev_bmp.getWidth() != wnd.nWidth || prev_bmp.getHeight() != wnd.nHeight)
                    prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
            }
            else {
                prev_bmp = Bitmap.createBitmap(wnd.nWidth, wnd.nHeight, Bitmap.Config.ARGB_8888);
            }
            
            prev_bmp.copyPixelsFromBuffer(ByteBuffer.wrap(wnd.frameBuffer));
            
            return prev_bmp;
    	}
    	default : {
    		Log.e(TAG, "Unknown media session");
    		return null;
    	}
    	}
    }
    
	@Override
	public void onUserStateChange(User user) {
		int ms_size = media_sessions.size();
		int du_size = display_users.size();
		
		//desktop session
		int userid = user.nUserID | DESKTOP_SESSION;
		if((user.uUserState & UserState.USERSTATE_DESKTOP) == 0) {
			display_users.remove(userid);
			media_sessions.remove(userid);
		}
		else if(display_users.get(userid) == null) {
			display_users.put(userid, user);
		}
		
		//media file session
		userid = user.nUserID | MEDIAFILE_SESSION;
		if((user.uUserState & UserState.USERSTATE_MEDIAFILE_VIDEO) == 0) {
			display_users.remove(userid);
			media_sessions.remove(userid);
		}
		else if(display_users.get(userid) == null) {
			display_users.put(userid, user);
		}
		
		//vidcap session
		userid = user.nUserID | WEBCAM_SESSION;
		if((user.uUserState & UserState.USERSTATE_VIDEOCAPTURE) == 0) {
			display_users.remove(userid);
			media_sessions.remove(userid);
			Log.d(TAG, "Remove webcam session #" + user.nUserID);
		}
		else if(display_users.get(userid) == null) {
			display_users.put(userid, user);
		}
		
		
		if(ms_size != media_sessions.size() || du_size != display_users.size())
			notifyDataSetChanged();
	}

	@Override
	public void onUserVideoCapture(int nUserID, int nStreamID) {
        //only update if user is expanded (bitmap is being displayed)
		int session_id = nUserID | WEBCAM_SESSION;
        if(media_sessions.indexOfKey(session_id) >= 0)
            updateUserBitmap(session_id);
	}

	@Override
	public void onUserMediaFileVideo(int nUserID, int nStreamID) {
        //only update if user is expanded (bitmap is being displayed)
		int session_id = nUserID | MEDIAFILE_SESSION;
        if(media_sessions.indexOfKey(session_id) >= 0)
            updateUserBitmap(session_id);
	}

	@Override
	public void onUserDesktopWindow(int nUserID, int nStreamID) {
        //only update if user is expanded (bitmap is being displayed)
		int session_id = nUserID | DESKTOP_SESSION;
        if(media_sessions.indexOfKey(session_id) >= 0)
            updateUserBitmap(session_id);
	}

	@Override
	public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput) {
	}

	@Override
	public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
	}

	@Override
	public void onUserAudioBlock(int nUserID, int nStreamType) {
	}

}

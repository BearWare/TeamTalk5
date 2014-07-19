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

package dk.bearware.gui;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Vector;

import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientFlag;
import dk.bearware.DesktopInput;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.UserListener;
import dk.bearware.gui.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity
extends FragmentActivity
implements TeamTalkConnectionListener, OnItemClickListener, ConnectionListener, CommandListener, UserListener {

    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide fragments for each of the sections. We use a
     * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which will keep every loaded fragment in memory.
     * If this becomes too memory intensive, it may be best to switch to a
     * {@link android.support.v4.app.FragmentStatePagerAdapter}.
     */
    SectionsPagerAdapter mSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    ViewPager mViewPager;
    
    public String tag = "bearware";

    private enum CmdComplete {
        CMD_COMPLETE_LOGIN, CMD_COMPLETE_JOIN, CMD_COMPLETE_TEXTMSG
    }

    public final int REQUEST_EDITCHANNEL = 1,
                     REQUEST_NEWCHANNEL = 2,
                     REQUEST_EDITUSER = 3;

    Channel curchannel;

    Map<Integer, CmdComplete> activecmds = new HashMap<Integer, CmdComplete>();

    ChannelListAdapter adapter;

    public ChannelListAdapter getChannelsAdapter() {
        return adapter;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mConnection = new TeamTalkConnection(this);
        
        // Create the adapter that will return a fragment for each of the three
        // primary sections of the app.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);

        adapter = new ChannelListAdapter(this.getBaseContext());
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_newchannel : {
                Intent intent = new Intent(MainActivity.this, ChannelPropActivity.class);
                // TODO: check 'curchannel' for null
                startActivityForResult(intent.putExtra(ChannelPropActivity.EXTRA_PARENTID, curchannel.nChannelID),
                    REQUEST_NEWCHANNEL);
            }
            break;
            case R.id.action_transmit : {
                boolean b = !item.isChecked();
                item.setChecked(b);
                ttclient.EnableVoiceTransmission(b);
            }
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;

    @Override
    protected void onStart() {
        super.onStart();
        // Bind to LocalService
        Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
        boolean ret = bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        assert (ret);
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
        unbindService(mConnection);
    }

    ChannelsSectionFragment channelsFragment;
    ChatSectionFragment chatFragment;

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter {
        
        public static final int CHANNELS_PAGE = 0,
                                CHAT_PAGE = 1,
                                
                                PAGE_COUNT = 2;

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {

            // getItem is called to instantiate the fragment for the given page.

            switch(position) {
                default :
                case CHANNELS_PAGE : {
                    channelsFragment = new ChannelsSectionFragment();
                    return channelsFragment;
                }
                case CHAT_PAGE : {
                    chatFragment = new ChatSectionFragment();
                    return chatFragment;
                }
            }
        }

        @Override
        public int getCount() {
            return PAGE_COUNT;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch(position) {
                case CHANNELS_PAGE :
                    return getString(R.string.title_section1).toUpperCase(l);
                case CHAT_PAGE :
                    return getString(R.string.title_section2).toUpperCase(l);
            }
            return null;
        }
    }

    public static class ChannelsSectionFragment extends Fragment {
        MainActivity mainActivity;

        public ChannelsSectionFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_channels, container, false);

            ListView channelsList = (ListView) rootView.findViewById(R.id.listChannels);
            channelsList.setAdapter(mainActivity.getChannelsAdapter());
            channelsList.setOnItemClickListener(mainActivity);

            return rootView;
        }
    }

    public static class ChatSectionFragment extends Fragment {
        MainActivity mainActivity;
        
        public ChatSectionFragment() {
        }
        
        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_chat, container, false);
            final EditText newmsg = (EditText) rootView.findViewById(R.id.textmsg);
            EditText chatlog = (EditText)rootView.findViewById(R.id.chatlog);
            chatlog.setKeyListener(null);
            Button sendBtn = (Button) rootView.findViewById(R.id.textmsg_send_btn);
            sendBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View arg0) {
                    TextMessage textmsg = new TextMessage();
                    textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
                    textmsg.nChannelID = mainActivity.ttclient.GetMyChannelID();
                    textmsg.szMessage = newmsg.getText().toString();
                    int cmdid = mainActivity.ttclient.DoTextMessage(textmsg);
                    if(cmdid>0) {
                        mainActivity.activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_TEXTMSG);
                        newmsg.setText("");
                    }
                    else
                        Toast.makeText(mainActivity, "Failed to send channel message", Toast.LENGTH_LONG).show();
                }
            });
            return rootView;
        }
        
        public void addTextMessage(TextMessage textmsg) {
            EditText chatlog = (EditText)getView().findViewById(R.id.chatlog);
            
            User from = mainActivity.ttservice.getUsers().get(textmsg.nFromUserID);
            if(from != null)
                chatlog.append("<" + from.szNickname + "> " + textmsg.szMessage + "\n");
        }
    }

    class ChannelListAdapter extends BaseAdapter {

        private LayoutInflater inflater;

        Vector<Channel> subchannels = new Vector<Channel>();
        Vector<User> currentusers = new Vector<User>();

        ChannelListAdapter(Context context) {
            inflater = LayoutInflater.from(context);
        }

        @Override
        public void notifyDataSetChanged() {
            int chanid = 0;

            subchannels.clear();
            currentusers.clear();

            if(curchannel != null) {
                chanid = curchannel.nChannelID;

                subchannels = Utils.getSubChannels(chanid, ttservice.getChannels());
                currentusers = Utils.getUsers(chanid, ttservice.getUsers());
            }
            else {
                chanid = ttclient.GetRootChannelID();
                Channel root = ttservice.getChannels().get(chanid);
                if(root != null)
                    subchannels.add(root);
            }

            super.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            int count = subchannels.size() + currentusers.size();
            if(curchannel != null) {
                count++; // include parent channel shortcut
            }
            return count;
        }

        @Override
        public Object getItem(int position) {
            if(curchannel != null) {
                if(position == 0) {
                    Channel parent = ttservice.getChannels().get(curchannel.nParentID);

                    if(parent != null)
                        return parent;
                    return new Channel();
                }

                position--; // substract parent channel shortcut
            }
            if(position < subchannels.size()) {
                return subchannels.get(position);
            }
            else {
                position -= subchannels.size();
                return currentusers.get(position);
            }
        }

        @Override
        public long getItemId(int arg0) {
            // TODO Auto-generated method stub
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            Object item = getItem(position);

            if(item instanceof Channel) {

                final Channel channel = (Channel) item;

                if(curchannel != null && position == 0) {
                    assert (curchannel.nParentID == ((Channel) getItem(position)).nChannelID);
                    // show parent channel shortcut
                    convertView = inflater.inflate(R.layout.channel_back_item, null);
                    TextView name = (TextView) convertView.findViewById(R.id.parentname);
                    name.setText("..");
                    return convertView;
                }
                else {
                    assert (channel.nChannelID > 0);

                    convertView = inflater.inflate(R.layout.channel_item, null);

                    TextView name = (TextView) convertView.findViewById(R.id.channelname);
                    Button edit = (Button) convertView.findViewById(R.id.edit_btn);
                    Button join = (Button) convertView.findViewById(R.id.join_btn);
                    if(channel.nParentID == 0) {
                        // show server name as channel name for root channel
                        ServerProperties srvprop = new ServerProperties();
                        ttclient.GetServerProperties(srvprop);
                        name.setText(srvprop.szServerName);
                    }
                    else {
                        assert (channel.szName.length() > 0);
                        name.setText(channel.szName);
                    }

                    OnClickListener listener = new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            switch(v.getId()) {
                                case R.id.edit_btn : {
                                    Intent intent = new Intent(MainActivity.this, ChannelPropActivity.class);
                                    startActivityForResult(
                                        intent.putExtra(ChannelPropActivity.EXTRA_CHANNELID, channel.nChannelID),
                                        REQUEST_EDITCHANNEL);
                                    break;
                                }
                                case R.id.join_btn : {
                                    if(channel.bPassword) {
                                        AlertDialog.Builder alert = new AlertDialog.Builder(MainActivity.this);
                                        alert.setTitle("Join Channel");
                                        alert.setMessage("Enter channel password");
                                        final EditText input = new EditText(MainActivity.this);
                                        alert.setView(input);
                                        alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                            public void onClick(DialogInterface dialog, int whichButton) {
                                                String passwd = input.getText().toString();
                                                int cmdid = ttclient.DoJoinChannelByID(channel.nChannelID, passwd);
                                                if(cmdid>0) {
                                                    activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
                                                }
                                            }
                                        });
                                        alert.show();
                                    }
                                    else {
                                        int cmdid = ttclient.DoJoinChannelByID(channel.nChannelID, "");
                                        if(cmdid>0) {
                                            activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    };
                    edit.setOnClickListener(listener);
                    join.setOnClickListener(listener);
                }
            }
            else if(item instanceof User) {
                convertView = inflater.inflate(R.layout.user_item, null);
                TextView nickname = (TextView) convertView.findViewById(R.id.nickname);
                User user = (User) item;
                nickname.setText(user.szNickname);
                
                if((user.uUserState & UserState.USERSTATE_VOICE) != 0)
                    convertView.setBackgroundColor(Color.rgb(133, 229, 141));
            }
            return convertView;
        }
    }

    @Override
    public void onItemClick(AdapterView< ? > l, View v, int position, long id) {

        Object item = adapter.getItem(position);
        if(item instanceof User) {
            User user = (User)item;
            Intent intent = new Intent(this, UserPropActivity.class);
            // TODO: check 'curchannel' for null
            startActivityForResult(intent.putExtra(UserPropActivity.EXTRA_USERID, user.nUserID),
                                   REQUEST_EDITUSER);
        }
        else if(item instanceof Channel) {
            Channel channel = (Channel) item;
            if(channel.nChannelID > 0)
                curchannel = channel;
            else
                curchannel = null;

            adapter.notifyDataSetChanged();
        }
        else {
            item = item;
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        int mychannel = ttclient.GetMyChannelID();
        if(curchannel == null && mychannel > 0) {
            curchannel = ttservice.getChannels().get(mychannel);
        }

        adapter.notifyDataSetChanged();

        ttservice.registerConnectionListener(MainActivity.this);
        ttservice.registerCommandListener(MainActivity.this);
        ttservice.registerUserListener(MainActivity.this);
        
        if(((ttclient.GetFlags() & ClientFlag.CLIENT_SNDINPUT_READY) == 0) &&
            !ttclient.InitSoundInputDevice(0))
            Toast.makeText(this, "Failed to initialize sound input device",
                Toast.LENGTH_LONG).show();

        if(((ttclient.GetFlags() & ClientFlag.CLIENT_SNDOUTPUT_READY) == 0) &&
            !ttclient.InitSoundOutputDevice(0))
            Toast.makeText(this, "Failed to initialize sound output device",
                Toast.LENGTH_LONG).show();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        ttservice.unregisterConnectionListener(MainActivity.this);
        ttservice.unregisterCommandListener(MainActivity.this);
        ttservice = null;
    }

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        Toast.makeText(this, errmsg.szErrorMsg, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onCmdSuccess(int cmdId) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfLoggedOut() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdMyselfKickedFromChannel(User kicker) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdUserUpdate(User user) {
        if(curchannel != null && curchannel.nChannelID == user.nChannelID)
            adapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        if(curchannel != null && curchannel.nChannelID == user.nChannelID)
            adapter.notifyDataSetChanged();
        else if(user.nUserID == ttclient.GetMyUserID()) {
            curchannel = ttservice.getChannels().get(user.nChannelID);
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        if(curchannel != null && curchannel.nChannelID == channelid)
            adapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {
        // TODO Auto-generated method stub
        chatFragment.addTextMessage(textmessage);
    }

    @Override
    public void onCmdChannelNew(Channel channel) {
        if(curchannel != null && curchannel.nChannelID == channel.nParentID)
            adapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
        if(curchannel != null && curchannel.nChannelID == channel.nParentID)
            adapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
        if(curchannel != null && curchannel.nChannelID == channel.nParentID)
            adapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onConnectSuccess() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onConnectFailed() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onConnectionLost() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onMaxPayloadUpdate(int payload_size) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onUserStateChange(User user) {
        if(curchannel != null && user.nChannelID == curchannel.nChannelID)
            adapter.notifyDataSetChanged();
    }

    @Override
    public void onUserVideoCapture(int nUserID, int nStreamID) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onUserMediaFileVideo(int nUserID, int nStreamID) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onUserDesktopWindow(int nUserID, int nStreamID) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
        // TODO Auto-generated method stub
        
    }
}

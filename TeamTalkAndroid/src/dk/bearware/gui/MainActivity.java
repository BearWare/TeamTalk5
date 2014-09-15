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
import dk.bearware.data.TextMessageAdapter;
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
import android.util.Log;
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

    ChannelListAdapter channelsAdapter;
    TextMessageAdapter textmsgAdapter;

    public ChannelListAdapter getChannelsAdapter() {
        return channelsAdapter;
    }
    
    public TextMessageAdapter getTextMessagesAdapter() {
        return textmsgAdapter;
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

        channelsAdapter = new ChannelListAdapter(this.getBaseContext());
        textmsgAdapter = new TextMessageAdapter(this.getBaseContext());
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
                ttclient.enableVoiceTransmission(b);
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
        if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
            Log.e(tag, "Failed to bind to TeamTalk service");
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
        if(isFinishing())
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
            final EditText newmsg = (EditText) rootView.findViewById(R.id.channel_im_edittext);
            ListView chatlog = (ListView)rootView.findViewById(R.id.channel_im_listview);
            chatlog.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
            chatlog.setAdapter(mainActivity.getTextMessagesAdapter());

            Button sendBtn = (Button) rootView.findViewById(R.id.channel_im_sendbtn);
            sendBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View arg0) {
                    TextMessage textmsg = new TextMessage();
                    textmsg.nMsgType = TextMsgType.MSGTYPE_CHANNEL;
                    textmsg.nChannelID = mainActivity.ttclient.getMyChannelID();
                    textmsg.szMessage = newmsg.getText().toString();
                    int cmdid = mainActivity.ttclient.doTextMessage(textmsg);
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
                chanid = ttclient.getRootChannelID();
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
                        ttclient.getServerProperties(srvprop);
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
                                                int cmdid = ttclient.doJoinChannelByID(channel.nChannelID, passwd);
                                                if(cmdid>0) {
                                                    activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
                                                }
                                            }
                                        });
                                        alert.show();
                                    }
                                    else {
                                        int cmdid = ttclient.doJoinChannelByID(channel.nChannelID, "");
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
                final User user = (User) item;
                nickname.setText(user.szNickname);
                
                if((user.uUserState & UserState.USERSTATE_VOICE) != 0)
                    convertView.setBackgroundColor(Color.rgb(133, 229, 141));
                
                Button sndmsg = (Button) convertView.findViewById(R.id.msg_btn);
                OnClickListener listener = new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        switch(v.getId()) {
                            case R.id.msg_btn : {
                                Intent intent = new Intent(MainActivity.this, TextMessageActivity.class);
                                startActivity(intent.putExtra(TextMessageActivity.EXTRA_USERID, user.nUserID));
                                break;
                            }
                        }
                    }
                };
                sndmsg.setOnClickListener(listener);
            }
            return convertView;
        }
    }

    @Override
    public void onItemClick(AdapterView< ? > l, View v, int position, long id) {

        Object item = channelsAdapter.getItem(position);
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

            channelsAdapter.notifyDataSetChanged();
        }
        else {
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        
        ttservice = service;
        ttclient = ttservice.getTTInstance();

        int mychannel = ttclient.getMyChannelID();
        if(curchannel == null && mychannel > 0) {
            curchannel = ttservice.getChannels().get(mychannel);
        }

        channelsAdapter.notifyDataSetChanged();
        
        textmsgAdapter.setTextMessages(ttservice.getChannelTextMsgs(mychannel));
        textmsgAdapter.notifyDataSetChanged();

        ttservice.registerConnectionListener(MainActivity.this);
        ttservice.registerCommandListener(MainActivity.this);
        ttservice.registerUserListener(MainActivity.this);
        
        if(((ttclient.getFlags() & ClientFlag.CLIENT_SNDINPUT_READY) == 0) &&
            !ttclient.initSoundInputDevice(0))
            Toast.makeText(this, "Failed to initialize sound input device",
                Toast.LENGTH_LONG).show();

        if(((ttclient.getFlags() & ClientFlag.CLIENT_SNDOUTPUT_READY) == 0) &&
            !ttclient.initSoundOutputDevice(0))
            Toast.makeText(this, "Failed to initialize sound output device",
                Toast.LENGTH_LONG).show();
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        ttservice.unregisterConnectionListener(MainActivity.this);
        ttservice.unregisterCommandListener(MainActivity.this);
        ttservice.unregisterUserListener(MainActivity.this);
        ttservice = null;
    }

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        Toast.makeText(this, errmsg.szErrorMsg, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onCmdSuccess(int cmdId) {
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
    }

    @Override
    public void onCmdMyselfLoggedOut() {
    }

    @Override
    public void onCmdMyselfKickedFromChannel() {
    }

    @Override
    public void onCmdMyselfKickedFromChannel(User kicker) {
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
    }

    @Override
    public void onCmdUserUpdate(User user) {
        if(curchannel != null && curchannel.nChannelID == user.nChannelID)
            channelsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        if(curchannel != null && curchannel.nChannelID == user.nChannelID)
            channelsAdapter.notifyDataSetChanged();
        else if(user.nUserID == ttclient.getMyUserID()) {
            curchannel = ttservice.getChannels().get(user.nChannelID);
            channelsAdapter.notifyDataSetChanged();
            
            //switch data source for text message to new channel
            if(textmsgAdapter != null) {
                textmsgAdapter.setTextMessages(ttservice.getChannelTextMsgs(user.nChannelID));
                textmsgAdapter.notifyDataSetChanged();
            }
        }
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        if(curchannel != null && curchannel.nChannelID == channelid)
            channelsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {
        
        if(textmessage.nMsgType != TextMsgType.MSGTYPE_CHANNEL)
            return;
        
        if(textmsgAdapter != null)
            textmsgAdapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdChannelNew(Channel channel) {
        if(curchannel != null && curchannel.nChannelID == channel.nParentID)
            channelsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
        if(curchannel != null && curchannel.nChannelID == channel.nParentID)
            channelsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
        if(curchannel != null && curchannel.nChannelID == channel.nParentID)
            channelsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {

    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
    }

    @Override
    public void onConnectSuccess() {
    }

    @Override
    public void onConnectFailed() {
    }

    @Override
    public void onConnectionLost() {
    }

    @Override
    public void onMaxPayloadUpdate(int payload_size) {
    }

    @Override
    public void onUserStateChange(User user) {
        if(curchannel != null && user.nChannelID == curchannel.nChannelID)
            channelsAdapter.notifyDataSetChanged();
    }

    @Override
    public void onUserVideoCapture(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserMediaFileVideo(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopWindow(int nUserID, int nStreamID) {
    }

    @Override
    public void onUserDesktopCursor(int nUserID, DesktopInput desktopinput) {
    }

    @Override
    public void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo) {
    }
}

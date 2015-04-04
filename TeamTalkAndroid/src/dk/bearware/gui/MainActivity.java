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

import java.util.Collections;
import java.util.Comparator;
import java.util.Locale;
import java.util.Vector;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientFlag;
import dk.bearware.ClientStatistics;
import dk.bearware.DesktopInput;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.StreamType;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.TextMsgType;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserState;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.UserListener;

import dk.bearware.backend.OnVoiceTransmissionToggleListener;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.DesktopAdapter;
import dk.bearware.data.FileListAdapter;
import dk.bearware.data.MyTextMessage;
import dk.bearware.data.ServerEntry;
import dk.bearware.data.TextMessageAdapter;
import dk.bearware.data.TTSWrapper;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.ListFragment;
import android.support.v4.view.ViewPager;
import android.text.method.SingleLineTransformationMethod;
import android.util.Log;
import android.util.SparseArray;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ExpandableListView;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity
extends FragmentActivity
implements TeamTalkConnectionListener, 
        ConnectionListener, 
        CommandListener, 
        UserListener, 
        OnItemClickListener, 
        OnItemLongClickListener, 
        OnMenuItemClickListener, 
        OnVoiceTransmissionToggleListener {

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
    
    public static final String TAG = "bearware";

    public final int REQUEST_EDITCHANNEL = 1,
                     REQUEST_NEWCHANNEL = 2,
                     REQUEST_EDITUSER = 3,
                     REQUEST_SELECT_FILE = 4;

    Channel curchannel;

    SparseArray<CmdComplete> activecmds = new SparseArray<CmdComplete>();

    ChannelListAdapter channelsAdapter;
    FileListAdapter filesAdapter;
    TextMessageAdapter textmsgAdapter;
    DesktopAdapter desktopAdapter;
    TTSWrapper ttsWrapper = null;
    AccessibilityAssistant accessibilityAssistant;
    AudioManager audioManager;
    SoundPool audioIcons;
    ComponentName mediaButtonEventReceiver;
    NotificationManager notificationManager;

    static final String MESSAGE_NOTIFICATION_TAG = "incoming_message";

    final int SOUND_VOICETX = 1,
              SOUND_USERMSG = 2,
              SOUND_CHANMSG = 3,
              SOUND_BCASTMSG = 4,
              SOUND_SERVERLOST = 5,
              SOUND_FILESUPDATE = 6;
    
    SparseIntArray sounds = new SparseIntArray();

    public ChannelListAdapter getChannelsAdapter() {
        return channelsAdapter;
    }
    
    public FileListAdapter getFilesAdapter() {
        return filesAdapter;
    }

    public TextMessageAdapter getTextMessagesAdapter() {
        return textmsgAdapter;
    }
    
    public DesktopAdapter getDesktopAdapter() {
        return desktopAdapter;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        String serverName = getIntent().getStringExtra(ServerEntry.KEY_SERVERNAME);
        if ((serverName != null) && !serverName.isEmpty())
            setTitle(serverName);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        accessibilityAssistant = new AccessibilityAssistant(this);
        audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mediaButtonEventReceiver = new ComponentName(getPackageName(), MediaButtonEventReceiver.class.getName());
        notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        channelsAdapter = new ChannelListAdapter(this.getBaseContext());
        filesAdapter = new FileListAdapter(this, accessibilityAssistant);
        textmsgAdapter = new TextMessageAdapter(this.getBaseContext(), accessibilityAssistant);
        desktopAdapter = new DesktopAdapter(this.getBaseContext());
        
        // Create the adapter that will return a fragment for each of the five
        // primary sections of the app.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);
        mViewPager.setOnPageChangeListener(mSectionsPagerAdapter);
        
        final Button tx_btn = (Button) findViewById(R.id.transmit_voice);
        tx_btn.setOnTouchListener(new OnTouchListener() {
            
            boolean tx_state = false;
            long tx_down_start = 0;
            
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                boolean tx = event.getAction() != MotionEvent.ACTION_UP;
                
                if(tx != tx_state) {

                    if(!tx) {
                        if(System.currentTimeMillis() - tx_down_start < 800) {
                            tx = true;
                            tx_down_start = 0;
                        }
                        else {
                            tx_down_start = System.currentTimeMillis();
                        }
                        
                        //Log.i(TAG, "TX is now: " + tx + " diff " + (System.currentTimeMillis() - tx_down_start));
                    }

                    ttservice.enableVoiceTransmission(tx);
                    boolean ptt_vibrate = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("vibrate_checkbox", true);
                    if (ptt_vibrate) {
                        Vibrator vibrat = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
                        vibrat.vibrate(50);
                    }
                }
                tx_state = tx;
                return true;
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        boolean isEditable = curchannel != null;
        boolean isJoinable = (ttclient != null) && (curchannel != null) && (ttclient.getMyChannelID() != curchannel.nChannelID);
        boolean isMyChannel = (ttclient != null) && (curchannel != null) && (ttclient.getMyChannelID() == curchannel.nChannelID);
        menu.findItem(R.id.action_edit).setEnabled(isEditable).setVisible(isEditable);
        menu.findItem(R.id.action_join).setEnabled(isJoinable).setVisible(isJoinable);
        menu.findItem(R.id.action_upload).setEnabled(isMyChannel).setVisible(isMyChannel);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_join : {
                if (curchannel != null)
                    joinChannel(curchannel);
            }
            break;
            case R.id.action_upload : {
                startActivityForResult(new Intent(this, FilePickerActivity.class), REQUEST_SELECT_FILE);
            }
            break;
            case R.id.action_edit : {
                if (curchannel != null)
                    editChannelProperties(curchannel);
            }
            break;

            case R.id.action_newchannel : {
                Intent intent = new Intent(MainActivity.this, ChannelPropActivity.class);
                
                int parent_chan_id = ttclient.getRootChannelID();
                if(curchannel != null)
                    parent_chan_id = curchannel.nChannelID;
                intent = intent.putExtra(ChannelPropActivity.EXTRA_PARENTID, parent_chan_id);
                
                startActivityForResult(intent, REQUEST_NEWCHANNEL);
            }
            break;
            case R.id.action_settings : {
                Intent intent = new Intent(MainActivity.this, PreferencesActivity.class);
                startActivity(intent);
                break;
            }
            case android.R.id.home : {
                if (filesAdapter.getActiveTransfersCount() > 0) {
                    AlertDialog.Builder alert = new AlertDialog.Builder(this);
                    alert.setMessage(R.string.disconnect_alert);
                    alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                filesAdapter.cancelAllTransfers();
                                finish();
                            }
                        });
                    alert.setNegativeButton(android.R.string.cancel, null);
                    alert.show();
                }
                else {
                    finish();
                }
                break;
            }
            default :
                return super.onOptionsItemSelected(item);
        }
        return true;
    }

    CountDownTimer stats_timer = null;
    
    TeamTalkConnection mConnection;
    TeamTalkService ttservice;
    TeamTalkBase ttclient;

    @Override
    protected void onStart() {
        super.onStart();

        if (ttsWrapper == null)
            ttsWrapper = TTSWrapper.getInstance(this);
        
        // Bind to LocalService
        Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
        mConnection = new TeamTalkConnection(this);
        Log.d(TAG, "Binding TeamTalk service");
        if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
            Log.e(TAG, "Failed to bind to TeamTalk service");
        else
            mConnection.setBound(true);
    }

    @Override
    public void onResume() {
        super.onResume();

        if (audioIcons != null)
            audioIcons.release();
        sounds.clear();

        audioIcons = new SoundPool(1, AudioManager.STREAM_MUSIC, 0);

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        
        if (prefs.getBoolean("server_lost_audio_icon", true)) {
            sounds.put(SOUND_SERVERLOST, audioIcons.load(getApplicationContext(), R.raw.serverlost, 1));
        }
        if (prefs.getBoolean("rx_tx_audio_icon", true)) {
            sounds.put(SOUND_VOICETX, audioIcons.load(getApplicationContext(), R.raw.hotkey, 1));
        }
        if (prefs.getBoolean("personal_message_audio_icon", true)) {
            sounds.put(SOUND_USERMSG, audioIcons.load(getApplicationContext(), R.raw.user_message, 1));
        }
        if (prefs.getBoolean("channel_message_audio_icon", true)) {
            sounds.put(SOUND_CHANMSG, audioIcons.load(getApplicationContext(), R.raw.channel_message, 1));
        }
        if (prefs.getBoolean("broadcast_message_audio_icon", true)) {
            sounds.put(SOUND_BCASTMSG, audioIcons.load(getApplicationContext(), R.raw.channel_message, 1));
        }
        if (prefs.getBoolean("files_updated_audio_icon", true)) {
            sounds.put(SOUND_FILESUPDATE, audioIcons.load(getApplicationContext(), R.raw.fileupdate, 1));
        }

        getTextMessagesAdapter().showLogMessages(prefs.getBoolean("show_log_messages", true));
        
        getWindow().getDecorView().setKeepScreenOn(prefs.getBoolean("keep_screen_on_checkbox", false));

        createStatusTimer();
    }

    @Override
    protected void onStop() {
        super.onStop();
        
        if(stats_timer != null) {
            stats_timer.cancel();
            stats_timer = null;
        }
        
        // Cleanup resources
        if(isFinishing()) {
            if (audioIcons != null) {
                audioIcons.release();
                audioIcons = null;
            }
            if (ttsWrapper != null) {
                ttsWrapper.shutdown();
                ttsWrapper = null;
            }
            notificationManager.cancelAll();
        }
        
        if(ttservice != null) {
            ttservice.setOnVoiceTransmissionToggleListener(null);
            ttservice.unregisterConnectionListener(this);
            ttservice.unregisterCommandListener(this);
            ttservice.unregisterUserListener(this);
            filesAdapter.setTeamTalkService(null);
        }
        
        // Unbind from the service
        if(mConnection.isBound()) {
            Log.d(TAG, "Unbinding TeamTalk service");
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "Activity destroyed " + this.hashCode());
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if ((requestCode == REQUEST_SELECT_FILE) && (resultCode == RESULT_OK)) {
            String path = data.getStringExtra(FilePickerActivity.SELECTED_FILE);
            String remoteName = filesAdapter.getRemoteName(path);
            if (remoteName != null) {
                Toast.makeText(this, getString(R.string.remote_file_exists, remoteName), Toast.LENGTH_LONG).show();
            } else if (ttclient.doSendFile(curchannel.nChannelID, path) <= 0) {
                Toast.makeText(this, getString(R.string.upload_failed, path), Toast.LENGTH_LONG).show();
            }
            else {
                Toast.makeText(this, R.string.upload_started, Toast.LENGTH_SHORT).show();
            }
        }
    }

    ChannelsSectionFragment channelsFragment;
    ChatSectionFragment chatFragment;
    VidcapSectionFragment vidcapFragment;
    DesktopSectionFragment desktopFragment;
    FilesSectionFragment filesFragment;

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter implements ViewPager.OnPageChangeListener {
        
        public static final int CHANNELS_PAGE   = 0,
                                CHAT_PAGE       = 1,
//                                VIDCAP_PAGE   = 2,
                                DESKTOP_PAGE    = 2,
                                FILES_PAGE      = 3,
                                
                                PAGE_COUNT      = 4;

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
//                case VIDCAP_PAGE : {
//                    vidcapFragment = new VidcapSectionFragment();
//                    return vidcapFragment;
//                }
                case DESKTOP_PAGE : {
                    desktopFragment = new DesktopSectionFragment();
                    return desktopFragment;
                }
                case FILES_PAGE : {
                    filesFragment = new FilesSectionFragment();
                    return filesFragment;
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
                    return getString(R.string.title_section_channels).toUpperCase(l);
                case CHAT_PAGE :
                    return getString(R.string.title_section_chat).toUpperCase(l);
//                case VIDCAP_PAGE :
//                    return getString(R.string.title_section_video).toUpperCase(l);
                case DESKTOP_PAGE :
                    return getString(R.string.title_section_desktop).toUpperCase(l);
                case FILES_PAGE :
                    return getString(R.string.title_section_files).toUpperCase(l);
            }
            return null;
        }

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
            accessibilityAssistant.setVisiblePage(position);
        }

        @Override
        public void onPageScrollStateChanged(int state) {
        }
    }

    private void editChannelProperties(Channel channel) {
        Intent intent = new Intent(this, ChannelPropActivity.class);
        startActivityForResult(intent.putExtra(ChannelPropActivity.EXTRA_CHANNELID, channel.nChannelID), REQUEST_EDITCHANNEL);
    }

    private void joinChannelUnsafe(Channel channel, String passwd) {
        int cmdid = ttclient.doJoinChannelByID(channel.nChannelID, passwd);
        if(cmdid>0) {
            activecmds.put(cmdid, CmdComplete.CMD_COMPLETE_JOIN);
            channel.szPassword = passwd;
            ttservice.setJoinChannel(channel);
        }
        else {
            Toast.makeText(this, R.string.text_con_cmderr, Toast.LENGTH_LONG).show();
        }
    }

    private void joinChannel(final Channel channel, final String passwd) {
        if (filesAdapter.getActiveTransfersCount() > 0) {
            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setMessage(R.string.channel_change_alert);
            alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        filesAdapter.cancelAllTransfers();
                        joinChannelUnsafe(channel, passwd);
                    }
                });
            alert.setNegativeButton(android.R.string.cancel, null);
            alert.show();
        }

        else {
            joinChannelUnsafe(channel, passwd);
        }
    }

    private void joinChannel(final Channel channel) {
        if(channel.bPassword) {
            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setTitle(R.string.pref_title_join_channel);
            alert.setMessage(R.string.channel_password_prompt);
            final EditText input = new EditText(this);
            input.setTransformationMethod(SingleLineTransformationMethod.getInstance());
            input.setText(channel.szPassword);
            alert.setView(input);
            alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        joinChannel(channel, input.getText().toString());
                    }
                });
            alert.setNegativeButton(android.R.string.cancel, null);
            alert.show();
        }
        else {
            joinChannel(channel, "");
        }
    }

    private void setCurrentChannel(Channel channel) {
        curchannel = channel;
        getActionBar().setSubtitle((channel != null) ? channel.szName : null);
        invalidateOptionsMenu();
    }

    private boolean isVisibleChannel(int chanid) {
        if (curchannel != null) {
            if (curchannel.nParentID == chanid)
                return true;
            Channel channel = ttservice.getChannels().get(chanid);
            if (channel != null)
                return curchannel.nChannelID == channel.nParentID;
        }
        else {
            return chanid == ttclient.getRootChannelID();
        }
        return false;
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
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.CHANNELS_PAGE);

            ListView channelsList = (ListView) rootView.findViewById(R.id.listChannels);
            channelsList.setAdapter(mainActivity.getChannelsAdapter());
            channelsList.setOnItemClickListener(mainActivity);
            channelsList.setOnItemLongClickListener(mainActivity);

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
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.CHAT_PAGE);
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
                    else {
                        Toast.makeText(mainActivity, getResources().getString(R.string.text_con_cmderr),
                            Toast.LENGTH_LONG).show();
                    }
                }
            });
            return rootView;
        }
    }

    public static class VidcapSectionFragment extends Fragment {
        MainActivity mainActivity;

        public VidcapSectionFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_vidcap, container, false);
//            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.VIDCAP_PAGE);

            return rootView;
        }
    }

    public static class DesktopSectionFragment extends Fragment {
        MainActivity mainActivity;

        public DesktopSectionFragment() {
        }

        @Override
        public void onAttach(Activity activity) {
            mainActivity = (MainActivity) activity;
            super.onAttach(activity);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_desktop, container, false);
            mainActivity.accessibilityAssistant.registerPage(rootView, SectionsPagerAdapter.DESKTOP_PAGE);

            ExpandableListView exview = (ExpandableListView) rootView.findViewById(R.id.desktop_elist_view);
            exview.setAdapter(mainActivity.getDesktopAdapter());
            return rootView;
        }
    }

    public static class FilesSectionFragment extends ListFragment {

        @Override
        public void onViewCreated(View view, Bundle savedInstanceState) {
            MainActivity mainActivity = (MainActivity) getActivity();
            mainActivity.accessibilityAssistant.registerPage(view, SectionsPagerAdapter.FILES_PAGE);
            setListAdapter(mainActivity.getFilesAdapter());
            super.onViewCreated(view, savedInstanceState);
        }
    }

    class ChannelListAdapter extends BaseAdapter {

        private static final int PARENT_CHANNEL_VIEW_TYPE = 0,
            CHANNEL_VIEW_TYPE = 1,
            USER_VIEW_TYPE = 2,

            VIEW_TYPE_COUNT = 3;

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

            Collections.sort(subchannels, new Comparator<Channel>() {
                    @Override
                    public int compare(Channel c1, Channel c2) {
                        return c1.szName.compareToIgnoreCase(c2.szName);
                    }
                });

            Collections.sort(currentusers, new Comparator<User>() {
                    @Override
                    public int compare(User u1, User u2) {
                        if (((u1.uUserState & UserState.USERSTATE_VOICE) != 0) &&
                            ((u2.uUserState & UserState.USERSTATE_VOICE) == 0))
                            return -1;
                        else if (((u1.uUserState & UserState.USERSTATE_VOICE) == 0) &&
                                 ((u2.uUserState & UserState.USERSTATE_VOICE) != 0))
                            return 1;
                        return u1.szNickname.compareToIgnoreCase(u2.szNickname);
                    }
                });

            super.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            int count = subchannels.size() + currentusers.size();
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                count++; // include parent channel shortcut
            }
            return count;
        }

        @Override
        public Object getItem(int position) {
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
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
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getItemViewType(int position) {
            if ((curchannel != null) && (curchannel.nParentID > 0)) {
                if (position == 0) {
                    return PARENT_CHANNEL_VIEW_TYPE;
                }
                else if (position <= subchannels.size()) {
                    return CHANNEL_VIEW_TYPE;
                }
                return USER_VIEW_TYPE;
            }
            else if (position < subchannels.size()) {
                return CHANNEL_VIEW_TYPE;
            }
            return USER_VIEW_TYPE;
        }

        @Override
        public int getViewTypeCount() {
            return VIEW_TYPE_COUNT;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            Object item = getItem(position);

            if(item instanceof Channel) {

                final Channel channel = (Channel) item;

                if ((curchannel != null) && (curchannel.nParentID > 0) && (position == 0)) {
                    assert (curchannel.nParentID == ((Channel) getItem(position)).nChannelID);
                    // show parent channel shortcut
                    if (convertView == null ||
                        convertView.findViewById(R.id.parentname) == null)
                        convertView = inflater.inflate(R.layout.item_channel_back, null);
                }
                else {
                    assert (channel.nChannelID > 0);

                    if (convertView == null ||
                        convertView.findViewById(R.id.channelname) == null)
                        convertView = inflater.inflate(R.layout.item_channel, null);

                    TextView name = (TextView) convertView.findViewById(R.id.channelname);
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
                                case R.id.join_btn : {
                                    joinChannel(channel);
                                }
                                break;
                            }
                        }
                    };
                    join.setOnClickListener(listener);
                    join.setAccessibilityDelegate(accessibilityAssistant);
                    join.setEnabled(channel.nChannelID != ttclient.getMyChannelID());
                }
                int population = Utils.getUsers(channel.nChannelID, ttservice.getUsers()).size();
                ((TextView)convertView.findViewById(R.id.population)).setText((population > 0) ? String.format("(%d)", population) : "");
            }
            else if(item instanceof User) {
                if (convertView == null ||
                    convertView.findViewById(R.id.nickname) == null)
                    convertView = inflater.inflate(R.layout.item_user, null);
                TextView nickname = (TextView) convertView.findViewById(R.id.nickname);
                final User user = (User) item;
                nickname.setText(user.szNickname);
                
                if((user.uUserState & UserState.USERSTATE_VOICE) != 0) {
                    convertView.setBackgroundColor(Color.rgb(133, 229, 141));
                    nickname.setContentDescription(getString(R.string.user_state_now_speaking, user.szNickname));
                }
                else {
                    convertView.setBackgroundColor(Color.rgb(0, 0, 0));
                    nickname.setContentDescription(null);
                }
                
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
                sndmsg.setAccessibilityDelegate(accessibilityAssistant);
            }
            convertView.setAccessibilityDelegate(accessibilityAssistant);
            return convertView;
        }
    }
    
    void createStatusTimer() {
        
        final TextView connection = (TextView) findViewById(R.id.connectionstat_textview);
        final TextView ping = (TextView) findViewById(R.id.pingstat_textview);
        final TextView total = (TextView) findViewById(R.id.totalstat_textview);
        final TextView voice = (TextView) findViewById(R.id.voicestat_textview);
        final TextView desktop = (TextView) findViewById(R.id.desktopstat_textview);
        final TextView mediafile = (TextView) findViewById(R.id.mediafilestat_textview);
        final int defcolor = connection.getTextColors().getDefaultColor();
        
        if(stats_timer == null) {
            stats_timer = new CountDownTimer(10000, 1000) {
                
                ClientStatistics prev_stats;
                
                public void onTick(long millisUntilFinished) {
                
                    if (ttclient == null || accessibilityAssistant.isUiUpdateDiscouraged())
                        return;
                    filesAdapter.performPendingUpdate();

                    int con = R.string.stat_offline;
                    int con_color = Color.RED;
                    int flags = ttclient.getFlags(); 
                    if((flags & ClientFlag.CLIENT_CONNECTED) == ClientFlag.CLIENT_CONNECTED) {
                        con = R.string.stat_online;
                        con_color = Color.GREEN;
                    }
                    else if((flags & ClientFlag.CLIENT_CONNECTING) == ClientFlag.CLIENT_CONNECTING) {
                        con = R.string.stat_connecting;
                    }
                    
                    connection.setText(con);
                    connection.setTextColor(con_color);

                    ClientStatistics stats = new ClientStatistics();
                    if(!ttclient.getClientStatistics(stats))
                        return;
                    
                    if(prev_stats == null)
                        prev_stats = stats;
                    
                    long totalrx = stats.nUdpBytesRecv - prev_stats.nUdpBytesRecv;
                    long totaltx = stats.nUdpBytesSent - prev_stats.nUdpBytesSent;
                    long voicerx = stats.nVoiceBytesRecv - prev_stats.nVoiceBytesRecv;
                    long voicetx = stats.nVoiceBytesSent - prev_stats.nVoiceBytesSent;
                    long deskrx = stats.nDesktopBytesRecv - prev_stats.nDesktopBytesRecv;
                    long desktx = stats.nDesktopBytesSent - prev_stats.nDesktopBytesSent;
                    long mfrx = (stats.nMediaFileAudioBytesRecv + stats.nMediaFileVideoBytesRecv) - 
                                (prev_stats.nMediaFileAudioBytesRecv + prev_stats.nMediaFileVideoBytesRecv);
                    long mftx = (stats.nMediaFileAudioBytesSent + stats.nMediaFileVideoBytesSent) - 
                                (prev_stats.nMediaFileAudioBytesSent + prev_stats.nMediaFileVideoBytesSent);

                    String str;
                    if(stats.nUdpPingTimeMs >= 0) {
                        str = String.format("%1$d", stats.nUdpPingTimeMs); 
                        ping.setText(str);
                        
                        if(stats.nUdpPingTimeMs > 250) {
                            ping.setTextColor(Color.RED);
                        }
                        else {
                            ping.setTextColor(defcolor);
                        }
                    }                    
                    
                    str = String.format("%1$d/%2$d KB", totalrx/ 1024, totaltx / 1024);
                    total.setText(str);
                    str = String.format("%1$d/%2$d KB", voicerx/ 1024, voicetx / 1024);
                    voice.setText(str);
                    str = String.format("%1$d/%2$d KB", deskrx/ 1024, desktx / 1024);
                    desktop.setText(str);
                    str = String.format("%1$d/%2$d KB", mfrx/ 1024, mftx / 1024);
                    mediafile.setText(str);
                    
                    prev_stats = stats;
                }

                public void onFinish() {
                    start();
                }
            }.start();
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
            setCurrentChannel((channel.nChannelID > 0) ? channel : null);
            channelsAdapter.notifyDataSetChanged();
        }
        else {
        }
    }

    Channel selectedChannel;

    @Override
    public boolean onItemLongClick(AdapterView< ? > l, View v, int position, long id) {
        Object item = channelsAdapter.getItem(position);
        if (item instanceof Channel) {
            selectedChannel = (Channel) item;
            if ((curchannel != null) && (curchannel.nParentID != selectedChannel.nChannelID)) {
                boolean isRemovable = (ttclient != null) && (selectedChannel.nChannelID != ttclient.getMyChannelID());
                PopupMenu channelActions = new PopupMenu(this, v);
                channelActions.setOnMenuItemClickListener(this);
                channelActions.inflate(R.menu.channel_actions);
                channelActions.getMenu().findItem(R.id.action_remove).setEnabled(isRemovable).setVisible(isRemovable);
                channelActions.show();
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.action_edit:
            editChannelProperties(selectedChannel);
            break;
        case R.id.action_remove: {
            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setMessage(getString(R.string.channel_remove_confirmation, selectedChannel.szName));
            alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        if (ttclient.doRemoveChannel(selectedChannel.nChannelID) <= 0)
                            Toast.makeText(MainActivity.this,
                                           getString(R.string.err_channel_remove,
                                                     selectedChannel.szName),
                                           Toast.LENGTH_LONG).show();
                    }
                });

            alert.setNegativeButton(android.R.string.no, null);
            alert.show();
            break;
        }

        default:
            return false;
        }
        return true;
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        
        ttservice = service;
        ttclient = ttservice.getTTInstance();
        
        //ttclient.DBG_SetSoundInputTone(StreamType.STREAMTYPE_VOICE, 440);

        int mychannel = ttclient.getMyChannelID();
        if(curchannel == null && mychannel > 0) {
            setCurrentChannel(ttservice.getChannels().get(mychannel));
        }

        mSectionsPagerAdapter.onPageSelected(mViewPager.getCurrentItem());

        channelsAdapter.notifyDataSetChanged();
        
        textmsgAdapter.setTextMessages(ttservice.getChatLogTextMsgs());
        textmsgAdapter.setMyUserID(ttclient.getMyUserID());
        textmsgAdapter.notifyDataSetChanged();
        
        desktopAdapter.setTeamTalkService(service);
        desktopAdapter.notifyDataSetChanged();
        
        filesAdapter.setTeamTalkService(service);
        filesAdapter.update(mychannel);

        Button tx_btn = (Button) findViewById(R.id.transmit_voice);
        tx_btn.setBackgroundColor(ttservice.isVoiceTransmissionEnabled() ? Color.GREEN : Color.RED);

        ttservice.registerConnectionListener(this);
        ttservice.registerCommandListener(this);
        ttservice.registerUserListener(this);
        ttservice.setOnVoiceTransmissionToggleListener(this);

        if(((ttclient.getFlags() & ClientFlag.CLIENT_SNDOUTPUT_READY) == 0) &&
            !ttclient.initSoundOutputDevice(0))
            Toast.makeText(this, R.string.err_init_sound_output,
                Toast.LENGTH_LONG).show();
        audioManager.registerMediaButtonEventReceiver(mediaButtonEventReceiver);
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
        audioManager.unregisterMediaButtonEventReceiver(mediaButtonEventReceiver);
    }

    @Override
    public void onCmdError(int cmdId, ClientErrorMsg errmsg) {
        Utils.notifyError(this, errmsg);
    }

    @Override
    public void onCmdSuccess(int cmdId) {
    }

    @Override
    public void onCmdProcessing(int cmdId, boolean complete) {
        if(complete) {
            activecmds.remove(cmdId);
        }
    }

    @Override
    public void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount) {
        textmsgAdapter.setMyUserID(my_userid);
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
        if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("server_login_checkbox", false))
            ttsWrapper.speak(user.szNickname + " " + getResources().getString(R.string.text_tts_loggedin));
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("server_logout_checkbox", false))
            ttsWrapper.speak(user.szNickname + " " + getResources().getString(R.string.text_tts_loggedout));
    }

    @Override
    public void onCmdUserUpdate(User user) {
        if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself joined channel
            
            Channel chan = ttservice.getChannels().get(user.nChannelID); 
            setCurrentChannel(chan);
            filesAdapter.update(curchannel);
        }
        else if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            //other user joined current channel
        }
        
        if(curchannel != null && curchannel.nChannelID == user.nChannelID) {
            //event took place in current channel
            
            if(user.nUserID != ttclient.getMyUserID()) {
                accessibilityAssistant.lockEvents();
                textmsgAdapter.notifyDataSetChanged();
                channelsAdapter.notifyDataSetChanged();
                if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("channel_join_checkbox", false))
                    ttsWrapper.speak(user.szNickname + " " + getResources().getString(R.string.text_tts_joined_chan));
                accessibilityAssistant.unlockEvents();
            }
            else {
                textmsgAdapter.notifyDataSetChanged();
                channelsAdapter.notifyDataSetChanged();
            }
        }
        else if (isVisibleChannel(user.nChannelID)) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdUserLeftChannel(int channelid, User user) {
        
        if(user.nUserID == ttclient.getMyUserID()) {
            //myself left current channel
            
            Channel chan = ttservice.getChannels().get(channelid);
            textmsgAdapter.notifyDataSetChanged();
            
            setCurrentChannel(null);
        }
        else if(curchannel != null && channelid == curchannel.nChannelID){
            //other user left current channel
            
            accessibilityAssistant.lockEvents();
            textmsgAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
        
        if(curchannel != null && curchannel.nChannelID == channelid) {
            //event took place in current channel
            
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            if (ttsWrapper != null && PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("channel_leave_checkbox", false))
                ttsWrapper.speak(user.szNickname + " " + getResources().getString(R.string.text_tts_left_chan));
            accessibilityAssistant.unlockEvents();
        }
        else if (isVisibleChannel(channelid)) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdUserTextMessage(TextMessage textmessage) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        switch (textmessage.nMsgType) {
        case TextMsgType.MSGTYPE_CHANNEL :
        case TextMsgType.MSGTYPE_BROADCAST :
            accessibilityAssistant.lockEvents();
            textmsgAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
            
            // audio event
            if (sounds.get(SOUND_CHANMSG) != 0)
                audioIcons.play(sounds.get(SOUND_CHANMSG), 1.0f, 1.0f, 0, 0, 1.0f);
            // TTS event
            if (ttsWrapper != null && prefs.getBoolean("broadcast_message_checkbox", false)) {
                User sender = ttservice.getUsers().get(textmessage.nFromUserID);
                ttsWrapper.speak(getString(R.string.text_tts_broadcast_message, (sender != null) ? sender.szNickname : ""));
            }
            Log.d(TAG, "Channel message in " + this.hashCode());
            break;
        case TextMsgType.MSGTYPE_USER :
            if (sounds.get(SOUND_USERMSG) != 0)
                audioIcons.play(sounds.get(SOUND_USERMSG), 1.0f, 1.0f, 0, 0, 1.0f);
            
            User sender = ttservice.getUsers().get(textmessage.nFromUserID);
            String senderName = (sender != null) ? sender.szNickname : "";
            if (ttsWrapper != null && prefs.getBoolean("personal_message_checkbox", false))
                ttsWrapper.speak(getString(R.string.text_tts_personal_message, senderName));
            Intent action = new Intent(this, TextMessageActivity.class);
            Notification.Builder notification = new Notification.Builder(this);
            notification.setSmallIcon(R.drawable.message)
                .setContentTitle(getString(R.string.personal_message_notification, senderName))
                .setContentText(getString(R.string.personal_message_notification_hint))
                .setContentIntent(PendingIntent.getActivity(this, textmessage.nFromUserID, action.putExtra(TextMessageActivity.EXTRA_USERID, textmessage.nFromUserID), 0))
                .setAutoCancel(true);
            notificationManager.notify(MESSAGE_NOTIFICATION_TAG, textmessage.nFromUserID, notification.build());
            break;
        case TextMsgType.MSGTYPE_CUSTOM:
        default:
            break;
        }
    }

    @Override
    public void onCmdChannelNew(Channel channel) {
        if (curchannel != null && curchannel.nChannelID == channel.nParentID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdChannelUpdate(Channel channel) {
        if (curchannel != null && curchannel.nChannelID == channel.nParentID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdChannelRemove(Channel channel) {
        if (curchannel != null && curchannel.nChannelID == channel.nParentID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }

    @Override
    public void onCmdServerUpdate(ServerProperties serverproperties) {
    }

    @Override
    public void onCmdFileNew(RemoteFile remotefile) {
        filesAdapter.update();
        
        if(activecmds.size() == 0) {
            if(sounds.get(SOUND_FILESUPDATE) != 0) {
                audioIcons.play(sounds.get(SOUND_FILESUPDATE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onCmdFileRemove(RemoteFile remotefile) {
        filesAdapter.update();
        
        if(activecmds.size() == 0) {
            if(sounds.get(SOUND_FILESUPDATE) != 0) {
                audioIcons.play(sounds.get(SOUND_FILESUPDATE), 1.0f, 1.0f, 0, 0, 1.0f);
            }
        }
    }

    @Override
    public void onCmdUserAccount(UserAccount useraccount) {
    }

    @Override
    public void onCmdBannedUser(BannedUser banneduser) {
    }

    @Override
    public void onConnectSuccess() {
    }

    @Override
    public void onConnectFailed() {
    }

    @Override
    public void onConnectionLost() {
        if(sounds.get(SOUND_SERVERLOST) != 0) {
            audioIcons.play(sounds.get(SOUND_SERVERLOST), 1.0f, 1.0f, 0, 0, 1.0f);
        }
    }

    @Override
    public void onMaxPayloadUpdate(int payload_size) {
    }

    @Override
    public void onUserStateChange(User user) {
        
        if (curchannel != null && user.nChannelID == curchannel.nChannelID) {
            accessibilityAssistant.lockEvents();
            channelsAdapter.notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
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

    @Override
    public void onVoiceTransmissionToggle(boolean voiceTransmissionEnabled) {
        Button tx_btn = (Button) findViewById(R.id.transmit_voice);
        tx_btn.setBackgroundColor( voiceTransmissionEnabled ? Color.GREEN : Color.RED);
        if (sounds.get(SOUND_VOICETX) != 0)
            audioIcons.play(sounds.get(SOUND_VOICETX), 1.0f, 1.0f, 0, 0, 1.0f);
    }

}

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
package dk.bearware.gui;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.app.AlertDialog;
import android.os.Bundle;
import android.os.IBinder;
import android.text.InputType;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;

import dk.bearware.ClientEvent;
import dk.bearware.Channel;
import dk.bearware.User;
import dk.bearware.ClientFlag;
import dk.bearware.ClientStatistics;
import dk.bearware.TeamTalkBase;
import dk.bearware.UserAccount;
import dk.bearware.UserRight;
import dk.bearware.UserState;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.ClientEventListener;

public class OnlineUsersActivity extends AppCompatActivity implements
        ClientEventListener.OnCmdUserLoggedInListener,
        ClientEventListener.OnCmdUserLoggedOutListener,
        ClientEventListener.OnCmdUserJoinedChannelListener,
        ClientEventListener.OnCmdUserLeftChannelListener,
        ClientEventListener.OnCmdUserUpdateListener {

    private static final String TAG = "OnlineUsersActivity";

    private TeamTalkService ttservice;
    private TeamTalkBase ttclient;
    private ListView onlineUsersList;
    private OnlineUserAdapter adapter;
    private final ArrayList<User> onlineUsers = new ArrayList<>();
    private boolean isBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_online_users);

        onlineUsersList = findViewById(R.id.online_users_list);
        adapter = new OnlineUserAdapter(this, onlineUsers);
        onlineUsersList.setAdapter(adapter);

        onlineUsersList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                User selectedUser = onlineUsers.get(position);
                Intent intent = new Intent(OnlineUsersActivity.this, UserPropActivity.class);
                intent.putExtra(UserPropActivity.EXTRA_USERID, selectedUser.nUserID);
                startActivity(intent);
            }
        });

        onlineUsersList.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View v, int position, long id) {
                User selectedUser = onlineUsers.get(position);
                AlertDialog.Builder alert = new AlertDialog.Builder(OnlineUsersActivity.this);
                UserAccount myuseraccount = new UserAccount();
                ttclient.getMyUserAccount(myuseraccount);
                boolean banRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_BAN_USERS) != UserRight.USERRIGHT_NONE;
                boolean kickRight = (myuseraccount.uUserRights & UserRight.USERRIGHT_KICK_USERS) != UserRight.USERRIGHT_NONE;
                // operator of a channel can also kick users
                int myuserid = ttclient.getMyUserID();
                boolean operatorRight = ttclient.isChannelOperator(myuserid, selectedUser.nChannelID);

                PopupMenu onlineActions = new PopupMenu(OnlineUsersActivity.this, v);
                onlineActions.inflate(R.menu.online_actions);

                onlineActions.setOnMenuItemClickListener(item -> {
                    switch (item.getItemId()) {
                        case R.id.action_edituser: {
                            Intent intent = new Intent(OnlineUsersActivity.this, UserPropActivity.class);
                            startActivity(intent.putExtra(UserPropActivity.EXTRA_USERID, selectedUser.nUserID));
                        }
                            return true;
                        case R.id.action_message: {
                            Intent intent = new Intent(OnlineUsersActivity.this, TextMessageActivity.class);
                            startActivity(intent.putExtra(TextMessageActivity.EXTRA_USERID, selectedUser.nUserID));
                        }
                            return true;
                        case R.id.action_banchan:
                confirmAction(alert, R.string.ban_confirmation, selectedUser,
                        () -> banAndKick(selectedUser, selectedUser.nChannelID));
                            return true;
                        case R.id.action_bansrv:
                confirmAction(alert, R.string.ban_confirmation, selectedUser,
                        () -> banAndKick(selectedUser, 0));
                            return true;
                        case R.id.action_kickchan:
                confirmAction(alert, R.string.kick_confirmation, selectedUser,
                        () -> ttclient.doKickUser(selectedUser.nUserID, selectedUser.nChannelID));
                            return true;
                        case R.id.action_kicksrv:
                confirmAction(alert, R.string.kick_confirmation, selectedUser,
                        () -> ttclient.doKickUser(selectedUser.nUserID, 0));
                            return true;
                        case R.id.action_makeop:
                boolean isOp = ttclient.isChannelOperator(selectedUser.nUserID, selectedUser.nChannelID);
                            if ((myuseraccount.uUserRights & UserRight.USERRIGHT_OPERATOR_ENABLE) != UserRight.USERRIGHT_NONE) {
                                ttclient.doChannelOp(selectedUser.nUserID, selectedUser.nChannelID, !isOp);
                                return true;
                            }
                            alert.setTitle(!isOp ? R.string.action_revoke_operator : R.string.action_make_operator);
                            alert.setMessage(R.string.text_operator_password);
                            final EditText input = new EditText(OnlineUsersActivity.this);
                            input.setInputType(InputType.TYPE_TEXT_VARIATION_PASSWORD | InputType.TYPE_CLASS_TEXT);
                            alert.setPositiveButton(android.R.string.yes, (dialog, whichButton) -> 
                            ttclient.doChannelOpEx(selectedUser.nUserID, selectedUser.nChannelID, input.getText().toString(), !isOp));
                            alert.setNegativeButton(android.R.string.no, null);
                            alert.setView(input);
                            alert.show();
                            return true;

                        default:
                            return false;
                    }
                });

            onlineActions.getMenu().findItem(R.id.action_kickchan).setEnabled(kickRight | operatorRight).setVisible(kickRight | operatorRight);
            onlineActions.getMenu().findItem(R.id.action_kicksrv).setEnabled(kickRight).setVisible(kickRight);
            onlineActions.getMenu().findItem(R.id.action_banchan).setEnabled(banRight | operatorRight).setVisible(banRight | operatorRight);
            onlineActions.getMenu().findItem(R.id.action_bansrv).setEnabled(banRight).setVisible(banRight);
            onlineActions.getMenu().findItem(R.id.action_makeop).setTitle(ttclient.isChannelOperator(selectedUser.nUserID , selectedUser.nChannelID) ? R.string.action_revoke_operator : R.string.action_make_operator);

                onlineActions.show();
                return true;
            }
        });

        Intent intent = new Intent(this, TeamTalkService.class);
        bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onDestroy() {
        if (isBound) {
            ttservice.getEventHandler().unregisterListener(this);
            unbindService(serviceConnection);
            isBound = false;
        }
        super.onDestroy();
    }

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            TeamTalkService.LocalBinder binder = (TeamTalkService.LocalBinder) service;
            ttservice = binder.getService();
            ttclient = ttservice.getTTInstance();
            isBound = true;
            registerEventListeners();
            populateUserList();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            isBound = false;
        }
    };

    private void confirmAction(AlertDialog.Builder alert, int messageResId, User user, Runnable action) {
        alert.setMessage(getString(messageResId, user.szNickname));
        alert.setPositiveButton(android.R.string.yes, (dialog, which) -> action.run());
        alert.setNegativeButton(android.R.string.no, null);
        alert.show();
    }

    private void banAndKick(User user, int channelId) {
        ttclient.doBanUser(user.nUserID, channelId);
        ttclient.doKickUser(user.nUserID, channelId);
    }

    private void registerEventListeners() {
        if (ttservice != null) {
            ttservice.getEventHandler().registerOnCmdUserLoggedIn(this, true);
            ttservice.getEventHandler().registerOnCmdUserLoggedOut(this, true);
            ttservice.getEventHandler().registerOnCmdUserJoinedChannel(this, true);
            ttservice.getEventHandler().registerOnCmdUserLeftChannel(this, true);
            ttservice.getEventHandler().registerOnCmdUserUpdate(this, true);
        }
    }

    private void populateUserList() {
        if (ttservice != null) {
            onlineUsers.clear();
            onlineUsers.addAll(ttservice.getUsers().values());
            sortAndNotifyDataSetChanged();
        }
    }

    private void sortAndNotifyDataSetChanged() {
        onlineUsers.sort(Comparator.comparing(u -> u.szNickname.toLowerCase()));
        adapter.notifyDataSetChanged();
    }

    private int findUserIndex(User user) {
        for (int i = 0; i < onlineUsers.size(); i++) {
            if (onlineUsers.get(i).nUserID == user.nUserID) {
                return i;
            }
        }
        return -1;
    }

    private void updateUser(User user) {
        int index = findUserIndex(user);
        if (index != -1) {
            onlineUsers.set(index, user);
        } else {
            onlineUsers.add(user);
        }
        sortAndNotifyDataSetChanged();
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        Log.d(TAG, "User logged in: " + user.szNickname);
        updateUser(user);
    }

    @Override
    public void onCmdUserLoggedOut(User user) {
        Log.d(TAG, "User logged out: " + user.szNickname);
        int index = findUserIndex(user);
        if (index != -1) {
            onlineUsers.remove(index);
        }
        sortAndNotifyDataSetChanged();
    }

    @Override
    public void onCmdUserJoinedChannel(User user) {
        Log.d(TAG, "User " + user.szNickname + " joined channel " + user.nChannelID);
        updateUser(user);
    }

    @Override
    public void onCmdUserLeftChannel(int nChannelID, User user) {
        Log.d(TAG, "User " + user.szNickname + " left channel " + nChannelID);
        updateUser(user);
    }

    @Override
    public void onCmdUserUpdate(User user) {
        Log.d(TAG, "User updated: " + user.szNickname);
        updateUser(user);
    }

    private class OnlineUserAdapter extends ArrayAdapter<User> {
        private final LayoutInflater inflater;

        public OnlineUserAdapter(Context context, List<User> users) {
            super(context, android.R.layout.simple_list_item_1, users);
            inflater = LayoutInflater.from(context);
        }

        @NonNull
        @Override
        public View getView(int position, View convertView, @NonNull ViewGroup parent) {
            if (convertView == null) {
                convertView = inflater.inflate(android.R.layout.simple_list_item_1, parent, false);
            }

            TextView textView = (TextView) convertView.findViewById(android.R.id.text1);
            User user = getItem(position);

            if (user != null) {
                StringBuilder userInfo = new StringBuilder();
                userInfo.append(getString(R.string.online_user_info_nickname)).append(": ").append(user.szNickname);

                if (!user.szUsername.isEmpty()) {
                    userInfo.append(", ").append(getString(R.string.online_user_info_username)).append(": ").append(user.szUsername);
                }

                if (ttservice != null && ttservice.getChannels() != null) {
                    Channel userChannel = ttservice.getChannels().get(user.nChannelID);
                    if (userChannel != null) {
                        userInfo.append(", ").append(getString(R.string.online_user_info_channel)).append(": ").append(userChannel.szName);
                    }
                }

                if (!user.szIPAddress.isEmpty()) {
                    userInfo.append(", ").append(getString(R.string.online_user_info_ip)).append(": ").append(user.szIPAddress);
                }

                                if (!user.szClientName.isEmpty()) {
                    userInfo.append(", ").append(getString(R.string.online_user_info_client)).append(": ").append(user.szClientName);
                    String clientVersion = ((user.uVersion >> 16) & 0xFF) + "." + ((user.uVersion >> 8) & 0xFF) + "." + (user.uVersion & 0xFF);
                    userInfo.append(" ").append(clientVersion);
                }

                textView.setText(userInfo.toString());
            }

            return convertView;
        }
    }
}
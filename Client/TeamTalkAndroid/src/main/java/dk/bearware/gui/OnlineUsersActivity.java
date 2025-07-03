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
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;

import dk.bearware.Channel;
import dk.bearware.User;
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
    private ListView onlineUsersList;
    private OnlineUserAdapter adapter;
    private ArrayList<User> onlineUsers;
    private boolean isBound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_online_users);

        onlineUsersList = findViewById(R.id.online_users_list);
        onlineUsers = new ArrayList<>();
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

        Intent intent = new Intent(this, TeamTalkService.class);
        bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (isBound) {
            ttservice.getEventHandler().unregisterListener(this);
            unbindService(serviceConnection);
            isBound = false;
        }
    }

    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            TeamTalkService.LocalBinder binder = (TeamTalkService.LocalBinder) service;
            ttservice = binder.getService();
            isBound = true;
            registerEventListeners();
            populateUserList();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            isBound = false;
        }
    };

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

    // Helper method to find user by nUserID
    private int findUserIndex(User user) {
        for (int i = 0; i < onlineUsers.size(); i++) {
            if (onlineUsers.get(i).nUserID == user.nUserID) {
                return i;
            }
        }
        return -1;
    }

    @Override
    public void onCmdUserLoggedIn(User user) {
        Log.d(TAG, "User logged in: " + user.szNickname);
        int index = findUserIndex(user);
        if (index != -1) {
            onlineUsers.set(index, user);
        } else {
            onlineUsers.add(user);
        }
        sortAndNotifyDataSetChanged();
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
        int index = findUserIndex(user);
        if (index != -1) {
            onlineUsers.set(index, user);
        } else {
            onlineUsers.add(user);
        }
        sortAndNotifyDataSetChanged();
    }

    @Override
    public void onCmdUserLeftChannel(int nChannelID, User user) {
        Log.d(TAG, "User " + user.szNickname + " left channel " + nChannelID);
        int index = findUserIndex(user);
        if (index != -1) {
            onlineUsers.set(index, user);
        }
        sortAndNotifyDataSetChanged();
    }

    @Override
    public void onCmdUserUpdate(User user) {
        Log.d(TAG, "User updated: " + user.szNickname);
        int index = findUserIndex(user);
        if (index != -1) {
            onlineUsers.set(index, user);
        } else {
            onlineUsers.add(user);
        }
        sortAndNotifyDataSetChanged();
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
                userInfo.append("Nickname: ").append(user.szNickname);

                if (!user.szUsername.isEmpty()) {
                    userInfo.append(", Username: ").append(user.szUsername);
                }

                if (ttservice != null && ttservice.getChannels() != null) {
                    Channel userChannel = ttservice.getChannels().get(user.nChannelID);
                    if (userChannel != null) {
                        userInfo.append(", Channel: ").append(userChannel.szName);
                    }
                }

                if (!user.szIPAddress.isEmpty()) {
                    userInfo.append(", IP: ").append(user.szIPAddress);
                }

                                if (!user.szClientName.isEmpty()) {
                    userInfo.append(", Client: ").append(user.szClientName);
                    String clientVersion = ((user.uVersion >> 16) & 0xFF) + "." + ((user.uVersion >> 8) & 0xFF) + "." + (user.uVersion & 0xFF);
                    userInfo.append(" ").append(clientVersion);
                }

                textView.setText(userInfo.toString());
            }

            return convertView;
        }
    }
}
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

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import dk.bearware.Codec;
import dk.bearware.TeamTalkBase;
import dk.bearware.VideoCodec;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.Permissions;

public class StreamMediaActivity
extends AppCompatActivity implements TeamTalkConnectionListener {

    public static final String TAG = "bearware";
    public static final int REQUEST_STREAM_MEDIA = 1;
    private EditText file_path;
    private static final String lastMedia = "last_media_file";
    TeamTalkConnection mConnection;

    TeamTalkService getService() {
        return mConnection.getService();
    }

    TeamTalkBase getClient() {
        return getService().getTTInstance();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mConnection = new TeamTalkConnection(this);
        setContentView(R.layout.activity_stream_media);
        EdgeToEdgeHelper.enableEdgeToEdge(this);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        file_path = this.findViewById(R.id.file_path_txt);
        file_path.setText(PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString(lastMedia, ""));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.text_message, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if(id == R.id.action_settings) {
            return true;
        }
        else if (id == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onStart() {
        super.onStart();        
        if (!mConnection.isBound()) {
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
                Log.e(TAG, "Failed to bind to TeamTalk service");
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if(mConnection.isBound()) {
            onServiceDisconnected(getService());
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Permissions granted = Permissions.onRequestResult(this, requestCode, grantResults);
        if (granted == null)
            return;
        if (granted == Permissions.READ_EXTERNAL_STORAGE ||
            granted == Permissions.READ_MEDIA_VIDEO ||
            granted == Permissions.READ_MEDIA_AUDIO) {
            if ((Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) || areMediaPermissionsComplete())
                mediaSelectionStart();
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
        Button browse_btn = this.findViewById(R.id.media_file_select_btn);
        Button stream_btn = this.findViewById(R.id.media_file_stream_btn);

        OnClickListener listener = v -> {
            int id = v.getId();
            if (id == R.id.media_file_select_btn) {
                if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) ?
                    requestMediaPermissions() :
                    Permissions.READ_EXTERNAL_STORAGE.request(this)) {
                    mediaSelectionStart();
                }
            } else if (id == R.id.media_file_stream_btn) {
                String path = file_path.getText().toString();
                if(path.isEmpty())
                    return;
                VideoCodec videocodec = new VideoCodec();
                videocodec.nCodec = Codec.NO_CODEC;
                if (!getClient().startStreamingMediaFileToChannel(path, videocodec)) {
                    Toast.makeText(StreamMediaActivity.this,
                    R.string.err_stream_media,
                    Toast.LENGTH_LONG).show();
                } else {
                    SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).edit();
                    editor.putString(lastMedia, path).apply();
                    finish();
                }
            }
        };

        browse_btn.setOnClickListener(listener);
        stream_btn.setOnClickListener(listener);
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if ((requestCode == REQUEST_STREAM_MEDIA) && (resultCode == RESULT_OK)) {
            file_path.setText(AbsolutePathHelper.getRealPath(this.getBaseContext(), data.getData()));
        }
        else {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    private void mediaSelectionStart() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        Intent i = Intent.createChooser(intent, "File");
        startActivityForResult(i, REQUEST_STREAM_MEDIA);
    }

    private boolean requestMediaPermissions() {
        boolean video = Permissions.READ_MEDIA_VIDEO.request(this);
        boolean audio = Permissions.READ_MEDIA_AUDIO.request(this);
        return areMediaPermissionsComplete() && (video || audio);
    }

    private boolean areMediaPermissionsComplete() {
        return !(Permissions.READ_MEDIA_VIDEO.isPending() ||
                 Permissions.READ_MEDIA_AUDIO.isPending());
    }

}

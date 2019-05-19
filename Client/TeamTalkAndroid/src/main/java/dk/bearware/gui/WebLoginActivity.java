package dk.bearware.gui;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import dk.bearware.data.AppInfo;
import dk.bearware.data.Preferences;

public class WebLoginActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_web_login);

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

        String username = prefs.getString(Preferences.PREF_GENERAL_BEARWARE_USERNAME, "");
        String token = prefs.getString(Preferences.PREF_GENERAL_BEARWARE_TOKEN, "");

        Button createBtn = findViewById(R.id.createbearwareidbutton);
        Button resetBtn = findViewById(R.id.resetbearwareidbutton);
        Button loginBtn = findViewById(R.id.bearwareloginbutton);

        createBtn.setOnClickListener(createClick);
        resetBtn.setOnClickListener(resetClick);
        loginBtn.setOnClickListener(loginClick);

        EditText bearwareid = findViewById(R.id.bearwareidedit);
        EditText bearwarepasswd = findViewById(R.id.bearwarepasswdedit);

        if (username.length() > 0) {
            createBtn.setVisibility(View.GONE);
            bearwarepasswd.setVisibility(View.GONE);
        }
        else {
            resetBtn.setVisibility(View.GONE);
        }

        bearwareid.setText(username);
    }

    View.OnClickListener createClick = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(AppInfo.BEARWARE_REGISTRATION_WEBSITE));
            startActivity(browserIntent);
        }
    };

    View.OnClickListener resetClick = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
            SharedPreferences.Editor editor = prefs.edit();
            editor.putString(Preferences.PREF_GENERAL_BEARWARE_USERNAME, "");
            editor.putString(Preferences.PREF_GENERAL_BEARWARE_TOKEN, "");
        }
    };

    View.OnClickListener loginClick = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            new ValidateLogin().execute();
        }
    };

    class ValidateLogin extends AsyncTask<Void, Void, Void> {

        String username, password, token = "";

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            EditText bearwareid = findViewById(R.id.bearwareidedit);
            EditText bearwarepasswd = findViewById(R.id.bearwarepasswdedit);
            this.username = bearwareid.getText().toString();
            this.password = bearwarepasswd.getText().toString();
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String xml = Utils.getURL(AppInfo.getBearWareTokenUrl(getBaseContext(),
                        this.username, this.password));
            Log.d(AppInfo.TAG, xml);
            return null;
        }


        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            if (token.length() > 0) {
                setResult(RESULT_OK);
                finish();
            }
            else {

            }
        }
    };

}

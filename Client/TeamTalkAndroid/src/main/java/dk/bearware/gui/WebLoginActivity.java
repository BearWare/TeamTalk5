package dk.bearware.gui;

import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import androidx.appcompat.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import dk.bearware.data.AppInfo;
import dk.bearware.data.Preferences;

public class WebLoginActivity extends AppCompatActivity {

    void setupUI() {

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

        String username = prefs.getString(Preferences.PREF_GENERAL_BEARWARE_USERNAME, "");

        Button createBtn = findViewById(R.id.createbearwareidbutton);
        Button resetBtn = findViewById(R.id.resetbearwareidbutton);
        Button loginBtn = findViewById(R.id.bearwareloginbutton);

        createBtn.setOnClickListener(createClick);
        resetBtn.setOnClickListener(resetClick);
        loginBtn.setOnClickListener(loginClick);

        EditText bearwareid = findViewById(R.id.bearwareidedit);
        TextView passwdlabel = findViewById(R.id.bearwarepasswdlabel);
        EditText bearwarepasswd = findViewById(R.id.bearwarepasswdedit);

        if (username.length() > 0) {
            createBtn.setVisibility(View.GONE);
            bearwareid.setEnabled(false);
            bearwarepasswd.setVisibility(View.GONE);
            resetBtn.setVisibility(View.VISIBLE);
            loginBtn.setVisibility(View.GONE);
            passwdlabel.setVisibility(View.GONE);
        }
        else {
            createBtn.setVisibility(View.VISIBLE);
            bearwareid.setEnabled(true);
            bearwarepasswd.setVisibility(View.VISIBLE);
            resetBtn.setVisibility(View.GONE);
            loginBtn.setVisibility(View.VISIBLE);
            passwdlabel.setVisibility(View.VISIBLE);
        }

        bearwareid.setText(username);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_web_login);

        setupUI();
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
            editor.apply();
            setupUI();
        }
    };

    View.OnClickListener loginClick = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            new ValidateLogin().execute();
        }
    };

    class ValidateLogin extends AsyncTask<Void, Void, Void> {

        String username = "", password, token = "", nickname = "";

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

            try {
                InputSource src = new InputSource(new StringReader(xml));
                DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                DocumentBuilder db = dbf.newDocumentBuilder();
                Document document = db.parse(src);
                XPathFactory factory = XPathFactory.newInstance();
                XPath xPath = factory.newXPath();

                this.username = (String)xPath.evaluate("/teamtalk/bearware/username", document, XPathConstants.STRING);
                this.nickname= (String)xPath.evaluate("/teamtalk/bearware/nickname", document, XPathConstants.STRING);
                this.token= (String)xPath.evaluate("/teamtalk/bearware/token", document, XPathConstants.STRING);

            } catch (XPathExpressionException e) {
                Log.e(AppInfo.TAG, "XPath failed: " + e);
            } catch (ParserConfigurationException e) {
                Log.e(AppInfo.TAG, "Parser cfg failed: " + e);
            } catch (IOException e) {
                Log.e(AppInfo.TAG, "XML IOException: " + e);
            } catch (SAXException e) {
                Log.e(AppInfo.TAG, "XML SAXException: " + e);
            }

            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            if (token.length() > 0) {

                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(WebLoginActivity.this.getBaseContext());
                SharedPreferences.Editor editor = prefs.edit();
                editor.putString(Preferences.PREF_GENERAL_BEARWARE_USERNAME, this.username);
                editor.putString(Preferences.PREF_GENERAL_BEARWARE_TOKEN, this.token);
                editor.apply();

                String msg = WebLoginActivity.this.getResources().getString(R.string.text_bearwarelogin_success, this.nickname, this.username);
                Toast.makeText(WebLoginActivity.this, msg, Toast.LENGTH_LONG).show();

                WebLoginActivity.this.setupUI();
                setResult(RESULT_OK);
                //finish();
            }
            else {
                String msg = WebLoginActivity.this.getResources().getString(R.string.text_bearwarelogin_failed, this.nickname, this.username);
                Toast.makeText(WebLoginActivity.this, msg, Toast.LENGTH_LONG).show();
            }
        }
    };

}

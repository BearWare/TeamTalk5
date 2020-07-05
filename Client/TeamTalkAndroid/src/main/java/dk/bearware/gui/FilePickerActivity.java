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

import java.io.File;
import java.io.FileFilter;
import java.util.Comparator;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import androidx.fragment.app.ListFragment;
import androidx.appcompat.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class FilePickerActivity
extends AppCompatActivity
implements AdapterView.OnItemClickListener, FileFilter, Comparator<File> {

    public static final String CURRENT_DIRECTORY = "filepicker_directory";
    public static final String SELECTED_FILE = "selected_file";
    public static final String FILTER_EXTENSION = "filter_extension";

    private File currentDirectory;
    private int currentPosition = 0;
    private String acceptedSuffix;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_file_picker);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        acceptedSuffix = getIntent().getStringExtra(FILTER_EXTENSION);
        getListFragment().getListView().setOnItemClickListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        currentPosition = getListFragment().getSelectedItemPosition();
    }

    @Override
    protected void onResume() {
        super.onResume();
        getListFragment().setSelection(currentPosition);
    }

    @Override
    protected void onStart() {
        super.onStart();
        getListFragment().setListAdapter(new ArrayAdapter<File>(this, R.layout.item_file) {
                @Override
                public View getView(int position, View convertView, ViewGroup parent) {
                    if(convertView == null)
                        convertView = LayoutInflater.from(getContext()).inflate(R.layout.item_file, parent, false);
                    File item = getItem(position);
                    TextView filename = (TextView) convertView.findViewById(R.id.filename);
                    ImageView icon = (ImageView) convertView.findViewById(R.id.icon);
                    if ((position != 0) || (currentDirectory.getParentFile() == null)) {
                        filename.setText(item.getName());
                        icon.setImageResource(item.isDirectory() ? R.drawable.folder : R.drawable.file);
                    }
                    else {
                        filename.setText("..");
                        icon.setImageResource(R.drawable.back);
                    }
                    return convertView;
                }
            });

        File currentPath = new File(CURRENT_DIRECTORY);
        if (!currentPath.exists()) {
            SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).edit();
            editor.remove(CURRENT_DIRECTORY).apply();
        }

        browseDirectory(new File(PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString(CURRENT_DIRECTORY, Environment.getExternalStorageDirectory().getAbsolutePath())));
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (currentDirectory != null) {
            SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).edit();
            editor.putString(CURRENT_DIRECTORY, currentDirectory.getAbsolutePath()).apply();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            setResult(RESULT_CANCELED);
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }


    @Override
    public void onItemClick(AdapterView<?> l, View v, int position, long id) {
        ArrayAdapter<File> files = getContentAdapter();
        File selectedFile = files.getItem(position);
        if (selectedFile.isDirectory()) {
            File item = currentDirectory;
            browseDirectory(selectedFile);
            getListFragment().setSelection(files.getPosition(item));
        }
        else {
            setResult(RESULT_OK, getIntent().putExtra(SELECTED_FILE, selectedFile.getAbsolutePath()));
            finish();
        }
    }


    @Override
    public int compare(File file1, File file2) {
        if (file1.isDirectory() && !file2.isDirectory()) {
            return -1;
        }
        else if (!file1.isDirectory() && file2.isDirectory()) {
            return 1;
        }
        else {
            return file1.getName().compareToIgnoreCase(file2.getName());
        }
    }


    @Override
    public boolean accept(File file) {
        return file.canRead() && (file.isDirectory() || (acceptedSuffix == null) || acceptedSuffix.isEmpty() || file.getName().endsWith(acceptedSuffix));
    }


    @SuppressWarnings("unchecked")
    private ArrayAdapter<File> getContentAdapter() {
        return (ArrayAdapter<File>) getListFragment().getListAdapter();
    }

    private void browseDirectory(File path) {
        ArrayAdapter<File> files = getContentAdapter();
        File parent = path.getParentFile();
        getSupportActionBar().setSubtitle(path.getAbsolutePath());
        files.clear();
        files.addAll(path.listFiles(this));
        files.sort(this);
        if (parent != null)
            files.insert(parent, 0);
        currentDirectory = path;
        getListFragment().setListAdapter(files);
    }

    private ListFragment getListFragment() {
        return (ListFragment) getSupportFragmentManager().findFragmentById(R.id.list_fragment);
    }

}

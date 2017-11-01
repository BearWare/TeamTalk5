/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.gui;

import java.io.File;
import java.io.FileFilter;
import java.util.Comparator;

import android.app.ListActivity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class FilePickerActivity
extends ListActivity
implements FileFilter, Comparator<File> {

    public static final String CURRENT_DIRECTORY = "filepicker_directory";
    public static final String SELECTED_FILE = "selected_file";
    public static final String FILTER_EXTENSION = "filter_extension";

    private File currentDirectory;
    private int currentPosition = 0;
    private String acceptedSuffix;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        acceptedSuffix = getIntent().getStringExtra(FILTER_EXTENSION);
    }

    @Override
    protected void onPause() {
        super.onPause();
        currentPosition = getSelectedItemPosition();
    }

    @Override
    protected void onResume() {
        super.onResume();
        setSelection(currentPosition);
    }

    @Override
    protected void onStart() {
        super.onStart();
        setListAdapter(new ArrayAdapter<File>(this, R.layout.item_file) {
                @Override
                public View getView(int position, View convertView, ViewGroup parent) {
                    if(convertView == null)
                        convertView = LayoutInflater.from(getContext()).inflate(R.layout.item_file, null);
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
    public void onListItemClick(ListView l, View v, int position, long id) {
        ArrayAdapter<File> files = getContentAdapter();
        File selectedFile = files.getItem(position);
        if (selectedFile.isDirectory()) {
            File item = currentDirectory;
            browseDirectory(selectedFile);
            setSelection(files.getPosition(item));
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
        return (ArrayAdapter<File>)getListAdapter();
    }

    private void browseDirectory(File path) {
        ArrayAdapter<File> files = getContentAdapter();
        File parent = path.getParentFile();
        getActionBar().setSubtitle(path.getAbsolutePath());
        files.clear();
        files.addAll(path.listFiles(this));
        files.sort(this);
        if (parent != null)
            files.insert(parent, 0);
        currentDirectory = path;
        setListAdapter(files);
    }

}

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

package dk.bearware.data;

import java.util.Vector;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

public class MapAdapter extends BaseAdapter {

    Vector<String> keys = new Vector<>();
    Vector<Integer> values = new Vector<>();
    
    LayoutInflater inflater;
    int layout_item, text_id;
    
    public MapAdapter(Context context, int layout_item, int text_id) {
        inflater = LayoutInflater.from(context);
        this.layout_item = layout_item;
        this.text_id = text_id;
    }
    
    public void addPair(String key, int value) {
        keys.add(key);
        values.add(value);
    }
    
    public int getIndex(int value, int invalid_index) {
        for(int i=0;i<values.size();i++) {
            if(values.get(i) == value)
                return i;
        }
        return invalid_index;
    }

    public int getValue(int position, int invalid_value) {
        if(position >= 0 && position < values.size())
            return (int)values.get(position);
        return invalid_value;
    }
    public int getValue(String key, int invalid_value) {
        for(int i=0;i<keys.size();i++)
            if(keys.get(i).equals(key))
                return values.get(i);
        return invalid_value;
    }
    
    @Override
    public int getCount() {
        return keys.size();
    }

    @Override
    public Object getItem(int position) {
        return values.get(position);
    }

    @Override
    public long getItemId(int arg0) {
        return arg0;
    }

    @Override
    public View getView(int pos, View convertView, ViewGroup parent) {
        if(convertView == null)
            convertView = inflater.inflate(layout_item, null);
        
        TextView text = (TextView)convertView.findViewById(text_id);
        text.setText(keys.get(pos));

        return convertView;
    }
}

package dk.bearware.data;

import java.util.HashMap;
import java.util.Vector;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

public class MapAdapter extends BaseAdapter {

    Vector<String> keys = new Vector<String>();
    Vector<Integer> values = new Vector<Integer>();
    
    LayoutInflater inflater;
    
    public MapAdapter(Context context) {
        inflater = LayoutInflater.from(context);
    }
    
    public void addPair(String key, int value) {
        keys.add(key);
        values.add(value);
    }
    
    public int getIndex(int value, int invalid_index) {
        for(int i=0;i<values.size();i++) {
            if(values.get(i).intValue() == value)
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
            convertView = inflater.inflate(android.R.layout.simple_spinner_item, null);
        
        TextView text = (TextView)convertView.findViewById(android.R.id.text1);
        text.setText(keys.get(pos));

        return convertView;
    }
}

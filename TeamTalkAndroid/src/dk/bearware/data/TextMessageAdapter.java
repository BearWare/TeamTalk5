package dk.bearware.data;

import java.util.Vector;

import dk.bearware.data.MyTextMessage;
import dk.bearware.gui.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

public class TextMessageAdapter extends BaseAdapter {

    private Vector<MyTextMessage> messages;
    
    private LayoutInflater inflater;

    public TextMessageAdapter(Context context, Vector<MyTextMessage> msgs) {
        inflater = LayoutInflater.from(context);
        setTextMessages(msgs);
    }
    
    public TextMessageAdapter(Context context) {
        inflater = LayoutInflater.from(context);
        setTextMessages(new Vector<MyTextMessage>());
    }
    
    public void setTextMessages(Vector<MyTextMessage> msgs) {
        messages = msgs;
    }
    
    @Override
    public int getCount() {
        return messages.size();
    }

    @Override
    public Object getItem(int position) {
        return messages.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView_, ViewGroup parent) {
        View convertView = inflater.inflate(R.layout.textmsg_item, null);
        
        TextView name = (TextView) convertView.findViewById(R.id.name_text);
        TextView msgtext = (TextView) convertView.findViewById(R.id.msg_text);
        TextView msgdate = (TextView) convertView.findViewById(R.id.time_text);
        name.setText(messages.get(position).szNickName);
        msgdate.setText(messages.get(position).time.toString());
        msgtext.setText(messages.get(position).szMessage);
        
        return convertView;
    }
}

package org.kemerelab.rsmcontrol;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.List;

/**
 * Created by ckemere on 9/16/15.
 */
public class RSMDeviceInfoListAdapter extends ArrayAdapter<RSMDeviceInfoAtom> {
//
//    public RSMDeviceInfoListAdapter(Context context, int textViewResourceId) {
//        super(context, textViewResourceId);
//    }

    public RSMDeviceInfoListAdapter(Context context, List<RSMDeviceInfoAtom> items) {
        super(context, 0, items);
    }

    @Override
    public int getViewTypeCount() {
        return 2;
    }


    @Override
    public int getItemViewType(int position) {
        RSMDeviceInfoAtom p = getItem(position);
        if (p.datum.equals("")) {
            return 1;
        }
        else {
            return 0;
        }
    }

    static class ViewHolder {
        TextView caption;
        TextView datum;
        TextView suffix;
        TextView header;
    }

    @Override
    public View getView(int position, View v, ViewGroup parent) {

        ViewHolder holder;
        RSMDeviceInfoAtom p = getItem(position);
        if (v == null) {
            holder = new ViewHolder();
            LayoutInflater vi;
            vi = LayoutInflater.from(getContext());
            if (p.datum.equals("")) {
                v = vi.inflate(R.layout.device_info_header, null);
                holder.header = (TextView) v.findViewById(R.id.SectionHeading);
                v.setTag(holder);
            } else {
                v = vi.inflate(R.layout.device_info_atom, null);
                holder.caption = (TextView) v.findViewById(R.id.Caption);
                holder.datum = (TextView) v.findViewById(R.id.Data);
                holder.suffix = (TextView) v.findViewById(R.id.Suffix);
                v.setTag(holder);
            }
        }
        else {
            holder = (ViewHolder) v.getTag();
        }

        if (p.datum.equals("")) {
            holder.header.setText(p.header);
        }
        else {
            holder.caption.setText(p.caption);
            holder.datum.setText(p.datum);
            holder.suffix.setText(p.suffix);
        }

        return v;
    }

}

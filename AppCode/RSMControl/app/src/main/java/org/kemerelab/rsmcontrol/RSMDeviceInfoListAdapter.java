package org.kemerelab.rsmcontrol;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
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
        return 3;
    }


    @Override
    public int getItemViewType(int position) {
        RSMDeviceInfoAtom p = getItem(position);
        if (p.datum.equals("")) {
            return 2;
        }
        else {
            if (p.settingsType == RSMDevice.UserSettings.STATUS_ATOM) {
                return 1;
            }
            else {
                return 0;
            }
        }
    }

    static class ViewHolder {
        TextView caption;
        TextView datum;
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
                switch (p.settingsType) {
                    case STATUS_ATOM:
                        v = vi.inflate(R.layout.device_info_status_atom, null);
                        break;
                    case ENABLE_STIMULATION:
                        v = vi.inflate(R.layout.device_info_atom_button, null);
                        break;
                    default:
                        v = vi.inflate(R.layout.device_info_atom, null);
                        break;
                }
                holder.caption = (TextView) v.findViewById(R.id.Caption);
                holder.datum = (TextView) v.findViewById(R.id.Data);
                v.setTag(holder);
            }
        }
        else {
            holder = (ViewHolder) v.getTag();
        }

        if (p.datum.equals("")) {
            holder.header.setText(p.caption);
        }
        else {
            holder.caption.setText(p.caption);
            holder.datum.setText(p.datum);
        }

        return v;
    }

}

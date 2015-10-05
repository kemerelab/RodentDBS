package org.kemerelab.rsmcontrol;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by ckemere on 9/16/15.
 */
public class RSMDeviceInfoListAdapter extends ArrayAdapter<RSMDeviceInfoAtom> {

    boolean displayOnly; // Just display options?
    boolean isEnabled; // Enable things like spinners?

    public RSMDeviceInfoListAdapter(Context context, List<RSMDeviceInfoAtom> items,
                                    boolean _displayOnly, boolean _isEnabled) {
        super(context, 0, items);
        displayOnly = _displayOnly;
        isEnabled = _isEnabled;
    }

    public interface RSMDeviceInfoSpinnerSelectionListener {
        public void onSpinnerSelectionListener(RSMDeviceInfoAtom.SettingsType settingsType, int spinnerPosition);
    }

    RSMDeviceInfoSpinnerSelectionListener spinnerListener;

    public void setSpinnerListener(RSMDeviceInfoSpinnerSelectionListener _spinnerListener) {
        spinnerListener = _spinnerListener;
    }


    @Override
    public int getViewTypeCount() {
        return 4;
    }


    @Override
    public int getItemViewType(int position) {
        RSMDeviceInfoAtom p = getItem(position);
        if ((p.atomFormatting == RSMDeviceInfoAtom.AtomFormatting.HEADER) ||
                (p.atomFormatting == RSMDeviceInfoAtom.AtomFormatting.STATUS_HEADER)){
            return 3; // Headers - formatted special!
        }
        else {
            switch (p.atomAction) {
                case NONE:
                    return 2;
                case SPINNER:
                    return 1;
                default:
                    return 0;
            }
        }
    }

    static class ViewHolder {
        TextView caption;
        TextView datum;
        TextView header;
        Spinner datumValues;
        RSMDeviceInfoAtom.SettingsType settingsType; // used by spinner callbacks
    }

    @Override
    public View getView(int position, View v, ViewGroup parent) {

        final ViewHolder holder;
        RSMDeviceInfoAtom p = getItem(position);
        if (v == null) {
            holder = new ViewHolder();
            holder.settingsType = p.settingsType;
            LayoutInflater vi;
            vi = LayoutInflater.from(getContext());
            if ((p.atomFormatting == RSMDeviceInfoAtom.AtomFormatting.HEADER) ||
                    (p.atomFormatting == RSMDeviceInfoAtom.AtomFormatting.STATUS_HEADER)) {
                v = vi.inflate(R.layout.device_info_header, null);
                holder.header = (TextView) v.findViewById(R.id.SectionHeading);
                v.setTag(holder);
            } else {
                switch (p.atomAction) {
                    case NONE:
                        v = vi.inflate(R.layout.device_info_status_atom, null);
                        holder.caption = (TextView) v.findViewById(R.id.Caption);
                        holder.datum = (TextView) v.findViewById(R.id.Data);
                        break;
                    case TOGGLE_BUTTON:
                        if (displayOnly)
                            v = vi.inflate(R.layout.device_info_atom, null);
                        else
                            v = vi.inflate(R.layout.device_info_atom_button, null);
                        holder.caption = (TextView) v.findViewById(R.id.Caption);
                        holder.datum = (TextView) v.findViewById(R.id.Data);
                        break;
                    case SPINNER:
                        if (displayOnly) {
                            v = vi.inflate(R.layout.device_info_atom, null);
                            holder.caption = (TextView) v.findViewById(R.id.Caption);
                            holder.datum = (TextView) v.findViewById(R.id.Data);
                        }
                        else {
                            v = vi.inflate(R.layout.device_info_atom_spinner, null);
                            holder.caption = (TextView) v.findViewById(R.id.Caption);
                            holder.datumValues = (Spinner) v.findViewById(R.id.DataSpinner);
                            ArrayAdapter<String> adapter = new ArrayAdapter<String>(getContext(),
                                    android.R.layout.simple_spinner_item, p.stringValues);
                            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                            holder.datumValues.setAdapter(adapter);
                            holder.datumValues.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                                @Override
                                public void onItemSelected(AdapterView<?> parentView,
                                                           View selectedItemView, int position, long id) {
                                    if (spinnerListener != null) {
                                        spinnerListener.onSpinnerSelectionListener(holder.settingsType, position);
                                    }
                                }

                                @Override
                                public void onNothingSelected(AdapterView<?> parentView) {
                                    // don't do anything
                                }
                            });
                            holder.datumValues.setEnabled(isEnabled);
                            holder.datumValues.setClickable(isEnabled);
                        }
                        break;
                    default:
                        v = vi.inflate(R.layout.device_info_atom, null);
                        holder.caption = (TextView) v.findViewById(R.id.Caption);
                        holder.datum = (TextView) v.findViewById(R.id.Data);
                        break;
                }
                v.setTag(holder);
            }
        }
        else {
            holder = (ViewHolder) v.getTag();
        }

        // Actually update values!
        if ((p.atomFormatting == RSMDeviceInfoAtom.AtomFormatting.HEADER) ||
                (p.atomFormatting == RSMDeviceInfoAtom.AtomFormatting.STATUS_HEADER)) {
            holder.header.setText(p.caption);
        }
        else {
            holder.caption.setText(p.caption);
            switch (p.atomAction) {
                case SPINNER:
                    if (displayOnly) {
                        holder.datum.setText(p.stringValues.get(p.currentPosition));
                    } else {
                        holder.datumValues.setSelection(p.currentPosition);
                        holder.datumValues.setEnabled(isEnabled);
                        holder.datumValues.setClickable(isEnabled);
                    }
                    break;
                case NONE:
                case TOGGLE_BUTTON:
                case DIALOG:
                default:
                    holder.datum.setText(p.stringDatum);
                    break;
            }
        }

        return v;
    }

}

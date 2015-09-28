package org.kemerelab.rsmcontrol;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

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
        public void onSpinnerSelectionListener(RSMDevice.SettingsType settingsType, int spinnerPosition);
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
        if (p.isHeader) {
            return 3; // Headers - formatted special!
        }
        else {
            if (p.settingsType == RSMDevice.SettingsType.STATUS_ATOM) {
                return 2; // Status info are always display only
            }
            else {
                if (p.datumValue >= 0)
                    return 1;  // Should be a spinner
                else
                    return 0; // Should be a selector dialog or (textview-based) button
            }
        }
    }

    static class ViewHolder {
        TextView caption;
        TextView datum;
        TextView header;
        Spinner datumValues;
        RSMDevice.SettingsType settingsType; // used by spinner callbacks
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
            if (p.isHeader) {
                v = vi.inflate(R.layout.device_info_header, null);
                holder.header = (TextView) v.findViewById(R.id.SectionHeading);
                v.setTag(holder);
            } else {
                switch (p.settingsType) {
                    case STATUS_ATOM:
                        v = vi.inflate(R.layout.device_info_status_atom, null);
                        holder.caption = (TextView) v.findViewById(R.id.Caption);
                        holder.datum = (TextView) v.findViewById(R.id.Data);
                        break;
                    case ENABLE_STIMULATION:
                        if (displayOnly)
                            v = vi.inflate(R.layout.device_info_atom, null);
                        else
                            v = vi.inflate(R.layout.device_info_atom_button, null);
                        holder.caption = (TextView) v.findViewById(R.id.Caption);
                        holder.datum = (TextView) v.findViewById(R.id.Data);
                        break;
                    case JITTER_LEVEL:
                        if (displayOnly) {
                            v = vi.inflate(R.layout.device_info_atom, null);
                            holder.caption = (TextView) v.findViewById(R.id.Caption);
                            holder.datum = (TextView) v.findViewById(R.id.Data);
                        }
                        else {
                            v = vi.inflate(R.layout.device_info_atom_spinner, null);
                            holder.caption = (TextView) v.findViewById(R.id.Caption);
                            holder.datumValues = (Spinner) v.findViewById(R.id.DataSpinner);
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
        if (p.isHeader) {
            holder.header.setText(p.caption);
        }
        else {
            holder.caption.setText(p.caption);
            switch (p.settingsType) {
                case JITTER_LEVEL:
                    if (displayOnly) {
                        String[] jitterValues = getContext().getResources().getStringArray(R.array.jitter_levels);
                        holder.datum.setText(jitterValues[p.datumValue]);
                    } else {
                        holder.datumValues.setSelection(p.datumValue);
                        holder.datumValues.setEnabled(isEnabled);
                        holder.datumValues.setClickable(isEnabled);
                    }
                    break;
                case STATUS_ATOM:
                case ENABLE_STIMULATION:
                default:
                    holder.datum.setText(p.datum);
                    break;
            }
        }

        return v;
    }

}

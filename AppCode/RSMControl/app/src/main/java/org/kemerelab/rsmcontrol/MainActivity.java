package org.kemerelab.rsmcontrol;

import org.ndeftools.Message;
import org.ndeftools.Record;
import org.ndeftools.externaltype.ExternalTypeRecord;
import org.ndeftools.util.activity.NfcReaderActivity;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.NumberPicker;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;


public class MainActivity extends NfcReaderActivity implements AdapterView.OnItemClickListener {


    static final String RSM_DEVICE_STRUCTURE = "rsmDevice";
    static final String NDEF_DATA_TO_BE_WRITTEN = "NDEF_Data";

    RSMDevice rsmDevice;

    public enum NFCAvalability {
        UNINITIALIZED, NOT_AVAIALBLE, AVAILABLE_NOT_ENABLED, AVAILABLE_ENABLED
    }

    NFCAvalability nfcAvalability = NFCAvalability.UNINITIALIZED;

    private Boolean editingDeviceInfo = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (savedInstanceState != null) {
            // Restore value of members from saved state
            rsmDevice = savedInstanceState.getParcelable(RSM_DEVICE_STRUCTURE);

        } else {
            // Probably initialize members with default values for a new instance
            rsmDevice = new RSMDevice();
        }

        setContentView(R.layout.activity_main);

        updateStatus(nfcAvalability);
        updateDeviceDisplay();
        updateDeviceInfoEditState();

        // lets start detecting NDEF message using foreground mode
        setDetecting(true);
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        // Save the current state of the
        savedInstanceState.putParcelable(RSM_DEVICE_STRUCTURE, rsmDevice);

        // Always call the superclass so it can save the view hierarchy state
        super.onSaveInstanceState(savedInstanceState);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
//            Intent intent = new Intent(this, SettingsActivity.class);
//            startActivity(intent);
//
//            return true;
        }

        return super.onOptionsItemSelected(item);
    }


    public void toast(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL, 0, 0);
        toast.show();
    }

    public void onEnableSwitchToggled (View view) {
        if (((Switch) view).isChecked()) {
            rsmDevice.enableStimulation();
        }
        else {
            rsmDevice.disableStimulation();
        }
    }


    public void onEditSwitchToggled (View view) {
        if (((Switch) view).isChecked()) {
            editingDeviceInfo = true;
        }
        else {
            updateRSMDeviceValues();
            editingDeviceInfo = false;
        }

        InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        mgr.hideSoftInputFromWindow(view.getWindowToken(), 0);

        updateDeviceInfoEditState();
    }

    public void onProgramButtonClicked (View view) {
        updateRSMDeviceValues();
        Intent intent = new Intent(this, NFCWriteActivity.class);
        intent.putExtra(NDEF_DATA_TO_BE_WRITTEN, rsmDevice.getDeviceInfoAsByteArray());
        intent.putExtra(RSM_DEVICE_STRUCTURE, rsmDevice);
        startActivity(intent);
    }

    public void updateDeviceInfoEditState() {
        ListView s = (ListView) findViewById(R.id.deviceInfoListView);
        if (!editingDeviceInfo) {
            switch (nfcAvalability) {
                case AVAILABLE_ENABLED:
                    s.setBackgroundColor(getResources().getColor(R.color.BackgroundLightGray));
                    break;
                case NOT_AVAIALBLE:
                case AVAILABLE_NOT_ENABLED:
                default:
                    s.setBackgroundColor(getResources().getColor(R.color.BackgroundLightRed));
                    break;
            }
       }
        else {
            s.setBackgroundColor(Color.WHITE);
        }
    }

    public void updateRSMDeviceValues() {
    }


    public void updateStatus(NFCAvalability nfcAvalability) {
        TextView t = (TextView) findViewById(R.id.statusTextView);
        if ( t!= null) {
            switch (nfcAvalability) {
                case NOT_AVAIALBLE:
                    t.setText(getString(R.string.noNfcMessage));
                    t.setTextColor(getResources().getColor(R.color.LightRed));
                    break;
                case AVAILABLE_NOT_ENABLED:
                    t.setText(getString(R.string.nfcAvailableDisabled));
                    t.setTextColor(getResources().getColor(R.color.LightRed));
                    break;
                case AVAILABLE_ENABLED:
                    t.setText(getString(R.string.nfcAvailableEnabled));
                    t.setTextColor(getResources().getColor(R.color.DarkBlue));
                    break;
                default:
                    t.setText("Unknown state");
            }
        }
        updateDeviceInfoEditState();
    }

    public void updateStatus(String message) {
        TextView t = (TextView) findViewById(R.id.statusTextView);
        if ( t!= null) {
            t.setText(getString(R.string.nfcAvailableEnabled));
            t.setTextColor(getResources().getColor(R.color.LightBlue));
        }
    }



    public void updateDeviceDisplay() {
        ListView lv = (ListView) findViewById(R.id.deviceInfoListView);
        RSMDeviceInfoListAdapter devAdapter = new RSMDeviceInfoListAdapter(this,
                rsmDevice.getDeviceInfoList(this));
        lv.setAdapter(devAdapter);
        lv.setOnItemClickListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> adapter, View view, int position,
                            long id) {
        RSMDeviceInfoAtom atom = (RSMDeviceInfoAtom) adapter.getItemAtPosition(position);
        if ((!editingDeviceInfo) | (atom.settingsType == RSMDevice.UserSettings.NA)) {
            return;
        }

        if (atom.settingsType != RSMDevice.UserSettings.ENABLE_STIMULATION) {

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            // Get the layout inflater
            final RSMDevice.UserSettings ws = atom.settingsType;
            LayoutInflater inflater = this.getLayoutInflater();

            // Inflate and set the layout for the dialog

            View v = inflater.inflate(R.layout.settings_dialog, null);
            final NumberPicker amp = (NumberPicker) v.findViewById(R.id.stimAmplitudeNumberPicker);
            final NumberPicker freq = (NumberPicker) v.findViewById(R.id.stimCurrentNumberPicker);
            final NumberPicker pulWid = (NumberPicker) v.findViewById(R.id.pulseWidthNumberPicker);
            final EditText devId = (EditText) v.findViewById(R.id.deviceIDEditText);

            switch (ws) {
                case AMPLITUDE:
                    amp.setMaxValue((int) rsmDevice.getMaxAmplitude());
                    amp.setMinValue(0);
                    amp.setValue(Integer.parseInt(rsmDevice.getAmplitudeString()));
                    amp.setVisibility(View.VISIBLE);
                    break;
                case FREQUENCY:
                    freq.setMaxValue((int) 200);
                    freq.setMinValue((int) rsmDevice.getMinFrequency());
                    freq.setValue(Integer.parseInt(rsmDevice.getStimFrequencyString()));
                    freq.setVisibility(View.VISIBLE);
                    break;
                case PULSE_WIDTH:
                    pulWid.setMaxValue(100);
                    pulWid.setMinValue(20);
                    pulWid.setValue(rsmDevice.getPulseWidth());
                    pulWid.setVisibility(View.VISIBLE);
                    break;
                case DEVICE_ID:
                    devId.setVisibility(View.VISIBLE);
                    break;
                case NA:
                default:
                    break;

            }

            // Pass null as the parent view because its going in the dialog layout
            builder.setView(v)
                    // Add action buttons
                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int id) {

                            switch (ws) {
                                case AMPLITUDE:
                                    rsmDevice.setStimCurrent(amp.getValue());
                                    break;
                                case FREQUENCY:
                                    rsmDevice.setStimPeriod(freq.getValue());
                                    break;
                                case PULSE_WIDTH:
                                    rsmDevice.setPulseWidth((short) pulWid.getValue());
                                    break;
                                case DEVICE_ID:
                                    rsmDevice.setDeviceID(devId.getText().toString());
                                    break;
                                case NA:
                                default:
                                    break;

                            }
                            updateDeviceDisplay();
                        }
                    })
                    .setNegativeButton(R.string.cancel, null);
            builder.show();

        }
        else { // Process tapping "Enable Stimulation"
            rsmDevice.toggleStimulationEnabled();
            updateDeviceDisplay();
        }
    }


    /**
     *
     * This device does not have NFC hardware - Called in onCreate of NfcReaderActivity
     *
     */

    @Override
    protected void onNfcFeatureNotFound() {
        toast(getString(R.string.noNfcMessage));
        nfcAvalability = NFCAvalability.NOT_AVAIALBLE;
    }

    /**
     * NFC feature was found and is currently enabled  - Called in onCreate of NfcReaderActivity
     */

    @Override
    protected void onNfcStateEnabled() {
        toast(getString(R.string.nfcAvailableEnabled));
        nfcAvalability = NFCAvalability.AVAILABLE_ENABLED;
    }

    /**
     * NFC feature was found but is currently disabled - Called in onCreate of NfcReaderActivity
     */

    @Override
    protected void onNfcStateDisabled() {
        toast(getString(R.string.nfcAvailableDisabled));
        nfcAvalability = NFCAvalability.AVAILABLE_NOT_ENABLED;

    }

    /**
     * NFC setting changed since last check. For example, the user enabled NFC in the wireless settings.
     */

    @Override
    protected void onNfcStateChange(boolean enabled) {
        if (enabled) {
            toast(getString(R.string.nfcAvailableEnabled));
            nfcAvalability = NFCAvalability.AVAILABLE_ENABLED;
            updateStatus(nfcAvalability);
        } else {
            toast(getString(R.string.nfcAvailableDisabled));
            nfcAvalability = NFCAvalability.AVAILABLE_NOT_ENABLED;
            updateStatus(nfcAvalability);
        }
    }

    @Override
    protected void onTagLost() {
        toast(getString(R.string.tagLost));
    }

    /**
     * An NDEF message was read and parsed. This method prints its contents to log and then shows its contents in the GUI.
     *
     * @param message the message
     */

    @Override
    public void readNdefMessage(Message message) {
        if (message.size() > 1) {
            toast(getString(R.string.readMultipleRecordNDEFMessage));
        } else {
            Record record = message.get(0);
            if (record instanceof ExternalTypeRecord) {
                String domain = ((ExternalTypeRecord) record).getDomain();
                String recordType = ((ExternalTypeRecord) record).getType();
                if ((((ExternalTypeRecord) record).getDomain().equals("rnel.rice.edu")) &&
                        (((ExternalTypeRecord) record).getType().equals("rsm"))) {
                    RSMDevice device = new RSMDevice(((ExternalTypeRecord) record).getData());
                    if (device.isValid == true) {
                        toast(getString(R.string.readRSMNDEFMessage));
                        rsmDevice = device;
                        updateDeviceDisplay();
                    } else {
                        toast(getString(R.string.readRSMMessageError));
                    }
                }
                else {
                    toast(getString(R.string.readExternalRecordNDEFMessage));
                }
            }
            else {
                toast(getString(R.string.readSingleRecordNDEFMessage));
            }
        }

    }

    /**
     * An empty NDEF message was read.
     *
     */

    @Override
    protected void readEmptyNdefMessage() {
        toast(getString(R.string.readEmptyMessage));
    }

    /**
     *
     * Something was read via NFC, but it was not an NDEF message.
     *
     * Handling this situation is out of scope of this project.
     *
     */

    @Override
    protected void readNonNdefMessage() {
        toast(getString(R.string.readNonNDEFMessage));
    }


}

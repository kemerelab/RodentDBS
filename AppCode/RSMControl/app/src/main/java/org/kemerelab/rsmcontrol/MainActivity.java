package org.kemerelab.rsmcontrol;

import org.ndeftools.Message;
import org.ndeftools.Record;
import org.ndeftools.externaltype.ExternalTypeRecord;
import org.ndeftools.util.activity.NfcReaderActivity;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.text.InputType;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.NumberPicker;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;


public class MainActivity extends NfcReaderActivity {


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

        NumberPicker np = (NumberPicker) findViewById(R.id.stimulationFrequencyPicker);
        np.setMaxValue(200);
        np.setMinValue(20);

        np = (NumberPicker) findViewById(R.id.stimulationAmplitudePicker);
        np.setMaxValue(100);
        np.setMinValue(0);

        np = (NumberPicker) findViewById(R.id.stimulationPulseWidthPicker);
        np.setMaxValue(100);
        np.setMinValue(30);

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
            return true;
        }

        return super.onOptionsItemSelected(item);
    }


    public void toast(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL, 0, 0);
        toast.show();
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
        startActivity(intent);
    }

    public void updateDeviceInfoEditState() {
        ScrollView s = (ScrollView) findViewById(R.id.deviceView);
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
            EditText t = (EditText) findViewById(R.id.deviceIDText);
            t.setInputType(InputType.TYPE_NULL);

            NumberPicker np = (NumberPicker) findViewById(R.id.stimulationFrequencyPicker);
            np.setEnabled(false);

            np = (NumberPicker) findViewById(R.id.stimulationAmplitudePicker);
            np.setEnabled(false);

            np = (NumberPicker) findViewById(R.id.stimulationPulseWidthPicker);
            np.setEnabled(false);

        }
        else {
            s.setBackgroundColor(Color.WHITE);
            EditText t = (EditText) findViewById(R.id.deviceIDText);
            t.setInputType(InputType.TYPE_CLASS_TEXT);

            NumberPicker np = (NumberPicker) findViewById(R.id.stimulationFrequencyPicker);
            np.setEnabled(true);

            np = (NumberPicker) findViewById(R.id.stimulationAmplitudePicker);
            np.setEnabled(true);

            np = (NumberPicker) findViewById(R.id.stimulationPulseWidthPicker);
            np.setEnabled(true);
        }
    }

    public void updateRSMDeviceValues() {
        EditText t = (EditText) findViewById(R.id.deviceIDText);
        rsmDevice.deviceID = Integer.parseInt(t.getText().toString());

        NumberPicker np = (NumberPicker) findViewById(R.id.stimulationFrequencyPicker);
        short period = (short) ((int) 1000000 / (int) np.getValue());
        rsmDevice.stimulationPeriod = period;

        np = (NumberPicker) findViewById(R.id.stimulationAmplitudePicker);
        rsmDevice.stimulationAmplitude = (short) np.getValue();

        np = (NumberPicker) findViewById(R.id.stimulationPulseWidthPicker);
        rsmDevice.stimulationWidth = (short) np.getValue();
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
                    t.setTextColor(R.color.LightRed);
                    break;
                case AVAILABLE_ENABLED:
                    t.setText(getString(R.string.nfcAvailableEnabled));
                    t.setTextColor(R.color.LightGreen);
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
            t.setTextColor(R.color.LightBlue);
        }
        ScrollView s = (ScrollView) findViewById(R.id.deviceView);
        if (s != null)
            s.setBackgroundColor(R.color.BackgroundLightRed);
    }

    public void updateDeviceDisplay() {
        EditText t = (EditText) findViewById(R.id.deviceIDText);
        if (t != null)
            t.setText(rsmDevice.deviceID.toString(), TextView.BufferType.EDITABLE);

        NumberPicker np = (NumberPicker) findViewById(R.id.stimulationFrequencyPicker);
        Integer freq = 1000000 / rsmDevice.stimulationPeriod;
        np.setValue(freq);

        np = (NumberPicker) findViewById(R.id.stimulationAmplitudePicker);
        np.setValue(rsmDevice.stimulationAmplitude);

        np = (NumberPicker) findViewById(R.id.stimulationPulseWidthPicker);
        np.setValue(rsmDevice.stimulationWidth);

        TextView t2 = (TextView) findViewById(R.id.lastUpdateText);
        t2.setText(rsmDevice.getLastUpdateString(), TextView.BufferType.EDITABLE);

        t2 = (TextView) findViewById(R.id.uptimeText);
        t2.setText(rsmDevice.getUptimeString(), TextView.BufferType.EDITABLE);

        t2 = (TextView) findViewById(R.id.batteryVoltageText);
        t2.setText(rsmDevice.getBatteryVoltageString(), TextView.BufferType.EDITABLE);
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

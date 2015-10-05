package org.kemerelab.rsmcontrol;

import android.content.Intent;
import android.nfc.NdefMessage;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import org.ndeftools.Message;
import org.ndeftools.externaltype.GenericExternalTypeRecord;
import org.ndeftools.util.activity.NfcTagWriterActivity;

import java.nio.charset.Charset;
import java.util.List;
import java.util.ListIterator;


public class NFCWriteActivity extends NfcTagWriterActivity {

    //RSMDevice rsmDevice;
    byte[] NDEFData;

    public enum NFCAvalability {
        UNINITIALIZED, NOT_AVAIALBLE, AVAILABLE_NOT_ENABLED, AVAILABLE_ENABLED
    }

    NFCAvalability nfcAvalability = NFCAvalability.UNINITIALIZED;

    RSMDevice rsmDevice;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nfcwrite);

        if (savedInstanceState == null) {
            Intent intent = getIntent();
            NDEFData = intent.getByteArrayExtra(MainActivity.NDEF_DATA_TO_BE_WRITTEN);
            rsmDevice = intent.getExtras().getParcelable(MainActivity.RSM_DEVICE_STRUCTURE);
        }
        else {
            super.onRestoreInstanceState(savedInstanceState);
            NDEFData = savedInstanceState.getByteArray(MainActivity.NDEF_DATA_TO_BE_WRITTEN);
            rsmDevice = savedInstanceState.getParcelable(MainActivity.RSM_DEVICE_STRUCTURE);
        }

        ListView lv = (ListView) findViewById(R.id.deviceInfoToBeWritten);

        List<RSMDeviceInfoAtom> deviceInfoList = rsmDevice.getDeviceInfoList(this, false);
        RSMDeviceInfoListAdapter devAdapter = new RSMDeviceInfoListAdapter(this, deviceInfoList, true, false);
        lv.setAdapter(devAdapter);

        // lets start detecting NDEF message using foreground mode
        setDetecting(true);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_nfcwrite, menu);
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


    @Override
    protected NdefMessage createNdefMessage() {
        // Create the NDEF message to be written when a tag is within range.
        Message message = new Message();

        // add an External type Record with the proper message
        String domain = "rnel.rice.edu";
        String externalRecordType = "rsm";

        byte[] byteDomain = domain.getBytes(Charset.forName("UTF-8"));
        byte[] byteType = externalRecordType.getBytes(Charset.forName("UTF-8"));
        byte[] b = new byte[byteDomain.length + 1 + byteType.length];
        System.arraycopy(byteDomain, 0, b, 0, byteDomain.length);
        b[byteDomain.length] = ':';
        System.arraycopy(byteType, 0, b, byteDomain.length + 1, byteType.length);

        byte[] EMPTY = new byte[]{};

        // external type id must be empty
        //NdefRecord record =  new NdefRecord(NdefRecord.TNF_EXTERNAL_TYPE, b, EMPTY, NDEFData);
        GenericExternalTypeRecord record = new GenericExternalTypeRecord(domain, externalRecordType, NDEFData);
        message.add(record);

        return message.getNdefMessage();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putByteArray(MainActivity.NDEF_DATA_TO_BE_WRITTEN, NDEFData);
        outState.putParcelable(MainActivity.RSM_DEVICE_STRUCTURE, rsmDevice);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        NDEFData = savedInstanceState.getByteArray(MainActivity.NDEF_DATA_TO_BE_WRITTEN);
        rsmDevice = savedInstanceState.getParcelable(MainActivity.RSM_DEVICE_STRUCTURE);
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
    protected void writeNdefSuccess() {
        // Successfully wrote NDEF message to tag.
        toast(getString(R.string.ndefWriteSuccess));
    }

    @Override
    protected void onTagLost() {
        toast(getString(R.string.tagLost));
    }

    @Override
    protected void writeNdefFailed(Exception e) {
        // Writing NDEF message to tag failed.
        toast(getString(R.string.ndefWriteFailed, e.toString()));
    }

        @Override
    public void writeNdefNotWritable() {
        // Tag is not writable or write-protected.
        toast(getString(R.string.tagNotWritable));
    }

    @Override
    public void writeNdefTooSmall(int required, int capacity) {
        // Tag capacity is lower than NDEF message size.
        toast(getString(R.string.tagTooSmallMessage, required, capacity));
    }


    @Override
    public void writeNdefCannotWriteTech() {
        // Unable to write this type of tag.
        toast(getString(R.string.cannotWriteTechMessage));
    }

    @Override
    protected void onNfcFeatureNotFound() {
        // This device does not have NFC hardware - Called in onCreate of NfcReaderActivity
        toast(getString(R.string.noNfcMessage));
        finish();
    }

    @Override
    protected void onNfcStateEnabled() {
        // NFC feature was found and is currently enabled  - Called in onCreate of NfcReaderActivity
        // we better be in this state!
    }

    @Override
    protected void onNfcStateDisabled() {
        // NFC feature was found but is currently disabled - Called in onCreate of NfcReaderActivity
        toast(getString(R.string.nfcAvailableDisabled));
        finish();
    }

    @Override
    protected void onNfcStateChange(boolean enabled) {
        // NFC setting changed since last check. For example, the user enabled NFC in the wireless settings.
        if (enabled) {
            //toast(getString(R.string.nfcAvailableEnabled));
        } else {
            toast(getString(R.string.nfcAvailableDisabled));
            finish();
        }
    }

    public void toast(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL, 0, 0);
        toast.show();
    }
}

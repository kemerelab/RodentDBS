package org.kemerelab.rsmcontrol;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.nfc.NdefMessage;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import org.kemerelab.rsmcontrol.R;
import org.ndeftools.Message;
import org.ndeftools.util.activity.NfcTagWriterActivity;
import org.ndeftools.wellknown.TextRecord;

import java.nio.charset.Charset;
import java.util.Locale;

public class NFCWriteActivity extends NfcTagWriterActivity {

    //RSMDevice rsmDevice;
    String NDEFText;

    public enum NFCAvalability {
        UNINITIALIZED, NOT_AVAIALBLE, AVAILABLE_NOT_ENABLED, AVAILABLE_ENABLED
    }

    NFCAvalability nfcAvalability = NFCAvalability.UNINITIALIZED;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nfcwrite);

        Intent intent = getIntent();
        NDEFText = intent.getStringExtra(MainActivity.NDEF_TEXT_TO_BE_WRITTEN);
        TextView t = (TextView) findViewById(R.id.writerStatusText);
        t.setText(NDEFText);

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

    public void updateStatus(NFCAvalability nfcAvalability) {
        TextView t = (TextView) findViewById(R.id.writerStatusText);
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
    }

    public void updateStatus(String message) {
        TextView t = (TextView) findViewById(R.id.statusTextView);
        if ( t!= null) {
            t.setText(getString(R.string.nfcAvailableEnabled));
            t.setTextColor(R.color.LightBlue);
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
    }

    /**
     * NFC feature was found and is currently enabled  - Called in onCreate of NfcReaderActivity
     */

    @Override
    protected void onNfcStateEnabled() {
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
            //toast(getString(R.string.nfcAvailableEnabled));
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
     *
     * Create an NDEF message to be written when a tag is within range.
     *
     * @return the message to be written
     */

    @Override
    protected NdefMessage createNdefMessage() {

        // compose our own message
        Message message = new Message();

        // add a Text Record with the message which is entered
        TextRecord textRecord = new TextRecord();
        textRecord.setText(NDEFText);
        //textRecord.setText(text.getText().toString());
        textRecord.setEncoding(Charset.forName("UTF-8"));
        textRecord.setLocale(Locale.ENGLISH);
        message.add(textRecord);

        return message.getNdefMessage();
    }


    /**
     *
     * Writing NDEF message to tag failed.
     *
     * @param e exception
     */

    @Override
    protected void writeNdefFailed(Exception e) {
        toast(getString(R.string.ndefWriteFailed, e.toString()));
    }

    /**
     *
     * Tag is not writable or write-protected.
     *
     * @param e exception
     */

    @Override
    public void writeNdefNotWritable() {
        toast(getString(R.string.tagNotWritable));
    }

    /**
     *
     * Tag capacity is lower than NDEF message size.
     *
     * @param e exception
     */

    @Override
    public void writeNdefTooSmall(int required, int capacity) {
        toast(getString(R.string.tagTooSmallMessage,  required, capacity));
    }


    /**
     *
     * Unable to write this type of tag.
     *
     */

    @Override
    public void writeNdefCannotWriteTech() {
        toast(getString(R.string.cannotWriteTechMessage));
    }

    /**
     *
     * Successfully wrote NDEF message to tag.
     *
     */

    @Override
    protected void writeNdefSuccess() {
        toast(getString(R.string.ndefWriteSuccess));
    }


    public void toast(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL, 0, 0);
        toast.show();
    }
}

package org.kemerelab.rsmcontrol;

import android.content.Context;
import android.content.res.Resources;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Created by ckemere on 8/18/15.
 */

class RSMDeviceInfoAtom {
    public enum AtomFormatting {DATA, HEADER, STATUS_DATA, STATUS_HEADER};

    public enum AtomAction {NONE, TOGGLE_BUTTON, SPINNER, DIALOG};

    public enum SettingsType {NA,
        DEVICE_ID, ENABLE_STIMULATION, AMPLITUDE, FREQUENCY, PULSE_WIDTH, JITTER_LEVEL
    };


    public String caption;
    public String stringDatum;
    public int currentPosition;
    public SettingsType settingsType;
    public AtomAction atomAction;
    public AtomFormatting atomFormatting;
    public List<String> stringValues;

    public RSMDeviceInfoAtom(String _caption, SettingsType _type, AtomFormatting _formatting,
                             AtomAction _action, String _stringDatum) {
        caption = _caption;
        stringDatum = _stringDatum;
        currentPosition = -1;
        settingsType = _type;
        atomAction = _action;
        atomFormatting = _formatting;
    }

    public RSMDeviceInfoAtom(String _caption, SettingsType _type, List<String> _values, int _currentPosition) {
        caption = _caption;
        stringDatum = "";
        stringValues = _values;
        currentPosition = _currentPosition;
        settingsType = _type;
        atomAction = AtomAction.SPINNER;
        atomFormatting = AtomFormatting.DATA;
    }


    public RSMDeviceInfoAtom(String _caption, String _currentValue) { // Status atom
        caption = _caption;
        stringDatum = _currentValue;
        currentPosition = -1;
        settingsType = SettingsType.NA;
        atomAction = AtomAction.NONE;
        atomFormatting = AtomFormatting.STATUS_DATA;
    }

    public RSMDeviceInfoAtom(String _caption, AtomFormatting _formatting) { // Header
        caption = _caption;
        stringDatum = "";
        currentPosition = -1;
        settingsType = SettingsType.NA;
        atomAction = AtomAction.NONE;
        atomFormatting = _formatting;
    }

}

public class RSMDevice implements Parcelable {

    // Constants
    private int currentSourceRangeResistor;
    public Boolean isValid = false;

    // Device identification information
    private byte[] deviceID;
    private byte firmwareVersion;

    // Stimulation control parameters
    private byte stimulationEnabled; // true or false
    private short stimulationWidth; // length of each phase of biphasic pulse in us
    private short stimulationPeriod; // in us
    private byte stimulationCurrentSetting; // follow formula! actual amplitude = 0.997 / (16 * 47 K) * sA
    private byte jitterLevel; // in us - can be 0, 1, 2, or 3 (corresponding to 0, 1 ms, 2 ms, and 4 ms)

    // Device status information
    private Integer uptime; // system uptime in seconds
    private Integer lastUpdate; // time since last update of data in seconds
    private short batteryVoltage; // battery voltage in value from ADC

    private int[] stimAmplitudeValues;
    private int[] stimFrequencyValues;
    private int[] pulseWidthValues;

    public RSMDevice() {
        String deviceIDStr = "ID00";
        deviceID = deviceIDStr.getBytes(StandardCharsets.UTF_8);
        stimulationEnabled = 0;
        stimulationPeriod = 10000;
        stimulationCurrentSetting = 0;
        stimulationWidth = 60;
        jitterLevel = 0;
        uptime = 0;
        lastUpdate = 0;
        batteryVoltage = 0;
        firmwareVersion = 6;
        isValid = false;
        currentSourceRangeResistor = 47000;
    }

    public RSMDevice(byte[] data) {
        deviceID = new byte[4];
        stimulationEnabled = 0;
        stimulationPeriod = 10000;
        stimulationCurrentSetting = 0;
        stimulationWidth = 0;
        jitterLevel = 0;
        uptime = 0;
        lastUpdate = 0;
        batteryVoltage = 0;
        firmwareVersion = 0;
        currentSourceRangeResistor = 47000;


        isValid = false;
        ByteBuffer buf = ByteBuffer.wrap(data);
        buf.order(ByteOrder.LITTLE_ENDIAN);
        try {
            firmwareVersion = buf.get();
            buf.get(deviceID); // deviceID is size 4!
            stimulationEnabled = buf.get(); // uint8_t
            stimulationPeriod = buf.getShort(); // uint16_t
            if (firmwareVersion == 3)
                    stimulationCurrentSetting = (byte) buf.getShort();
            else
                stimulationCurrentSetting = buf.get(); // uint8_t

            stimulationWidth = buf.getShort(); // uint16_t

            if (firmwareVersion > 4)
                jitterLevel = buf.get(); // uint8_t

            batteryVoltage = buf.getShort();  // uint16_t
            uptime = buf.getInt();  // uint32_t
            lastUpdate = buf.getInt();  // uint32_t
            isValid = true;
        } catch (Exception e) {
            e.printStackTrace(); // buffer underflow, for e.g.
        }
    }


    public byte[] getDeviceInfoAsByteArray() {
        ByteBuffer buf;
        switch (firmwareVersion) {
            case 3:
                buf = ByteBuffer.allocate(22); // hard coded for current data string!
                break;
            case 4:
                buf = ByteBuffer.allocate(21); // hard coded for current data string!
                break;
            case 5:
            default:
                buf = ByteBuffer.allocate(22); // hard coded for current data string!
                break;
        }

        buf.order(ByteOrder.LITTLE_ENDIAN);
        buf.put((byte) firmwareVersion);
        buf.put(deviceID);
        buf.put((byte) stimulationEnabled);
        buf.putShort((short) stimulationPeriod);
        switch (firmwareVersion) {
            case 3:
                buf.putShort((short) stimulationCurrentSetting);
                break;
            case 4:
            case 5:
            default:
                buf.put((byte) stimulationCurrentSetting);
                break;
        }
        buf.putShort((short) stimulationWidth);

        if (firmwareVersion > 4)
            buf.put((byte) jitterLevel);

        buf.putShort((short) 0); // battery voltage will get reset by device
        buf.putInt(0); // uptime will get reset by device
        buf.putInt(0); // lastUpdate will get reset by device

        return buf.array();
    }

    public void enableStimulation() {
        stimulationEnabled = 1;
    }

    public void disableStimulation() {
        stimulationEnabled = 0;
    }

    public void toggleStimulationEnabled() {
        if (stimulationEnabled == 0)
            stimulationEnabled = 1;
        else
            stimulationEnabled = 0;
    }

    public void setStimulationEnabled(Boolean val) {
        if (val)
            stimulationEnabled = 1;
        else
            stimulationEnabled = 0;
    }

    public String getStimulationEnabledState() {
        if (stimulationEnabled == 0)
            return "disabled";
        else
            return "enabled";
    }

    public void setDeviceID(String idString) {
        byte[] idStringBytes = idString.getBytes(StandardCharsets.UTF_8);
        deviceID = Arrays.copyOfRange(idStringBytes,0,4);
    }

    public String getDeviceID() {
        return new String(deviceID);
    }


    public String getAmplitudeString() {
        float amp = 1e6f * 0.997f * stimulationCurrentSetting /
                (16 * currentSourceRangeResistor);
        return String.format("%.0f", amp);

    }

    public int getStimAmplitude() {
        // current stimulation amplitude in microAmps
        return (int)( 1e6f * 0.997f * stimulationCurrentSetting /
                (16 * currentSourceRangeResistor) );
    }

    public float getMaxAmplitude() {
        return ( 0.997f * 127 * 1e6f / (16 * currentSourceRangeResistor) );
    }

    public void setStimCurrent(float amplitude) {
        int cur = (int) ( amplitude * 16.0f * currentSourceRangeResistor / 0.997f / 1e6f );
        if (cur > 127) {
            // log bug. signed int to byte
            stimulationCurrentSetting = 127;
        } else if (cur < 0) {
            stimulationCurrentSetting = 0;
        } else {
            stimulationCurrentSetting = (byte) cur;
        }
    }

    public void setPulseWidth(short value) {
        stimulationWidth = value;
    }

    public short getPulseWidth() {
        return stimulationWidth;
    }

    public float getMinFrequency() {
        return 15.26f; // 1,000,000 / 65,535
    }

    public void setStimPeriod(float freq) {
        if (freq == 0) {
            stimulationEnabled = 0;
            stimulationPeriod = (short) ((int) 60000); // close to max int
        }
        else {
            int per = (int) (1000000.0f / freq);
            stimulationPeriod = (short) per;
        }
    }

    public String getStimFrequencyString() {
        float freq = 1e6f / stimulationPeriod;
        return String.format("%.0f", freq);

    }

    public int getStimFrequency() {
        return (int) (1e6f / stimulationPeriod);
    }

    public int getClosestValueInList(int value, int[] list) {
        int minDistance = Math.abs(value - list[0]);
        int p = 0;
        for (int i = 1; i < list.length; i++) {
            if (Math.abs(value - list[i]) < minDistance) {
                minDistance = Math.abs(value - list[i]);
                p = i;
            }
        }

        if (minDistance != 0) {
            Log.d("getClosest",
                    "value not found in list" + value + ". closest is " + list[p]);
        }

        return p;
    }

    public void setStimAmplitudeFromList(int position) {
        if (position < stimAmplitudeValues.length)
            setStimCurrent(stimAmplitudeValues[position]);
        else
            Log.d("initialization issue!", "empty stimAmplitudeValues list");
    }

    public void setStimFrequencyFromList(int position) {
        if (position < stimFrequencyValues.length)
            setStimPeriod(stimFrequencyValues[position]);
        else
            Log.d("initialization issue!", "empty stimFrequencyValues list");
    }

    public void setPulseWidthFromList(int position) {
        if (position < pulseWidthValues.length)
            setPulseWidth((short) pulseWidthValues[position]);
        else
            Log.d("initialization issue!", "empty pulseWidthValues list");
    }


    // Jitter is special, because the value we set is actually a shift amount.
    //  0 = no jitter, 1 = +/- 0.5 ms, 2 = +/- 1 ms, 3 = +/- 2 ms
    //  the maximum jitter, +/- 2 ms corresponds to a maximum stim frequency of 250 Hz minus epsilon
    //  where epsilon is determined by pulsewidth
    public int getJitterLevel() {
        return jitterLevel;
    }

    public void setJitterLevel(int _jitterLevel) {
        if ((_jitterLevel >= 0) && (_jitterLevel <= 3))
            jitterLevel = (byte) _jitterLevel;
        else
            jitterLevel = 0;
    }


    public String getUptimeString() {
        if (uptime < 0) {
            return "no data";
        }
        else {
            // uptime is in seconds. convert to hr:min:ss
            Integer hr = uptime / 3600;
            Integer min = (uptime - hr * 3600) / 60;
            Integer ss = uptime - hr * 3600 - min * 60;
            return String.format("%d:%02d:%02d", hr, min, ss);
        }
    }

    public String getLastUpdateString() {
        if (lastUpdate > 0) {
            // uptime is in seconds. convert to hr:min:ss
            Integer hr = lastUpdate / 3600;
            Integer min = (lastUpdate - hr * 3600) / 60;
            Integer ss = lastUpdate - hr * 3600 - min * 60;
            return String.format("%d:%02d:%02d", hr, min, ss);
        }
        else
            return "no data";
    }

    public String getBatteryVoltageString() {
        if (batteryVoltage > 0)
            return String.valueOf((int) batteryVoltage * 2500 * 18/5 / 1024);
        else
            return "no data";
    }

    public List<String> addUnitsToValueList(int[] values, String unitLabel) {
        List<String> valueStrings = new ArrayList<String>();
        for (int val : values) {
            valueStrings.add(String.valueOf(val) + " " + unitLabel);
        }
        return valueStrings;
    }

    public List<RSMDeviceInfoAtom> getDeviceInfoList(Context context, boolean includeStatus) {
        Resources res = context.getResources();
        List deviceInfoList = new ArrayList<RSMDeviceInfoAtom>();

        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.deviceInfoHeader),
                RSMDeviceInfoAtom.AtomFormatting.HEADER));

        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.deviceIDlabel),
                RSMDeviceInfoAtom.SettingsType.DEVICE_ID,
                RSMDeviceInfoAtom.AtomFormatting.DATA,
                RSMDeviceInfoAtom.AtomAction.DIALOG,
                getDeviceID()));

        if (includeStatus) {
            deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.firmwareLabel),
                    Byte.toString(firmwareVersion)));
        }

        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimSettingsHeader),
                RSMDeviceInfoAtom.AtomFormatting.HEADER));

        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimEnabledLabel),
                RSMDeviceInfoAtom.SettingsType.ENABLE_STIMULATION,
                RSMDeviceInfoAtom.AtomFormatting.DATA,
                RSMDeviceInfoAtom.AtomAction.TOGGLE_BUTTON,
                getStimulationEnabledState()));

        stimAmplitudeValues = res.getIntArray(R.array.stimAmplitudeValues);
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimAmplitudeLabel),
                RSMDeviceInfoAtom.SettingsType.AMPLITUDE,
                addUnitsToValueList(stimAmplitudeValues, res.getString(R.string.stimAmplitudeUnit)),
                getClosestValueInList(getStimAmplitude(), stimAmplitudeValues)));

        stimFrequencyValues = res.getIntArray(R.array.stimFrequencyValues);
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimFrequencyLabel),
                RSMDeviceInfoAtom.SettingsType.FREQUENCY,
                addUnitsToValueList(stimFrequencyValues, res.getString(R.string.stimFrequencyUnit)),
                getClosestValueInList(getStimFrequency(), stimFrequencyValues)));


        pulseWidthValues = res.getIntArray(R.array.pulseWidthValues);
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.pulseWidthLabel),
                RSMDeviceInfoAtom.SettingsType.PULSE_WIDTH,
                addUnitsToValueList(pulseWidthValues, res.getString(R.string.pulseWidthUnit)),
                getClosestValueInList(getPulseWidth(), pulseWidthValues)));

        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.jitterLabel),
                RSMDeviceInfoAtom.SettingsType.JITTER_LEVEL,
                Arrays.asList(res.getStringArray(R.array.jitter_levels)),
                getJitterLevel()));

        if (includeStatus) {
            deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.deviceStatusHeader),
                    RSMDeviceInfoAtom.AtomFormatting.STATUS_HEADER));
            deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.uptimeLabel),
                    getUptimeString()));
            deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.batteryVoltageLabel),
                    getBatteryVoltageString() + " mV"));
            deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.lastUpdateLabel),
                    getLastUpdateString()));
        }

        return deviceInfoList;
    }

    // PARCELABLE Interface

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeByteArray(deviceID);
        out.writeByte(stimulationEnabled);
        out.writeInt(stimulationPeriod);
        out.writeInt(stimulationCurrentSetting);
        out.writeInt(stimulationWidth);
        out.writeByte(jitterLevel);
        out.writeInt(currentSourceRangeResistor);
    }

    public static final Parcelable.Creator<RSMDevice> CREATOR
            = new Parcelable.Creator<RSMDevice>() {
        public RSMDevice createFromParcel(Parcel in) {
            return new RSMDevice(in);
        }

        public RSMDevice[] newArray(int size) {
            return new RSMDevice[size];
        }
    };

    private RSMDevice(Parcel in) {
        uptime = -1;
        lastUpdate = -1;
        batteryVoltage = -1;
        isValid = false;

        deviceID = new byte[4];

        in.readByteArray(deviceID);
        stimulationEnabled = in.readByte();
        stimulationPeriod = (short) in.readInt();
        stimulationCurrentSetting = (byte) in.readInt();
        stimulationWidth = (short) in.readInt();
        jitterLevel = in.readByte();
        currentSourceRangeResistor = in.readInt();

    }

}

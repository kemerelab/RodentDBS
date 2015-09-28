package org.kemerelab.rsmcontrol;

import android.content.Context;
import android.content.res.Resources;
import android.os.Parcel;
import android.os.Parcelable;

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
    public String caption;
    public String datum;
    public RSMDevice.SettingsType settingsType;
    public int datumValue;
    public boolean isHeader;

    public RSMDeviceInfoAtom(String _caption, String _datum, RSMDevice.SettingsType _settingsType) {
        caption = _caption; datum = _datum; settingsType = _settingsType;
        isHeader = false; datumValue = -1;
    }

    public RSMDeviceInfoAtom(String _caption, int _datumValue, RSMDevice.SettingsType _settingsType) {
        caption = _caption; settingsType = _settingsType; datumValue = _datumValue;
        isHeader = false; datum = "";
    }

    public RSMDeviceInfoAtom(String _caption, RSMDevice.SettingsType _settingsType) {
        caption = _caption; isHeader = true; settingsType = _settingsType;
        datum = ""; datumValue = -1;
    }
}

public class RSMDevice implements Parcelable {

    // Constants
    private int currentSourceRangeResistor;
    public Boolean isValid = false;

    public enum SettingsType {NA, STATUS_ATOM, DEVICE_ID,
        ENABLE_STIMULATION, AMPLITUDE, FREQUENCY, PULSE_WIDTH, JITTER_LEVEL
    };

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
        firmwareVersion = 5;
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
                buf = ByteBuffer.allocate(21); // hard coded for current data string!
                break;
            case 4:
                buf = ByteBuffer.allocate(22); // hard coded for current data string!
                break;
            case 5:
            default:
                buf = ByteBuffer.allocate(23); // hard coded for current data string!
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
            default:
                buf.put((byte) stimulationCurrentSetting);
                break;
        }
        buf.putShort((short) stimulationWidth);
        buf.put(jitterLevel);
        buf.putShort((short) 0); // battery voltage will get reset by device
        buf.putInt(0); // uptime will get reset by device
        buf.putInt(0); // lastUpdate will get reset by device

        return buf.array();
    }

    public String getAmplitudeString() {
        float amp = 1e6f * 0.997f * stimulationCurrentSetting /
                (16 * currentSourceRangeResistor);
        return String.format("%.0f", amp);

    }

    public float getMaxAmplitude() {
        return ( 0.997f * 127 * 1e6f / (16 * currentSourceRangeResistor) );
    }

    public void setStimCurrent(float amplitude) {
        int cur = (int) ( amplitude * 16.0f * currentSourceRangeResistor / 0.997f / 1e6f );
        if (cur > 127) {
            // log bug?
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
            return String.valueOf((int) batteryVoltage * 2500 * 2 / 1024);
        else
            return "no data";
    }

    public List<RSMDeviceInfoAtom> getDeviceInfoList(Context context) {
        List deviceInfoList = new ArrayList<RSMDeviceInfoAtom>();

        Resources res = context.getResources();
        deviceInfoList.add(new RSMDeviceInfoAtom("Device Info", SettingsType.NA));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.deviceIDlabel),
                getDeviceID(),  SettingsType.DEVICE_ID));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.firmwareLabel),
                Byte.toString(firmwareVersion), SettingsType.STATUS_ATOM));

        deviceInfoList.add(new RSMDeviceInfoAtom("Stimulation Settings", SettingsType.NA));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimEnabledLabel),
                getStimulationEnabledState(), SettingsType.ENABLE_STIMULATION));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimAmplitudeLabel),
                getAmplitudeString() + " µV", SettingsType.AMPLITUDE));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.stimFrequencyLabel),
                getStimFrequencyString() + " Hz", SettingsType.FREQUENCY));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.pulseWidthLabel),
                Short.toString(stimulationWidth) + " µs", SettingsType.PULSE_WIDTH));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.jitterLabel),
                getJitterLevel(), SettingsType.JITTER_LEVEL));

        deviceInfoList.add(new RSMDeviceInfoAtom("Device Status", SettingsType.STATUS_ATOM));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.uptimeLabel),
                getUptimeString(), SettingsType.STATUS_ATOM));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.batteryVoltageLabel),
                getBatteryVoltageString() +" mV",  SettingsType.STATUS_ATOM));
        deviceInfoList.add(new RSMDeviceInfoAtom(res.getString(R.string.lastUpdateLabel),
                getLastUpdateString(), SettingsType.STATUS_ATOM));

        return deviceInfoList;
    }


    public void showSettingsDialog(Context context, SettingsType whichSetting) {

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

package org.kemerelab.rsmcontrol;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.style.AlignmentSpan;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by ckemere on 8/18/15.
 */
public class RSMDevice implements Parcelable {


    public byte stimulationEnabled; // true or false
    public short stimulationWidth; // length of each phase of biphasic pulse in us
    public byte[] deviceID;

    public short stimulationPeriod; // in us
    public short stimulationAmplitude; // in uV

    public Integer uptime; // system uptime in seconds
    public Integer lastUpdate; // time since last update of data in seconds
    public short batteryVoltage; // battery voltage in millivolts

    public byte commandVersion;

    public Boolean isValid = false;


    public RSMDevice() {
        String deviceIDStr = "ID00";
        deviceID = deviceIDStr.getBytes(StandardCharsets.UTF_8);
        stimulationEnabled = 0;
        stimulationPeriod = 10000;
        stimulationAmplitude = 0;
        stimulationWidth = 10;
        uptime = 0;
        lastUpdate = 0;
        batteryVoltage = 0;
        commandVersion = 1;
        isValid = false;
    }

    public RSMDevice(byte[] data) {
        deviceID = new byte[4];
        stimulationEnabled = 0;
        stimulationPeriod = 10000;
        stimulationAmplitude = 0;
        stimulationWidth = 0;
        uptime = 0;
        lastUpdate = 0;
        batteryVoltage = 0;
        commandVersion = 0;

        isValid = false;
        ByteBuffer buf = ByteBuffer.wrap(data);
        buf.order(ByteOrder.LITTLE_ENDIAN);
        try {
            commandVersion = buf.get();
            buf.get(deviceID); // deviceID is size 4!
            stimulationEnabled = buf.get();
            stimulationPeriod = buf.getShort();
            stimulationAmplitude = buf.getShort();
            stimulationWidth = buf.getShort();
            batteryVoltage = buf.getShort();
            uptime = buf.getInt();
            lastUpdate = buf.getInt();
            isValid = true;
        } catch (Exception e) {
            e.printStackTrace(); // buffer underflow, for e.g.
        }
    }


    public byte[] getDeviceInfoAsByteArray() {
        ByteBuffer buf = ByteBuffer.allocate(22); // hard coded for current data string!

        buf.order(ByteOrder.LITTLE_ENDIAN);
        buf.put((byte) commandVersion);
        buf.put(deviceID);
        buf.put((byte) stimulationEnabled);
        buf.putShort((short) stimulationPeriod);
        buf.putShort((short) stimulationAmplitude);
        buf.putShort((short) stimulationWidth);
        buf.putShort((short) 0); // battery voltage will get reset by device
        buf.putInt(0); // uptime will get reset by device
        buf.putInt(0); // lastUpdate will get reset by device

        return buf.array();
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

    public String getBatteryVoltageString() {
        if (batteryVoltage > 0)
            return String.valueOf(batteryVoltage) + " mV";
        else
            return "no data";
    }

    // PARCELABLE Interface

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeByteArray(deviceID);
        out.writeByte(stimulationEnabled);
        out.writeInt(stimulationPeriod);
        out.writeInt(stimulationAmplitude);
        out.writeInt(stimulationWidth);
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

        in.readByteArray(deviceID);
        stimulationEnabled = in.readByte();
        stimulationPeriod = (short) in.readInt();
        stimulationAmplitude = (short) in.readInt();
        stimulationWidth = (short) in.readInt();
    }

}

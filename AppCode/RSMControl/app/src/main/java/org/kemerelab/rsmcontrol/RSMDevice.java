package org.kemerelab.rsmcontrol;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by ckemere on 8/18/15.
 */
public class RSMDevice implements Parcelable {


    public Integer stimulationEnabled; // true or false
    public Integer stimulationWidth; // length of each phase of biphasic pulse in us
    public Integer deviceID;

    public Integer stimulationPeriod; // in us
    public Integer stimulationAmplitude; // in uV

    public Integer uptime; // system uptime in seconds
    public Integer lastUpdate; // time since last update of data in seconds
    public Integer batteryVoltage; // battery voltage in millivolts

    public Integer commandVersion;

    public Boolean isValid = false;


    public RSMDevice() {
        deviceID = 0;
        stimulationEnabled = 0;
        stimulationPeriod = 10000000;
        stimulationAmplitude = 0;
        stimulationWidth = 10;
        uptime = 0;
        lastUpdate = 0;
        batteryVoltage = 0;
        commandVersion = 1;
        isValid = false;
    }

    public RSMDevice(String recordString) {
        deviceID = 0;
        stimulationEnabled = 0;
        stimulationPeriod = 10000000;
        stimulationAmplitude = 0;
        stimulationWidth = 0;
        uptime = 0;
        lastUpdate = 0;
        batteryVoltage = 0;
        commandVersion = 1;
        Pattern p = Pattern.compile("RSM(\\d{3});E(\\d{1});I(.{4});P(\\d{5});A(\\d{3});W(\\d{3});B(\\d{2});");
        Matcher m = p.matcher(recordString);
        isValid = false;
        if (m.matches()) {
            commandVersion = Integer.parseInt(m.group(1));
            stimulationEnabled = Integer.parseInt(m.group(2));
            deviceID = Integer.parseInt(m.group(3));
            stimulationPeriod = Integer.parseInt(m.group(4));
            stimulationAmplitude = Integer.parseInt(m.group(5));
            stimulationWidth = Integer.parseInt(m.group(6));
            batteryVoltage = Integer.parseInt(m.group(7)) * 100;

            isValid = true;
        }
    }


    public String getDeviceInfoAsNDEFString() {
        //return String.format("RSM%03d;E%d;I%04d;P%05d;A%03d;W%03d;B%02d;L%02d;U%07d;",
        return String.format("RSM%03d;E%d;I%04d;P%05d;A%03d;W%03d;B%02d;",
                commandVersion, stimulationEnabled, deviceID, stimulationPeriod, stimulationAmplitude,
                stimulationWidth, 0);
    }

    public String getLastUpdateString() {
        if (lastUpdate > 0)
            return lastUpdate.toString() + " s";
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
            return batteryVoltage.toString() + " mV";
        else
            return "no data";
    }

    // PARCELABLE Interface

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(deviceID);
        out.writeInt(stimulationEnabled);
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

        deviceID = in.readInt();
        stimulationEnabled = in.readInt();
        stimulationPeriod = in.readInt();
        stimulationAmplitude = in.readInt();
        stimulationWidth = in.readInt();
    }

}

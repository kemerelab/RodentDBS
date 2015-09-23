## Overview

The firmware runs on an MSP430G2553. This is a low power TI microcontroller. 

## Estimating power consumption 

**MSP430**
In sleep mode, we can only go to LPM0 because the stimulation counter is using 
the DCO to generate a 1 MHz SMCLK. Unclear is how often we are in active mode.

   - Active power  = ~70 uA
   - Sleep (LPM0) = ~35 uA

**Battery voltage sampling**
The MSP430 ADC10 will consume something like 800 uA when it's turned on, but we sample only
once per minute, so expect not to have significant contribution to power consumption.

**I2C current**
I2C pullup resistors should not be causing signifant current use as I/Os should be high-Z
except during data transmission.

**Power LED**
We're using PWM and flashing, so we're active for only PWM rate * On time.  An on PWM level of
20% and a 25 ms flash every second implies a scaling of 0.2 * 0.025 = 0.005. If VDD is 3 V, and
the on voltage of the (red) LED is 1.8 V, with a 1 K current limiting resistor, our on current
is 1.2 mA. So expect consumption is 6 uA.

   - LED power = 6 uA

 **DS4432**
 Unfortunately, the current source has to be on all the time because turning on and off would
 essentially take the entire inter-pulse period (due to I2C transmission time). Hard to tell
 from the data sheet exactly what power will be. Max is 150 uA, but we expect a bit less.

   - DS4432 power = 100 uA

**TS3A4751** (switch matrix)
The switch matrix is fairly efficient, though it, like the DS4432, is powered constantly. We
expect supply current to be close to 3.6 V table values.

  - TS3A4751 power = 1 uA

**RF430** (NFC power)
The RF430 will actually work much better with a 3V supply than a 3.6V supply because the NFC
field provides a 3V power source. In that regime, using NFC to read data is essentially free,
and only during periods when the device is communicating with the MSP430 does it consume power.
When we read data, we are read/writing in the following pattern:
  (1) check status (i.e., address  + 2 bytes = 2 bytes)
  (2) potentially clear interrupts (address + 4 bytes = 5 bytes)
  (3) disable RF (address + 4 bytes = 5 bytes)
  (4) read stim parameters (address + 2 + 6 = 9 bytes)
  (5) re-enable RF (5 bytes)
This is a total of ~26 bytes worth of I2C at 400 kHz, which corresponds to 520 us.
Writing is the same except that the data transmission stage is 21 instead of 6 bytes, so 820
us. Currently, the firmware is set to write 3x/second and read once per 30 seconds or on
demand. So that suggests that the device will be active for 2.5 ms per second. Active power is
250 uA and "passive" power is 40 uA (though we could lower to 15 uA by disabling RF). So we
expect passive mode to dominate, or ~40 uA consumption.

  - RF430 power = 40 uA

  - Total power assuming active mode dominates for MS430 = 217 uA
  - Total power assuming passive mode dominates for MS430 = 182 uA

According to Energizer, we can expect a CR2032 to drop below 2.7 V (the rated minimum voltage
for the DS4432 current source) after about 1000 hours at a constant draw of 0.19 mA. So we
expect our battery life to be ~900 hours, which would be a month. However, in practice, we're
seeing something less than that!?

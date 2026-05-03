# Description
The project describes how to program the IAW5AF ECU from Magneti Marelli in order to read/write the EEPROM data without any specific proprietary interface or software.

# Prerequisites
To be able to program the ECU you will need the following:
- A Windows 10 PC;
- [uVision](https://www.keil.com/demo/eval/c166.htm) from Keil (Infineon C167/C166 compiler);
- [ST10Flasher](https://community.st.com/t5/others-hardware-and-software/how-can-i-get-the-st10-flasher-tool/td-p/315885) tool. The tool is quite old and the link that I include in this repo is from the ST community forum;
- Serial reader (PuTTY, Arduino, Gtkterm, etc..);
- [MultiECUScan](https://www.multiecuscan.net/) to verify that the K-Line connection is up and running. The software provides a way to communicate with the legacy ECU software which presents a standard diagnostic interface.

# USB Interface
The K-Line bus is quite similar to a UART one with the TX/RX lines connected on the same line. <b>Unfortunately</b>, you cannot use a standard Arduino Uno serial converter (due to protocol strict timing constraints). I used an FTDI FT2232H which can be used with a 1ms buffer (Option on the Windows Driver).<br/><br/>
To connect to the ECU, a USB to K-Line interface is needed. I built my own by assembling a 12V-3.3V level shifter on breadboard based on the following schematic. <br/><br/>
<b>IMPORTANT: BUILD THE INTERFACE ONLY IF YOU KNOW WHAT YOU ARE DOING!</b><br/>
<b>ADAPT THE RESISTORS BASED ON YOU TRANSISTORS AND DESIRED INTERFACE VOLTAGE LEVELS!</b>
![Level Shifter Converter](https://europe1.discourse-cdn.com/arduino/original/4X/0/4/3/0433816896ccd02ba86fbcfa92af04248ffae1e4.jpeg)

# Programming
In order to read/write the EEPROM data, we will use two different Infineon C167 programs that I already prepared. The reader outputs via K-Line (A simple serial terminal that can be read at 9600 bauds from any Serial Reader) the EEPROM dump. The writer, will write the data inside the array to the EEPROM.

## ECU Connection
Follow the Pinout to connect the ECU to the correct pins.
<p align="center">
<img src="https://lh6.googleusercontent.com/proxy/ZROcrXGsiUa-snhQA9uBNVBf1pWCzuiQiQWqoyZZStozLlt6i5Xaxo_vK3x8p_Up1Js4-Aa4geNbFRJdPzdx_MaEd4qnMYCTLIjF4BA_cqK1B73QEa9leum7_w"/>
</p>


## Boot the ECU in Programming Mode
To boot the ECU in programming mode, you have to ground a specific pin inside the board. The following shows which pin you must connect to the ECU. Be aware that the pin is not exposed on the connector. Therefore, you will have to open the ECU at your own risk!<br/>
This [Guide](https://obdrus.ru/f/instruction-iaw-59f_5af_800_.pdf) illustrates how to open the ECU to reach the boostrap pin. <br/><br/>
Once the boostrap pin is reachable, ground it and power on the ECU. You will have a lower current consumption that the nominal operations and the ECU will result in programming mode. To confirm that, open the ST10Flasher tool and it will automatically connect to the ECU. A correct connection will show the microcontroller (ST10F269) and its clock frequency (which should be around 24.9MHz). At this point dump the current firmware and save it (it will be required to restore the ECU original status).
# Arduino M0 Pro Bootloader
## Description
This is the alternative bootloader for Arduino M0 Pro.
Is able to update firmware via USB Native port and UART using Stk500 protocol.  
Source project comes from: https://github.com/arduino/ArduinoCore-samd/tree/master/bootloaders/mzero

This bootloader implements the tap on Reset button. By pressing this button, the board will reset and stay in bootloader, waiting for communication on either USB or USART by 8 second. After this time bootloader try start user application.
The USB port in use is the USB Native port, close to the Reset button. 

The USART in use is labelled respectively RX/TX.
Communication parameters are a baudrate at 9600, 8bits of data, no parity and 1 stop bit.
## Burn instruction
1. Download and install [Atmel Studio](http://www.microchip.com/mplab/avr-support/atmel-studio-7)
2. Download binary file from repository: **Bootloader/Bootloader_D21.bin**
3. Connect the Arduino M0 Pro board to PC via **Programming** USB Port
4. Start Atmel Studio, click **Tools** → **Device Programming**
5. In the Device Programming dialog box, select **EDBG** in the Tool drop-down box, **ATSAMD21G18A** in the Device drop-down box and **SWD** in the Interface drop-down box
6. Click the **Apply** button and then **Read** button under *Device signature* to make sure that you can connect to the ATSAMD21G18A microcontroller
7. Click **Fuses**  in the left pane of the dialog box

    * Change the fuse value   

            USER_WORD_0.NVMCTRL_BOOTPROT : 0x07
    * Click **Program** button
8. Click **Memories** in the left pane of the dialog box

    * Browse to the **Bootloader_D21.bin** file using the **"..."** button
    * Click the **Program** button
9. Close Atmel Studio
10. Start the Arduino IDE and load a sketch to test that the bootloader is working

## Lily AVR bootloader

Fully customizable bootloader for AVR family MCU.
Including Pyhton application software for easily uploading firmware. 

## Use the bootloader

Create a main.cpp file for your target and add the bootloader library files:

    /* Bootloader usage example */
    
    #include <avr/pgmspace.h>               // Required for using PROGMEM.
    #include "uart.h"                       // Using the UART interface from the bootloader library.
    #include "bootloader.h"                 // Using the bootloader library.

    #define SYS_CLOCK   16000000            // The crystal/clock frequency (16MHz in this case).
    #define BAUD_RATE   115200              // The baudrate for the UART.

    // Device name and module name are just for showing in the GUI what module is detected.
    // You can use any name you like. These are stored in FLASH so they do not occupy RAM.
    const char DEVICE_NAME[] PROGMEM = "ATmega328P";
    const char MODULE_NAME[] PROGMEM = "My module";


    int main(void) {
        Uart uart(SYS_CLOCK, BAUD_RATE);    // Initialize the UART.

	    Bootloader bootloader(              // Initialize the bootloader.
	        SYS_CLOCK,                      // Used for time outs and LED blinking speed.
	        PORTB5,                         // Pin for the LED (0 for no LED).
	        &DDRB,                          // DDR for the LED is connected (0 for no LED).
	        &PORTB,                         // Port for the LED is connected (0 for no LED).
	        &uart,                          // Pointer to the UART.
	        DEVICE_NAME,                    // Flash address to device name.
	        MODULE_NAME                     // Flash address to module name.
	    );
        
        while (1) {
            bootloader.process_events();    // Process bootloader events, like timer overflow and UART.
        }
    }

The target can have a LED connected to a pin (in this case pin 5 of PORTB). The LED indicates the state of the bootloader:

* Blinking at 2Hz: bootloader running and waiting for activation command throught UART.
* Short flashing at 1Hz: bootloader is active, until a deactivation command is received or a reset.

If you do not whish to have a LED blinking, set the pin, DDR and PORT values to 0 (zero).

When the bootloader starts it waits for an activation command for 3 seconds. If not received, the main program in the FLASH will be started.
If there is no program in the flash, the bootloader will stop and the LED will be off. If the activation command is received, the bootloader is activated.
Then the flash can be programmed page by page. After programming, the flash can be read for verification.
When sending the deactivation command, the bootloader is reset and will start the main program.


2023 - LilyTronics.nl (https://lilytronics.nl)

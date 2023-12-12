/*
 * General bootloader based on Arduino Uno platform with ATmega238 running at 16MHz.
 *
 * With fuses set the boot flash size to 2048 words (start address 0x3800).
 * In the project properties, go to tool chain. Under AVR/GNU linker, go to memory settings.
 * Add the flash segment: .text=0x3800.
 * This should place the boot loader code in the boot flash section.
 * Note that the compiler is not checking if the bootloader is too big.
 * Check in the output: Program Memory Usage : xx bytes
 * Note this is in bytes. The size of the boot section is in words (2048 words = 4096 bytes).
 * You can also check the .hex output file. The code should start at address 0x7000 (byte address).
 *
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "interfaces/uart.h"
#include "bootloader.h"


#define SYS_CLOCK               16000000
#define BAUD_RATE               115200

const char DEVICE_NAME[] PROGMEM = "ATmega328P";
const char MODULE_NAME[] PROGMEM = "Arduino Uno R3";


int main(void) {
    Uart uart(SYS_CLOCK, BAUD_RATE);

    Bootloader bootloader(
        SYS_CLOCK,
        PORTB5,
        &DDRB,
        &PORTB,
        &uart,
		DEVICE_NAME,
		MODULE_NAME
    );

    while (1) {
        bootloader.process_events();
    }
}

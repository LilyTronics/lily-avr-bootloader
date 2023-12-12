/*
 * Bootloader for Arduino Uno R3
 */ 

#include <avr/pgmspace.h>
#include "uart.h"
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

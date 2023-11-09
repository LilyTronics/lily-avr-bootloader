/*
 * General bootloader based on Arduino Uno platform with ATmega238 running at 16MHz.
 */

#include <avr/io.h>
#include "bootloader.h"


int main(void) {
    Bootloader bootloader(
        16000000,
        PORTB5,
        &DDRB,
        &PORTB
    );

    while (1) {
        bootloader.process_events();
    }
}

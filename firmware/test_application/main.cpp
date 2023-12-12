/*
 * Test Application
 * Generate some FLASH code for programming using the bootloader.
 *
 */


#define F_CPU   16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "random_data.h"


int main(void)
{

    DDRB |= (1 << PORTB5);
    PORTB |= (1 << PORTB5);

    while (1) {
        for (int i = 0; i < DATA_SIZE; i++) {
            char c = pgm_read_byte(DATA + i);

            for (int j = 0; j < 8; j++) {
                if (c & 0x01) {
                    PORTB |= (1 << PORTB5);
                }
                else {
                    PORTB &= ~(1 << PORTB5);
                }
                _delay_ms(100);
                c = c >> 1;
            } // j

        } // i

    } // while

}


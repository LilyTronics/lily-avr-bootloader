/*
 * General bootloader based on Arduino Uno platform with ATmega238 running at 16MHz.
 */

#include <avr/io.h>
#include "interfaces/uart.h"
#include "bootloader.h"


#define SYS_CLOCK               16000000
#define BAUD_RATE               115200
#define UART_TIMEOUT            1
#define UART_TIME_OUT_COUNTS    (SYS_CLOCK / TIMER_DIV) * UART_TIMEOUT


int main(void) {
    Uart uart;
    uart.init(SYS_CLOCK, BAUD_RATE, UART_TIME_OUT_COUNTS);

    Bootloader bootloader(
        SYS_CLOCK,
        PORTB5,
        &DDRB,
        &PORTB,
        &uart
    );

    while (1) {
        bootloader.process_events();
    }
}

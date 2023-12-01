/*
 * UART interface code.
 */

#include "uart.h"


Uart::Uart(uint32_t sys_clock, uint32_t baud_rate) {
    UCSR0A = (1 << U2X0);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    UBRR0 = (uint16_t)((sys_clock / (8L * baud_rate)) - 1);
}


uint8_t Uart::get_data_byte(uint8_t& data_byte) {
    uint8_t result = 0;

    if (UCSR0A & (1 << RXC0)) {
        data_byte = UDR0;
        result = 1;
    }

    return result;
}


void Uart::send_response(uint8_t response_data[]) {
    uint16_t size = (uint16_t) (response_data[2] << 8);
    size += response_data[3];
    size += 4;

    for (uint8_t i = 0; i < size; i++) {
        while (!(UCSR0A & (1 << UDRE0)));        UDR0 = response_data[i];    }
}

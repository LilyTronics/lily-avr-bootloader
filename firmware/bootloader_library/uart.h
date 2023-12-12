/*
 * UART interface (base on class Interface).
 */

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include "interface.h"


class Uart : public Interface {
    public:
        Uart(uint32_t sys_clock, uint32_t baud_rate);
        uint8_t get_data_byte(uint8_t& data_byte);
        void send_response(uint8_t response_data[]);

};

#endif /* UART_H_ */

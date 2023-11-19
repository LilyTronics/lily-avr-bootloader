/*
 * Interface header
 * Base class for all interfaces that can be used for communication.
 * Such as UART, TWI, SPI, etc.
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <avr/io.h>


class Interface {
    public:
        virtual uint8_t get_data_byte(uint8_t& data_byte) {
            data_byte = 0;
            return 0;
        }
        virtual void send_response(uint8_t response_data[], uint8_t size) {}

};

#endif /* INTERFACE_H_ */

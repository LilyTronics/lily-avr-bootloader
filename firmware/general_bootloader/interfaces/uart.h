/*
 * UART interface (base on class Interface).
 */

#ifndef UART_H_
#define UART_H_

#include "interface.h"


#define STATE_IDLE          0
#define STATE_COMMAND       1
#define STATE_COUNTER_HIGH  2
#define STATE_COUNTER_LOW   3
#define STATE_DATA          4

#define START_OF_PACKET     0x01


class Uart : public Interface {
    public:
        void init(uint32_t sys_clock, uint32_t baud_rate, uint16_t time_out_counts);
        uint8_t is_command_received(void);
        void send_response(PacketData packetData);

    private:
        uint8_t m_receiver_state;
        void send_byte(uint8_t data_byte);

};

#endif /* UART_H_ */

/*
 * UART interface code.
 */

#include "uart.h"


void Uart::init(uint32_t sys_clock, uint32_t baud_rate, uint16_t time_out_counts) {
    Interface::init(sys_clock, time_out_counts);

    UCSR0A = (1 << U2X0);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    UBRR0 = (uint16_t)((sys_clock / (8L * baud_rate)) - 1);

    m_receiver_state = STATE_IDLE;
}


uint8_t Uart::is_command_received(void) {
    uint8_t data_byte;

    if (UCSR0A & (1 << RXC0)) {
        // Byte received, clear timeout counter and process byte
        m_interface_timeout_counter = 0;
        data_byte = UDR0;

        switch (m_receiver_state) {
            case STATE_IDLE:
                if (data_byte == START_OF_PACKET) {
                    m_receiver_state++;
                }
                break;

            case STATE_COMMAND:
                m_command_data.command = data_byte;
                m_command_data.n_data_bytes = 0;
                m_receiver_state++;
                break;

            case STATE_COUNTER_HIGH:
                m_command_data.n_data_bytes = data_byte << 8;
                m_receiver_state++;
                break;

            case STATE_COUNTER_LOW:
                m_command_data.n_data_bytes += data_byte;
                m_receiver_state++;
                m_is_command_received = 1;
                break;

            case STATE_DATA:
                if (m_command_data.n_data_bytes > 0) {
                    send_byte(data_byte);
                }
                else {
                    m_receiver_state = STATE_IDLE;
                }
                break;
        }
    }
    else {
        // No bytes received, check for timeout
        if (m_interface_timeout_counter >= m_interface_timeout_counts) {
            // Reset receiver state, to get back in sync with the host
            m_receiver_state = STATE_IDLE;
            m_interface_timeout_counter = 0;
            m_is_command_received = 0;
        }
    }

    return m_is_command_received;
}


void Uart::send_response(PacketData packetData) {
    send_byte(START_OF_PACKET);
    send_byte(packetData.command);
    send_byte((uint8_t) (packetData.n_data_bytes >> 8));
    send_byte((uint8_t) (packetData.n_data_bytes & 0xFF));
}


void Uart::send_byte(uint8_t data_byte) {
    while (!(UCSR0A & (1 << UDRE0)));    UDR0 = data_byte;}

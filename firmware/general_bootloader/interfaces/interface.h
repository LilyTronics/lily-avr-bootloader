/*
 * Interface header
 * Base class for all interfaces that can be used for communication.
 * Such as UART, TWI, SPI, Ethernet, etc.
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_


struct PacketData {
    uint8_t command;
    uint16_t n_data_bytes;
};


class Interface {
    public:
        virtual void init(uint32_t sys_clock, uint16_t time_out_counts) {
            m_is_command_received = 0;
            m_interface_timeout_counter = 0;
            m_interface_timeout_counts = time_out_counts;
        }

        virtual void increment_timeout_counter() {
            m_interface_timeout_counter++;
        }

        virtual uint8_t is_command_received(void) {
            return m_is_command_received;
        }

        virtual PacketData get_command_data(void) {
            return m_command_data;
        }

        virtual void clear_command_received(void) {
            m_is_command_received = 0;
        }

        virtual void send_response(PacketData packetData) {}

    protected:
        uint16_t m_interface_timeout_counter;
        uint16_t m_interface_timeout_counts;
        uint8_t m_is_command_received;
        PacketData m_command_data;

};

#endif /* INTERFACE_H_ */

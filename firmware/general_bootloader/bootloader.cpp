/*
 * Bootloader code.
 */

#include "bootloader.h"


Bootloader::Bootloader(uint32_t sys_clock, uint8_t led_pin, volatile uint8_t* led_ddr,
                       volatile uint8_t* led_port, Interface* interface) {
    TCCR0B = (1 << CS02);
    TIFR0  = (1 << TOV0);

    *led_ddr |= (1 << led_pin);
    *led_port |= (1 << led_pin);

    m_led_pin = led_pin;
    m_led_port = led_port;
    m_led_blink_counts = sys_clock / LED_BLINK_DIV;
    m_led_flash_on_counts = sys_clock / LED_FLASH_ON_DIV;
    m_led_flash_off_counts = (sys_clock / TIMER_DIV) - m_led_flash_on_counts;

    m_is_bootloader_active = 0;
    m_led_mode = LED_MODE_BLINK;
    m_led_counter = 0;

    m_boot_timeout_counts = (sys_clock / TIMER_DIV) * BOOT_TIMEOUT;
    m_boot_timeout_counter = 0;

    m_com_timeout_counts = (sys_clock / TIMER_DIV) * COM_TIMEOUT;
    m_com_timeout_counter = 0;

    m_interface = interface;

    m_com_state = COM_STATE_IDLE;
}


/**********
 * Public *
 **********/

void Bootloader::process_events(void) {
    uint8_t data_byte;
    static uint8_t command;
    static uint16_t n_data_bytes;

    // Check timer overflow
    if (TIFR0 & (1 << TOV0)) {
        TIFR0 |= (1 << TOV0);

        m_led_counter++;
        m_com_timeout_counter++;

        if (m_is_bootloader_active) {
            m_boot_timeout_counter = 0;
        }
        else {
            m_boot_timeout_counter++;
            if (m_boot_timeout_counter >= m_boot_timeout_counts) {
                run_main_application();
            }
        }

        process_led();
    }

    if (m_com_state < COM_STATE_PROCESS) {
        // Check for data from interface
        if (m_interface->get_data_byte(data_byte)) {
            m_com_timeout_counter = 0;

            switch (m_com_state) {
                case COM_STATE_IDLE:
                    if (data_byte == START_OF_PACKET) {
                        m_com_state++;
                    }
                    break;

                case COM_STATE_COMMAND:
                    command = data_byte;
                    n_data_bytes = 0;
                    m_com_state++;
                    break;

                case COM_STATE_COUNTER_HIGH:
                    n_data_bytes = data_byte << 8;
                    m_com_state++;
                    break;

                case COM_STATE_COUNTER_LOW:
                    n_data_bytes += data_byte;
                    m_com_state++;
                    process_command(command, n_data_bytes);
                    break;
            }
        }
        else {
            // No bytes received, check for timeout
            if (m_com_timeout_counter >= m_com_timeout_counts) {
                // Reset receiver state, to get back in sync with the host
                m_com_state = COM_STATE_IDLE;
                m_com_timeout_counter = 0;
            }
        }
    }
}


void Bootloader::process_command(uint8_t command, uint16_t n_data_bytes) {
    uint8_t response_data[4];
    uint8_t command_finished = 0;

    switch (command) {
        case CMD_ACTIVATE:
            m_led_mode = LED_MODE_FLASH_ON;
            m_led_counter = 0;
            m_is_bootloader_active = 1;
            response_data[0] = START_OF_PACKET;
            response_data[1] = ~command;
            response_data[2] = 0;
            response_data[3] = 0;
            command_finished = 1;
            break;

        case CMD_DEACTIVATE:
            m_led_mode = LED_MODE_BLINK;
            m_led_counter = 0;
            m_is_bootloader_active = 0;
            response_data[0] = START_OF_PACKET;
            response_data[1] = ~command;
            response_data[2] = 0;
            response_data[3] = 0;
            command_finished = 1;
            break;

        default:
            command_finished = 1;
            break;
    }

    if (command_finished) {
        m_interface->send_response(response_data, 4);
        m_com_state = COM_STATE_IDLE;
        m_com_timeout_counter = 0;
    }
}


/***********
 * Private *
 ***********/

void Bootloader::process_led(void) {
    if (m_led_mode == LED_MODE_BLINK && m_led_counter == m_led_blink_counts) {
        m_led_counter = 0;
        *m_led_port ^= (1 << m_led_pin);
    }
    if (m_led_mode == LED_MODE_FLASH_ON && m_led_counter == m_led_flash_on_counts) {
        m_led_mode = LED_MODE_FLASH_OFF;
        *m_led_port &= ~(1 << m_led_pin);
        m_led_counter = 0;
    }
    if (m_led_mode == LED_MODE_FLASH_OFF && m_led_counter == m_led_flash_off_counts) {
        m_led_mode = LED_MODE_FLASH_ON;
        *m_led_port |= (1 << m_led_pin);
        m_led_counter = 0;
    }
}


void Bootloader::run_main_application(void) {
    *m_led_port &= ~(1 << m_led_pin);
    // TODO: jump to main application
    while (1);
}

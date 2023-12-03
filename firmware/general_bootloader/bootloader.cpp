/*
 * Bootloader code.
 */

#include "bootloader.h"
#include "device_info.h"
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>


// Default program when flash is empty
// start:  RJMP start   (endless loop)
uint8_t default_prog[] = { 0xFF, 0xCF };


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

    if (is_flash_empty()) {
        // Program default prog
        program_page(0, default_prog, 2);
    }

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
    uint8_t response_data[MAX_PACKET_SIZE];
    uint8_t command_finished = 0;

    response_data[0] = START_OF_PACKET;
    response_data[1] = ~command;

    switch (command) {
        case CMD_ACTIVATE:
            m_led_mode = LED_MODE_FLASH_ON;
            m_led_counter = 0;
            m_is_bootloader_active = 1;
            response_data[2] = 0;
            response_data[3] = 0;
            command_finished = 1;
            break;

        case CMD_DEACTIVATE:
            m_led_mode = LED_MODE_BLINK;
            m_led_counter = 0;
            m_is_bootloader_active = 0;
            response_data[2] = 0;
            response_data[3] = 0;
            command_finished = 1;
            break;

        case CMD_VERSION:
            if (m_is_bootloader_active) {
                response_data[2] = 0;
                response_data[3] = 1;
                response_data[4] = BOOTLOADER_VERSION;
                command_finished = 1;
            }
            break;

        case CMD_DEVICE_NAME:
            if (m_is_bootloader_active) {
                uint16_t data_size = strlen(DEVICE_NAME);
                response_data[2] = HIGH(data_size);
                response_data[3] = LOW(data_size);
                for (uint16_t i = 0; i < data_size; i++) {
                    response_data[4 + i] = DEVICE_NAME[0 + i];
                }
                command_finished = 1;
            }
            break;

        case CMD_MODULE_NAME:
            if (m_is_bootloader_active) {
                uint16_t data_size = strlen(MODULE_NAME);
                response_data[2] = HIGH(data_size);
                response_data[3] = LOW(data_size);
                for (uint16_t i = 0; i < data_size; i++) {
                    response_data[4 + i] = MODULE_NAME[0 + i];
                }
                command_finished = 1;
            }
            break;

        default:
            command_finished = 1;
            break;
    }

    if (command_finished) {
        m_interface->send_response(response_data);
    }

    if (command_finished || !m_is_bootloader_active) {
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
    asm("JMP 0");
}


uint8_t Bootloader::is_flash_empty(void) {
    // Check first 8 bytes at least one of them should not be 0xFF when programmed
    for (uint8_t i = 0; i < 8; i++) {
        if (pgm_read_byte(i) != 0xFF) {
            return 0;
        }
    }

    return 1;
}


void Bootloader::program_page(uint32_t page_address, uint8_t* page_data, uint8_t data_count) {
    uint8_t sreg = SREG;
    cli();

    eeprom_busy_wait();
    boot_page_erase(page_address);
    boot_spm_busy_wait();

    // Page is filled per two bytes
    for (uint16_t i = 0; i < SPM_PAGESIZE; i += 2) {
        // Little-endian word, padding with empty data in case buffer is not a complete page
        uint16_t data_word = 0;
        if (data_count > 0) {
            data_word |= *page_data++;
            data_count--;
            if (data_count > 0) {
                data_word |= (*page_data++) << 8;
                data_count--;
            }
            else {
                data_word |= 0xFF00;
            }
        }
        else {
            data_word = 0xFFFF;
        }
        boot_page_fill(page_address + i, data_word);
    }

    boot_page_write(page_address);
    boot_spm_busy_wait();
    boot_rww_enable();

    SREG = sreg;
}

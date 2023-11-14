/*
 * Bootloader code.
 */

#include <avr/io.h>
#include "bootloader.h"
#include "interfaces/interface.h"


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

    m_boot_timeout_counts = (sys_clock / TIMER_DIV) * BOOT_TIME_OUT;
    m_boot_timeout_counter = 0;

    m_interface = interface;
}


/**********
 * Public *
 **********/

void Bootloader::process_events(void) {
    // Check timer overflow
    if (TIFR0 & (1 << TOV0)) {
        TIFR0 |= (1 << TOV0);

        m_led_counter++;

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
        m_interface->increment_timeout_counter();
    }

    if (m_interface->is_command_received()) {
        process_command();
        m_interface->clear_command_received();
    }
}


void Bootloader::process_command() {
    PacketData packetData = m_interface->get_command_data();

    switch (packetData.command) {
        case CMD_ACTIVATE:
            m_led_mode = LED_MODE_FLASH_ON;
            m_led_counter = 0;
            m_is_bootloader_active = 1;
            packetData.command = ~packetData.command;
            packetData.n_data_bytes = 0;
            m_interface->send_response(packetData);
            break;

        case CMD_DEACTIVATE:
            m_led_mode = LED_MODE_BLINK;
            m_led_counter = 0;
            m_is_bootloader_active = 0;
            packetData.command = ~packetData.command;
            packetData.n_data_bytes = 0;
            m_interface->send_response(packetData);
            break;

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

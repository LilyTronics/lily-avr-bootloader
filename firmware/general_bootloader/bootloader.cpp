/*
 * Bootloader code.
 */

#include <avr/io.h>
#include "bootloader.h"


Bootloader::Bootloader(uint32_t sys_clock, uint8_t led_pin, volatile uint8_t* led_ddr,
                       volatile uint8_t* led_port) {
    TCCR0B = (1 << CS02);
    TIFR0  = (1 << TOV0);

    *led_ddr |= (1 << led_pin);
    *led_port |= (1 << led_pin);

    m_led_pin = led_pin;
    m_led_port = led_port;
    m_led_blink_counts = sys_clock / LED_BLINK_DIV;
    m_led_flash_on_counts = sys_clock / LED_FLASH_ON_DIV;
    m_led_flash_off_counts = (sys_clock / TIMER_DIV) - m_led_flash_on_counts;

    m_led_mode = LED_MODE_BLINK;
    m_led_counter = 0;
}


void Bootloader::process_events(void) {
    if (TIFR0 & (1 << TOV0)) {
        TIFR0 |= (1 << TOV0);

        m_led_counter++;

        process_led();
    }
}


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

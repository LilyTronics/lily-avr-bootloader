/*
 * Bootloader header.
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <avr/io.h>
#include "interfaces/interface.h"


class Bootloader {
    public:
        Bootloader(
            uint32_t sys_clock,
            uint8_t led_pin,
            volatile uint8_t* led_ddr,
            volatile uint8_t* led_port,
            Interface* interface
        );
        void process_events(void);
        void process_command(uint8_t command, uint16_t n_data_bytes);

    private:
        uint8_t m_is_bootloader_active;
        uint8_t m_led_pin;
        volatile uint8_t* m_led_port;
        uint8_t m_led_mode;
        uint16_t m_led_counter;
        uint16_t m_led_blink_counts;
        uint16_t m_led_flash_on_counts;
        uint16_t m_led_flash_off_counts;
        uint16_t m_boot_timeout_counter;
        uint16_t m_boot_timeout_counts;
        uint16_t m_com_timeout_counter;
        uint16_t m_com_timeout_counts;
        Interface* m_interface;
        uint8_t m_com_state;
        void process_led(void);
        void run_main_application(void);
        void program_page(uint32_t page_address, uint8_t* page_data, uint8_t data_count);
        uint8_t is_flash_empty(void);

};


// Timer 0 (8 bit) is used for:
// - blinking/flashing LED
// - boot loader timeout
// - interface timeout
#define TIMER_PRESCALER         256
#define TIMER_OVERFLOW          256
#define TIMER_DIV               TIMER_PRESCALER / TIMER_OVERFLOW

#define DIV_2HZ_ON_OFF          4
#define DIV_1HZ_ON              8

// LED definitions
#define LED_MODE_BLINK          1
#define LED_MODE_FLASH_ON       2
#define LED_MODE_FLASH_OFF      3
#define LED_BLINK_DIV           TIMER_DIV / DIV_2HZ_ON_OFF
#define LED_FLASH_ON_DIV        TIMER_DIV / DIV_1HZ_ON

// Timeouts
#define BOOT_TIMEOUT            3
#define COM_TIMEOUT             2

// Communication states
#define COM_STATE_IDLE          0
#define COM_STATE_COMMAND       1
#define COM_STATE_COUNTER_HIGH  2
#define COM_STATE_COUNTER_LOW   3
#define COM_STATE_PROCESS       4

// Bootloader commands
#define START_OF_PACKET         0x01
#define CMD_ACTIVATE            0x02
#define CMD_DEACTIVATE          0x03


#endif /* BOOTLOADER_H_ */

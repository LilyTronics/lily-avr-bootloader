/*
 * Bootloader header.
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

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
        void process_command(void);

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
        Interface* m_interface;

        void process_led(void);
        void run_main_application(void);
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

// Boot timeout
#define BOOT_TIME_OUT           3

// Bootloader commands
#define CMD_ACTIVATE            0x02
#define CMD_DEACTIVATE          0x03

#endif /* BOOTLOADER_H_ */

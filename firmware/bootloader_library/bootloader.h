/*
 * Bootloader header.
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <avr/io.h>
#include "interface.h"


class Bootloader {

    public:
        // Constructor
        Bootloader(
            uint32_t sys_clock,
            uint8_t led_pin,
            volatile uint8_t* led_ddr,
            volatile uint8_t* led_port,
            Interface* interface,
			const char *device_name,
			const char *module_name
        );
        // Process events
        void process_events(void);

};


// The only place where the bootloader version should be defined
#define BOOTLOADER_VERSION      1

// Flash size depending on the device (FLASHEND comes from the device io include file)
#define FLASH_SIZE				(uint16_t)(((uint32_t)FLASHEND + 1) / 1024)

// The amount of bytes the bootloader needs
#define BOOT_SIZE               4096


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

#define START_OF_PACKET         0x01
#define ERROR_CODE              0xFF

#define LOW(x)                  (uint8_t) (x & 0xFF)
#define HIGH(x)                 (uint8_t) ((x >> 8) & 0xFF)

#endif /* BOOTLOADER_H_ */

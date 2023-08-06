/*
 * Bootloader header.
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_


class Bootloader {
    public:
        Bootloader(
            uint32_t sys_clock,
            uint8_t led_pin,
            volatile uint8_t* led_ddr,
            volatile uint8_t* led_port
        );
        void process_events(void);

    private:
        uint8_t m_led_pin;
        volatile uint8_t* m_led_port;
        uint8_t m_led_mode;
        uint16_t m_led_counter;
        uint32_t m_led_blink_counts;
        uint32_t m_led_flash_on_counts;
        uint32_t m_led_flash_off_counts;

        void process_led(void);
};


// Timer 0 (8 bit) is used for:
// - blinking/flashing LED
#define TIMER_PRESCALER         256L
#define TIMER_OVERFLOW          256L
#define TIMER_DIV               TIMER_PRESCALER / TIMER_OVERFLOW

#define DIV_2HZ_ON_OFF          4L
#define DIV_1HZ_ON              8L

// LED definitions
#define LED_MODE_BLINK          1
#define LED_MODE_FLASH_ON       2
#define LED_MODE_FLASH_OFF      3
#define LED_BLINK_DIV           TIMER_DIV / DIV_2HZ_ON_OFF
#define LED_FLASH_ON_DIV        TIMER_DIV / DIV_1HZ_ON


#endif /* BOOTLOADER_H_ */

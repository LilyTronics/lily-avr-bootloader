/*
 * Bootloader code.
 */

#include "bootloader.h"
#include "device_info.h"
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>


uint8_t m_led_pin;
volatile uint8_t* m_led_port;
uint16_t m_led_blink_counts;
uint16_t m_led_flash_on_counts;
uint16_t m_led_flash_off_counts;
uint8_t m_is_bootloader_active;
uint8_t m_led_mode;
uint16_t m_led_counter;
uint16_t m_boot_timeout_counts;
uint16_t m_boot_timeout_counter;
uint16_t m_com_timeout_counts;
uint16_t m_com_timeout_counter;
Interface* m_interface;
uint8_t m_rx_data[PAGE_SIZE + 4];
uint8_t m_rx_index;
uint8_t m_tx_data[PAGE_SIZE + 4];
uint32_t m_active_page_address;

void process_led(void);
uint8_t is_flash_empty(void);
void program_page(uint32_t page_address, uint8_t* page_data, uint8_t data_count);
void run_main_application(void);

// Command functions
uint8_t activate(void);
uint8_t deactivate(void);
uint8_t get_version(void);
uint8_t get_device_name(void);
uint8_t get_module_name(void);

uint8_t get_boot_size(void);
uint8_t get_flash_size(void);
uint8_t get_page_size(void);

uint8_t set_page_address(void);
uint8_t read_page(void);
uint8_t write_page(void);


// Bootloader command entry
typedef struct {
    uint8_t command;
    uint8_t (*function)(void);
} COMMAND_ENTRY;

COMMAND_ENTRY m_commands[] = {
    { 0x02, activate         },          // Activate the boot loader

    // All command below here require an activated bootloader

    { 0x03, deactivate       },         // Deactivate the boot loader
    { 0x04, get_version      },         // Get the bootloader version

    { 0x10, get_device_name  },         // Get the device name
    { 0x11, get_module_name  },         // Get the module name

    { 0x20, get_boot_size    },         // Get the size of the flash in kB
    { 0x21, get_flash_size   },         // Get the size of the boot section in bytes
    { 0x22, get_page_size    },         // Get the size of the page in bytes

    { 0x30, set_page_address },         // Sets the page address
    { 0x31, read_page        },         // Reads data from the page
    { 0x32, write_page       },         // Write data to the page

    { 0x00, 0               }           // End of list
};

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

    m_rx_index = 0;

    m_active_page_address = 0;

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
    uint16_t size;
    COMMAND_ENTRY *p;
    uint8_t result = 0;

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

    // Check for data from interface
    if (m_interface->get_data_byte(data_byte)) {
        // Check for valid data:
        // - no data yet and start of packet is received
        // - already data available
        if ((m_rx_index == 0 && data_byte == START_OF_PACKET) || m_rx_index > 0) {
            m_com_timeout_counter = 0;
            m_rx_data[m_rx_index] = data_byte;
            m_rx_index++;

            // Check if the packet is complete
            if (m_rx_index >= 4) {
                size = (uint16_t) ((m_rx_data[2] << 8) + m_rx_data[3] + 4);
                if (m_rx_index == size) {
                    // Complete package received

                    // Prepare response
                    m_tx_data[0] = START_OF_PACKET;
                    m_tx_data[1] = ~m_rx_data[1];
                    m_tx_data[2] = 0;
                    m_tx_data[3] = 0;

                    // Process command
                    p = &m_commands[0];
                    if (m_rx_data[1] < 3 || m_is_bootloader_active) {
                        while (p->command > 0) {
                            if (p->command == m_rx_data[1]) {
                                result = p->function();
                                break;
                            }
                            p++;
                        }
                    }

                    // Command not found or error from function, send error
                    if (p->command == 0 || result == 0) {
                        m_tx_data[1] = ERROR_CODE;
                    }

                    // Send response
                    m_interface->send_response(m_tx_data);

                    // Reset and wait for new data
                    m_rx_index = 0;
                    m_com_timeout_counter = 0;
                }
            }
        }
    }
    else {
        // No bytes received, check for timeout
        if (m_com_timeout_counter >= m_com_timeout_counts) {
            // Reset receiver index, to get back in sync with the host
            m_rx_index = 0;
            m_com_timeout_counter = 0;
        }
    }
}


/***********
 * Private *
 ***********/

void process_led(void) {
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


void run_main_application(void) {
    *m_led_port &= ~(1 << m_led_pin);
    asm("JMP 0");
}


uint8_t is_flash_empty(void) {
    // Check first 8 bytes at least one of them should not be 0xFF when programmed
    for (uint8_t i = 0; i < 8; i++) {
        if (pgm_read_byte(i) != 0xFF) {
            return 0;
        }
    }
    return 1;
}


void program_page(uint32_t page_address, uint8_t* page_data, uint8_t data_count) {
    uint8_t sreg = SREG;
    cli();

    eeprom_busy_wait();
    boot_page_erase(page_address);
    boot_spm_busy_wait();

    // Page is filled per two bytes
    for (uint16_t i = 0; i < PAGE_SIZE; i += 2) {
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


/*********************
 * Command functions *
 *********************/

uint8_t activate(void) {
    m_led_mode = LED_MODE_FLASH_ON;
    m_led_counter = 0;
    m_is_bootloader_active = 1;
    return 1;
}


uint8_t deactivate(void) {
    m_led_mode = LED_MODE_BLINK;
    m_led_counter = 0;
    m_is_bootloader_active = 0;
    return 1;
}


uint8_t get_version(void) {
    m_tx_data[3] = 1;
    m_tx_data[4] = BOOTLOADER_VERSION;
    return 1;
}


uint8_t get_device_name(void) {
    uint16_t data_size = strlen(DEVICE_NAME);
    m_tx_data[2] = HIGH(data_size);
    m_tx_data[3] = LOW(data_size);
    for (uint16_t i = 0; i < data_size; i++) {
        m_tx_data[4 + i] = DEVICE_NAME[0 + i];
    }
    return 1;
}


uint8_t get_module_name(void) {
    uint16_t data_size = strlen(MODULE_NAME);
    m_tx_data[2] = HIGH(data_size);
    m_tx_data[3] = LOW(data_size);
    for (uint16_t i = 0; i < data_size; i++) {
        m_tx_data[4 + i] = MODULE_NAME[0 + i];
    }
    return 1;
}


uint8_t get_boot_size(void) {
    m_tx_data[3] = 2;
    m_tx_data[4] = HIGH(BOOT_SIZE);
    m_tx_data[5] = LOW(BOOT_SIZE);
    return 1;
}


uint8_t get_flash_size(void) {
    m_tx_data[3] = 2;
    m_tx_data[4] = HIGH(FLASH_SIZE);
    m_tx_data[5] = LOW(FLASH_SIZE);
    return 1;
}


uint8_t get_page_size(void) {
     m_tx_data[3] = 2;
     m_tx_data[4] = HIGH(PAGE_SIZE);
     m_tx_data[5] = LOW(PAGE_SIZE);
     return 1;
}


uint8_t set_page_address(void) {
    m_active_page_address = (
        ((uint32_t) m_rx_data[4] << 24) +
        ((uint32_t) m_rx_data[5] << 16) +
        ((uint32_t) m_rx_data[6] <<  8) +
         (uint32_t) m_rx_data[7]
    );
    return 1;
}


uint8_t read_page(void) {
    return 0;
}


uint8_t write_page(void) {
    return 0;
}

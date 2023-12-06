/*
 * device_info.h
 *
 * This contains all info of the device.
 */


#ifndef DEVICE_INFO_H_
#define DEVICE_INFO_H_

#define SYS_CLOCK               16000000
#define BAUD_RATE               115200

#define FLASH_SIZE              32          // KB (32 * 1024)
#define PAGE_SIZE               128         // Bytes
#define BOOT_SIZE               2048        // Bytes

const char DEVICE_NAME[] = "ATmega328P";
const char MODULE_NAME[] = "Arduino Uno R3";

#endif /* DEVICE_INFO_H_ */

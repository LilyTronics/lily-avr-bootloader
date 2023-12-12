# Bootloader library

These are the file shared through all the boot loader projects.

## Device setup:

With fuses set the boot flash size to 2048 words (start address 0x3800).
In the project properties, go to tool chain. Under AVR/GNU linker, go to memory settings.
Add the flash segment: .text=0x3800.
This should place the boot loader code in the boot flash section.
Note that the compiler is not checking if the bootloader is too big.
Check in the output: Program Memory Usage : xx bytes
Note this is in bytes. The size of the boot section is in words (2048 words = 4096 bytes).
You can also check the .hex output file. The code should start at address 0x7000 (byte address).

## How to use the library

1. Create a new project (like the Arduino Uno 3 project)

2. In the project properties:
   - C   compiler directories: add the bootloader library folder (relative)
   - C++ compiler directories: add the bootloader library folder (relative)
   - Linker memory settings  : add a directive: .text=0x3800 (start address of the bootloader)
   
3. Add only the required cpp files from the bootloader library files as LINKED items to the project.


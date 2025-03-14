#ifndef RAM_H
#define RAM_H

#include "stdint.h"
#include "stdio.h"

#define RAM_SIZE_WORDS 16384

class RAM
{

public:
    RAM();
    ~RAM();

    // Store a word in RAM
    void store_word(uint32_t address, uint32_t data);

    // Store a halfword in RAM
    void store_halfword(uint32_t address, uint16_t data);

    // Store a byte in RAM
    void store_byte(uint32_t address, uint8_t data);

    // Load a word from RAM
    uint32_t load_word(uint32_t address);

    // Load a halfword from RAM
    uint16_t load_halfword(uint32_t address);

    // Load a byte from RAM
    uint8_t load_byte(uint32_t address);

    // Load a memory image from an Intel HEX file
    int load_memory_ihex(char *filename);

    // Dump memory to stdout in Intel HEX format
    void dump_memory_ihex(uint32_t start_address, uint32_t end_address);

    // Dump memory to a file in Intel HEX format
    void dump_memory_ihex(char *filename, uint32_t start_address, uint32_t end_address);

private:
    uint32_t memory[RAM_SIZE_WORDS];
};

#endif // RAM_H

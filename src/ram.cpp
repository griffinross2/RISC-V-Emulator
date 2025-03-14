// Simulates a simple RAM loaded from an Intel HEX file.

#include "ram.h"

#include <stdio.h>
#include <string.h>
#include <trace.h>
#include <regex>

RAM::RAM()
{
    // Initialize memory to zero
    memset(memory, 0, sizeof(memory));
}

RAM::~RAM()
{
}

void RAM::store_word(uint32_t address, uint32_t data)
{
    TRACE(TRACE_LEVEL_DEBUG, "Storing 0x%08X at 0x%08X\n", data, address);
    memory[address / 4] = data;
}

void RAM::store_halfword(uint32_t address, uint16_t data)
{
    TRACE(TRACE_LEVEL_DEBUG, "Storing 0x%04X at 0x%08X\n", data, address);
    uint32_t word_address = address / 4;
    uint32_t mem_data = memory[word_address];
    if (address % 4 == 0)
    {
        mem_data = (mem_data & 0xFFFF0000) | data;
    }
    else
    {
        mem_data = (mem_data & 0x0000FFFF) | (data << 16);
    }
    memory[word_address] = mem_data;
}

void RAM::store_byte(uint32_t address, uint8_t data)
{
    TRACE(TRACE_LEVEL_DEBUG, "Storing 0x%02X at 0x%08X\n", data, address);
    uint32_t word_address = address / 4;
    uint32_t mem_data = memory[word_address];
    uint32_t shift = (address % 4) * 8;
    mem_data = (mem_data & ~(0xFF << shift)) | (data << shift);
    memory[word_address] = mem_data;
}

uint32_t RAM::load_word(uint32_t address)
{
    TRACE(TRACE_LEVEL_DEBUG, "Loading 0x%08X from 0x%08X\n", memory[address / 4], address);
    return memory[address / 4];
}

uint32_t RAM::load_instruction(uint32_t address)
{
    TRACE(TRACE_LEVEL_DEBUG, "Loading instruction 0x%08X from 0x%08X\n", memory[address / 4], address);
    return memory[address / 4];
}

uint16_t RAM::load_halfword(uint32_t address)
{
    uint32_t word_address = address / 4;
    uint32_t mem_data = memory[word_address];
    if (address % 4 == 0)
    {
        TRACE(TRACE_LEVEL_DEBUG, "Loading 0x%04X from 0x%08X\n", mem_data & 0xFFFF, address);
        return mem_data & 0xFFFF;
    }
    else
    {
        TRACE(TRACE_LEVEL_DEBUG, "Loading 0x%04X from 0x%08X\n", (mem_data >> 16) & 0xFFFF, address);
        return (mem_data >> 16) & 0xFFFF;
    }
}

uint8_t RAM::load_byte(uint32_t address)
{
    uint32_t word_address = address / 4;
    uint32_t mem_data = memory[word_address];
    uint32_t shift = (address % 4) * 8;
    TRACE(TRACE_LEVEL_DEBUG, "Loading 0x%02X from 0x%08X\n", (mem_data >> shift) & 0xFF, address);
    return (mem_data >> shift) & 0xFF;
}

int RAM::load_memory_ihex(char *filename)
{
    // Open file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        TRACE(TRACE_LEVEL_ERROR, "Unable to open load file %s\n", filename);
        return -1;
    }

    char linechar[256];
    while (fgets(linechar, sizeof(linechar), file) != NULL)
    {
        TRACE(TRACE_LEVEL_DEBUG, "Processing ihex line: %s\n", linechar);

        // Use regex to parse the line
        // Group 1: Byte count
        // Group 2: Address
        // Group 3: Record type
        // Group 4: Data
        // Group 5: Checksum
        std::regex ihex_regex(R"rgx(:([0-9A-Fa-f]{2})([0-9A-Fa-f]{4})([0-9A-Fa-f]{2})([0-9A-Fa-f]{0,})([0-9A-Fa-f]{2})[\r|\n|\r\n]*)rgx");
        std::smatch ihex_match;
        std::string line(linechar);
        if (std::regex_match(line, ihex_match, ihex_regex))
        {
            // Group 1: Byte count
            int byte_count = std::stoi(ihex_match[1].str(), nullptr, 16);
            // Group 2: Address
            int address = std::stoi(ihex_match[2].str(), nullptr, 16);
            // Group 3: Record type
            int record_type = std::stoi(ihex_match[3].str(), nullptr, 16);
            // Group 4: Data
            std::string data = ihex_match[4].str();
            // Group 5: Checksum
            int checksum = std::stoi(ihex_match[5].str(), nullptr, 16);

            // Calculate expected checksum
            uint8_t expected_checksum = (uint8_t)(byte_count + (address >> 8) + (address & 0xFF) + record_type);
            for (int i = 0; i < byte_count; i++)
            {
                expected_checksum += (uint8_t)std::stoi(data.substr(i * 2, 2), nullptr, 16);
            }
            expected_checksum = ~expected_checksum + 1;

            // Check checksum
            if (expected_checksum != checksum)
            {
                TRACE(TRACE_LEVEL_ERROR, "RAM init checksum mismatch\n");
                fclose(file);
                return -1;
            }

            // Process record
            switch (record_type)
            {
            case 0:
                // Data record
                for (int i = 0; i < byte_count; i += 1)
                {
                    uint32_t word = std::stoi(data.substr(i * 2, 2), nullptr, 16);
                    int tlevel_save = trace_level;
                    TRACE_SET(TRACE_LEVEL_NONE);
                    store_byte(address + i, word);
                    TRACE_SET(tlevel_save);
                }
                break;
            case 1:
                // End of file record
                fclose(file);
                return 0;
            default:
                TRACE(TRACE_LEVEL_ERROR, "Unknown record type %d\n", record_type);
                fclose(file);
                return -1;
            }
        }
    }

    fclose(file);
    TRACE(TRACE_LEVEL_WARNING, "Unexpected end of hex file\n");
    return 0;
}

void RAM::dump_memory_ihex(uint32_t start_address, uint32_t end_address)
{
    int tlevel_save = trace_level;
    TRACE_SET(TRACE_LEVEL_NONE);

    // Dump memory to stdout in Intel HEX format (word at a time)
    for (uint32_t address = start_address; address <= end_address; address += 4)
    {
        // Skip if word is zero
        if (load_word(address) == 0)
        {
            continue;
        }

        // Calculate byte count
        int byte_count = 4;

        // Calculate checksum
        uint8_t checksum = (uint8_t)(byte_count + ((address >> 8) & 0xFF) + (address & 0xFF));
        for (int i = 0; i < byte_count; i++)
        {
            checksum += load_byte(address + i);
        }
        checksum = ~checksum + 1;

        // Print record
        printf(":%02X%04X00", byte_count, address);
        printf("%08X", load_word(address));
        printf("%02X\n", checksum);
    }

    // Print end of file record
    printf(":00000001FF\n");

    TRACE_SET(tlevel_save);
}

void RAM::dump_memory_ihex(char *filename, uint32_t start_address, uint32_t end_address)
{
    // Open file
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        TRACE(TRACE_LEVEL_ERROR, "Unable to open dump file %s\n", filename);
        return;
    }

    int tlevel_save = trace_level;
    TRACE_SET(TRACE_LEVEL_NONE);

    // Dump memory to stdout in Intel HEX format (word at a time)
    for (uint32_t address = start_address; address <= end_address; address += 4)
    {
        // Skip if word is zero
        if (load_word(address) == 0)
        {
            continue;
        }

        // Calculate byte count
        int byte_count = 4;

        // Calculate checksum
        uint8_t checksum = (uint8_t)(byte_count + ((address >> 8) & 0xFF) + (address & 0xFF));
        for (int i = 0; i < byte_count; i++)
        {
            checksum += load_byte(address + i);
        }
        checksum = ~checksum + 1;

        // Print record
        fprintf(file, ":%02X%04X00", byte_count, address);
        fprintf(file, "%08X", load_word(address));
        fprintf(file, "%02X\n", checksum);
    }

    // Print end of file record
    fprintf(file, ":00000001FF\n");

    TRACE_SET(tlevel_save);

    fclose(file);
}
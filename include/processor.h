#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "ram.h"
#include "stdint.h"
#include "register_file.h"

class Processor
{

public:
    Processor(RAM *ram, uint32_t start_address);
    ~Processor();

    // Reset the processor
    void reset(uint32_t start_address);

    // Execute a single instruction
    void execute_instruction();

    // Dump the state of the processor
    void dump_state();

    // Check if the processor is halted
    bool is_halted();

private:
    // General purpose registers
    RegisterFile registers;

    // Program counter
    uint32_t pc;

    // Memory
    RAM *ram;

    // Halt flag
    bool halt;
};

#endif // PROCESSOR_H

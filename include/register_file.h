#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H

#include <stdint.h>

class RegisterFile
{
public:
    RegisterFile();
    ~RegisterFile();

    // Reset
    void reset();

    // Set a register
    void set_reg(int reg, uint32_t data);

    // Get a register
    uint32_t get_reg(int reg);

    // Dump the state of the register file
    void dump_state();

private:
    uint32_t registers[31];
};

#endif // REGISTER_FILE_H

#include "register_file.h"

#include <string.h>
#include "trace.h"

RegisterFile::RegisterFile()
{
    // Initialize registers to zero
    reset();
}

RegisterFile::~RegisterFile()
{
}

void RegisterFile::reset()
{
    // Initialize registers to zero
    memset(registers, 0, sizeof(registers));
}

void RegisterFile::set_reg(int reg, uint32_t data)
{
    if (reg > 31)
    {
        TRACE(TRACE_LEVEL_ERROR, "Register File: Invalid register %d\n", reg);
        return;
    }

    if (reg == 0)
    {
        return;
    }

    TRACE(TRACE_LEVEL_DEBUG, "Register File: Setting register %d to 0x%08X\n", reg, data);
    registers[reg - 1] = data;
}

uint32_t RegisterFile::get_reg(int reg)
{
    if (reg > 31)
    {
        TRACE(TRACE_LEVEL_ERROR, "Register File: Invalid register %d\n", reg);
        return 0;
    }

    if (reg == 0)
    {
        return 0;
    }

    TRACE(TRACE_LEVEL_DEBUG, "Register File: Getting register %d with value 0x%08X\n", reg, registers[reg - 1]);
    return registers[reg - 1];
}

void RegisterFile::dump_state()
{
    for (int i = 1; i < 32; i++)
    {
        if (i % 4 == 0)
        {
            printf("\n");
        }

        printf("x%02d: 0x%08X  ", i, registers[i - 1]);
    }

    printf("\n");
}
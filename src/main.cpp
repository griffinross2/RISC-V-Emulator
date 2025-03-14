#include <stdio.h>
#include "ram.h"
#include "processor.h"

int main()
{
    RAM ram;
    Processor processor(&ram, 0x00000000);

    // Load memory image
    ram.load_memory_ihex("meminit.hex");

    // Execute instructions
    while (!processor.is_halted())
    {
        processor.execute_instruction();
    }

    // Dump processor state
    processor.dump_state();

    // Dump memory image
    ram.dump_memory_ihex("memsim.hex", 0x00000000, RAM_SIZE_WORDS * 4 - 4);

    return 0;
}
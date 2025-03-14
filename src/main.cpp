#include <stdio.h>
#include "ram.h"

int main()
{
    RAM ram;

    ram.load_memory_ihex("meminit.hex");

    ram.dump_memory_ihex("memsim.hex", 0x00000000, RAM_SIZE_WORDS * 4 - 4);

    return 0;
}
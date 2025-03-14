#ifndef ALU_H
#define ALU_H

#include <stdint.h>
#include "control.h"

// Execute an ALU operation
uint32_t alu_execute(uint32_t a, uint32_t b, aluop_t aluop, bool mul_signed_a, bool mul_signed_b, bool mul_half);

#endif // ALU_H

#include "alu.h"

uint32_t alu_execute(uint32_t a, uint32_t b, aluop_t aluop, bool mul_signed_a, bool mul_signed_b, bool mul_half)
{
    switch (aluop)
    {
    case ALUOP_ADD:
        return (uint32_t)((int32_t)a + (int32_t)b);
    case ALUOP_SUB:
        return (uint32_t)((int32_t)a - (int32_t)b);
    case ALUOP_AND:
        return a & b;
    case ALUOP_OR:
        return a | b;
    case ALUOP_XOR:
        return a ^ b;
    case ALUOP_SLT:
        return (int32_t)a < (int32_t)b;
    case ALUOP_SLTU:
        return a < b;
    case ALUOP_SLL:
        return a << (b & 0x1F);
    case ALUOP_SRL:
        return a >> (b & 0x1F);
    case ALUOP_SRA:
        return (int32_t)a >> (b & 0x1F);
    case ALUOP_MUL:
    {
        int32_t a_s = (int32_t)a;
        int32_t b_s = (int32_t)b;
        if (mul_signed_a)
        {
            a_s = (a_s > 0) ? a_s : -a_s;
        }
        if (mul_signed_b)
        {
            b_s = (b_s > 0) ? b_s : -b_s;
        }
        int64_t result = (int64_t)a_s * (int64_t)b_s;
        if (mul_half)
        {
            return (uint32_t)(result >> 32);
        }
        else
        {
            return (uint32_t)result;
        }
    }
    default:
        return 0;
    }
}
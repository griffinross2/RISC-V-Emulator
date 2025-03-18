#include "control.h"
#include <string.h>

#include "trace.h"

void control(control_t *control, uint32_t instruction)
{
    control->halt = false;
    control->mem_read = false;
    control->mem_write = false;
    control->mem_to_reg = false;
    control->alu_op = ALUOP_ADD;
    control->branch = false;
    control->branch_pol = false;
    control->jump = false;
    control->mul_signed_a = false;
    control->mul_signed_b = false;
    control->mul_half = false;
    control->rd = 0;
    control->rs1 = 0;
    control->rs2 = 0;
    control->imm = 0;
    control->alu_a_src = false;
    control->alu_b_src = false;

    // Decode instruction
    int opcode = instruction & 0x7F;
    switch (opcode)
    {
    case OP_RTYPE:
    {
        rtype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.funct3 = (funct3_r_t)((instruction & FUNCT3_MASK) >> FUNCT3_SHIFT);
        inst.rs1 = (instruction & RS1_MASK) >> RS1_SHIFT;
        inst.rs2 = (instruction & RS2_MASK) >> RS2_SHIFT;
        inst.funct7 = (funct7_r_t)((instruction & FUNCT7_MASK) >> FUNCT7_SHIFT);

        control->rd = inst.rd;
        control->rs1 = inst.rs1;
        control->rs2 = inst.rs2;

        switch (inst.funct3)
        {
        case ADD_SUB_MUL:
            switch (inst.funct7)
            {
            case ADD_SRL:
                TRACE(TRACE_LEVEL_DEBUG, "ADD x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_ADD;
                break;
            case SUB_SRA:
                TRACE(TRACE_LEVEL_DEBUG, "SUB x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_SUB;
                break;
            case MULT:
                TRACE(TRACE_LEVEL_DEBUG, "MUL x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_MUL;
                control->mul_signed_a = true;
                control->mul_signed_b = true;
                control->mul_half = false;
                break;
            }
            break;
        case AND:
            TRACE(TRACE_LEVEL_DEBUG, "AND x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
            control->alu_op = ALUOP_AND;
            break;
        case OR:
            TRACE(TRACE_LEVEL_DEBUG, "OR x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
            control->alu_op = ALUOP_OR;
            break;
        case XOR:
            TRACE(TRACE_LEVEL_DEBUG, "XOR x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
            control->alu_op = ALUOP_XOR;
            break;
        case SLT_MULHSU:
            switch (inst.funct7)
            {
            case 0:
                TRACE(TRACE_LEVEL_DEBUG, "SLT x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_SLT;
                break;
            case MULT:
                TRACE(TRACE_LEVEL_DEBUG, "MULHSU x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_MUL;
                control->mul_signed_a = true;
                control->mul_signed_b = false;
                control->mul_half = true;
                break;
            }
            break;
        case SLTU_MULHU:
            switch (inst.funct7)
            {
            case 0:
                TRACE(TRACE_LEVEL_DEBUG, "SLTU x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_SLTU;
                break;
            case MULT:
                TRACE(TRACE_LEVEL_DEBUG, "MULHU x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_MUL;
                control->mul_signed_a = false;
                control->mul_signed_b = false;
                control->mul_half = true;
                break;
            }
            break;
        case SLL_MULH:
            switch (inst.funct7)
            {
            case 0:
                TRACE(TRACE_LEVEL_DEBUG, "SLL x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_SLL;
                break;
            case MULT:
                TRACE(TRACE_LEVEL_DEBUG, "MULH x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_MUL;
                control->mul_signed_a = true;
                control->mul_signed_b = true;
                control->mul_half = true;
                break;
            }
            break;
        case SRL_SRA:
            switch (inst.funct7)
            {
            case ADD_SRL:
                TRACE(TRACE_LEVEL_DEBUG, "SRL x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_SRL;
                break;
            case SUB_SRA:
                TRACE(TRACE_LEVEL_DEBUG, "SRA x%02d, x%02d x%02d\n", control->rd, control->rs1, control->rs2);
                control->alu_op = ALUOP_SRA;
                break;
            }
            break;
        default:
            TRACE(TRACE_LEVEL_ERROR, "Illegal Instruction 0x%08X\n", instruction);
            control->halt = true;
            break;
        }
        break;
    }

    case OP_ITYPE:
    {
        itype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.funct3 = (funct3_i_t)((instruction & FUNCT3_MASK) >> FUNCT3_SHIFT);
        inst.rs1 = (instruction & RS1_MASK) >> RS1_SHIFT;
        inst.imm = (instruction & IMM_I_MASK) >> IMM_I_SHIFT;

        control->rd = inst.rd;
        control->rs1 = inst.rs1;
        // Sign extend immediate
        uint32_t imm = inst.imm;
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000;
        }
        control->imm = imm;

        control->alu_b_src = true;

        switch (inst.funct3)
        {
        case ADDI:
            TRACE(TRACE_LEVEL_DEBUG, "ADDI x%02d, x%02d, %d\n", control->rd, control->rs1, (int32_t)control->imm);
            control->alu_op = ALUOP_ADD;
            break;
        case SLTI:
            TRACE(TRACE_LEVEL_DEBUG, "SLTI x%02d, x%02d, %d\n", control->rd, control->rs1, (int32_t)control->imm);
            control->alu_op = ALUOP_SLT;
            break;
        case SLTIU:
            TRACE(TRACE_LEVEL_DEBUG, "SLTIU x%02d, x%02d, %d\n", control->rd, control->rs1, control->imm);
            control->alu_op = ALUOP_SLTU;
            break;
        case XORI:
            TRACE(TRACE_LEVEL_DEBUG, "XORI x%02d, x%02d, %08X\n", control->rd, control->rs1, control->imm);
            control->alu_op = ALUOP_XOR;
            break;
        case ORI:
            TRACE(TRACE_LEVEL_DEBUG, "ORI x%02d, x%02d, %08X\n", control->rd, control->rs1, control->imm);
            control->alu_op = ALUOP_OR;
            break;
        case ANDI:
            TRACE(TRACE_LEVEL_DEBUG, "ANDI x%02d, x%02d, %08X\n", control->rd, control->rs1, control->imm);
            control->alu_op = ALUOP_AND;
            break;
        case SLLI:
            TRACE(TRACE_LEVEL_DEBUG, "SLLI x%02d, x%02d, %d\n", control->rd, control->rs1, control->imm);
            control->alu_op = ALUOP_SLL;
            break;
        case SRLI_SRAI:
            if (inst.imm & 0x400)
            {
                TRACE(TRACE_LEVEL_DEBUG, "SRAI x%02d, x%02d, %d\n", control->rd, control->rs1, control->imm & 0x1F);
                control->alu_op = ALUOP_SRA;
            }
            else
            {
                TRACE(TRACE_LEVEL_DEBUG, "SRLI x%02d, x%02d, %d\n", control->rd, control->rs1, control->imm);
                control->alu_op = ALUOP_SRL;
            }
            break;
        default:
            TRACE(TRACE_LEVEL_ERROR, "Illegal Instruction 0x%08X\n", instruction);
            control->halt = true;
            break;
        }
        break;
    }

    case OP_LUI:
    {
        utype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.imm = (instruction & IMM_U_MASK) >> IMM_U_SHIFT;

        control->rd = inst.rd;
        control->imm = inst.imm << 12;
        control->alu_b_src = true;

        TRACE(TRACE_LEVEL_DEBUG, "LUI x%02d, 0x%08X\n", control->rd, control->imm);

        break;
    }

    case OP_AUIPC:
    {
        utype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.imm = (instruction & IMM_U_MASK) >> IMM_U_SHIFT;

        control->rd = inst.rd;
        control->imm = inst.imm << 12;
        control->alu_a_src = true;
        control->alu_b_src = true;

        TRACE(TRACE_LEVEL_DEBUG, "AUIPC x%02d, 0x%08X\n", control->rd, control->imm);

        break;
    }

    case OP_STYPE:
    {
        stype_t inst;
        inst.opcode = opcode;
        inst.rs1 = (instruction & RS1_MASK) >> RS1_SHIFT;
        inst.rs2 = (instruction & RS2_MASK) >> RS2_SHIFT;
        inst.funct3 = (funct3_s_t)((instruction & FUNCT3_MASK) >> FUNCT3_SHIFT);
        inst.imm5 = (instruction & IMM5_S_MASK) >> IMM5_S_SHIFT;
        inst.imm7 = (instruction & IMM7_S_MASK) >> IMM7_S_SHIFT;

        control->rs1 = inst.rs1;
        control->rs2 = inst.rs2;

        // Compose immediate
        uint32_t imm = inst.imm7 << 5;
        imm |= inst.imm5;

        // Sign extend immediate
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000;
        }

        control->imm = imm;
        control->alu_b_src = true;

        switch (inst.funct3)
        {
        case SW:
            TRACE(TRACE_LEVEL_DEBUG, "SW x%02d, %d(x%02d)\n", control->rs2, (int32_t)control->imm, control->rs1);
            control->mem_write = true;
            break;
        }

        break;
    }

    case OP_LD_ITYPE:
    {
        ld_itype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.funct3 = (funct3_ld_i_t)((instruction & FUNCT3_MASK) >> FUNCT3_SHIFT);
        inst.rs1 = (instruction & RS1_MASK) >> RS1_SHIFT;
        inst.imm = (instruction & IMM_I_MASK) >> IMM_I_SHIFT;

        control->rd = inst.rd;
        control->rs1 = inst.rs1;
        // Sign extend immediate
        uint32_t imm = inst.imm;
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000;
        }
        control->imm = imm;

        control->alu_b_src = true;

        switch (inst.funct3)
        {
        case LW:
            TRACE(TRACE_LEVEL_DEBUG, "LW x%02d, %d(x%02d)\n", control->rd, (int32_t)control->imm, control->rs1);
            control->mem_read = true;
            control->mem_to_reg = true;
            break;
        default:
            TRACE(TRACE_LEVEL_ERROR, "Illegal Instruction 0x%08X\n", instruction);
            control->halt = true;
            break;
        }
        break;
    }

    case OP_JAL:
    {
        jtype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.imm = (instruction & IMM_J_MASK) >> IMM_J_SHIFT;
        
        control->rd = inst.rd;
        // Unswizzle immediate
        uint32_t imm20 = (inst.imm >> 19) & 0x1;
        uint32_t imm10_1 = (inst.imm >> 9) & 0x3FF;
        uint32_t imm11 = (inst.imm >> 8) & 0x1;
        uint32_t imm19_12 = inst.imm & 0xFF;
        uint32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
        // Sign extend immediate
        if (imm & 0x100000)
        {
            imm |= 0xFFF00000;
        }
        control->imm = imm;

        TRACE(TRACE_LEVEL_DEBUG, "JAL x%02d, %d\n", control->rd, (int32_t)control->imm);

        control->alu_a_src = true;
        control->alu_b_src = true;
        control->jump = true;
        break;
    }

    case OP_JALR:
    {
        itype_t inst;
        inst.opcode = opcode;
        inst.rd = (instruction & RD_MASK) >> RD_SHIFT;
        inst.rs1 = (instruction & RS1_MASK) >> RS1_SHIFT;
        inst.imm = (instruction & IMM_I_MASK) >> IMM_I_SHIFT;

        control->rd = inst.rd;
        control->rs1 = inst.rs1;
        // Sign extend immediate
        uint32_t imm = inst.imm;
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000;
        }
        control->imm = imm;

        TRACE(TRACE_LEVEL_DEBUG, "JALR x%02d, x%02d, %d\n", control->rd, control->rs1, (int32_t)control->imm);

        control->alu_b_src = true;
        control->jump = true;
        break;
    }

    case OP_BTYPE:
    {
        btype_t inst;
        inst.opcode = opcode;
        inst.rs1 = (instruction & RS1_MASK) >> RS1_SHIFT;
        inst.rs2 = (instruction & RS2_MASK) >> RS2_SHIFT;
        inst.funct3 = (funct3_b_t)((instruction & FUNCT3_MASK) >> FUNCT3_SHIFT);
        inst.imm5 = (instruction & IMM5_B_MASK) >> IMM5_B_SHIFT;
        inst.imm7 = (instruction & IMM7_B_MASK) >> IMM7_B_SHIFT;

        // Unswizzle immediate
        uint32_t imm12 = (inst.imm7 >> 6) & 0x1;
        uint32_t imm10_5 = inst.imm7 & 0x3F;
        uint32_t imm11 = inst.imm5 & 0x1;
        uint32_t imm4_1 = (inst.imm5 >> 1) & 0xF;
        uint32_t imm = (imm4_1 << 1) | (imm10_5 << 5) | (imm11 << 11) | (imm12 << 12);

        // Sign-extend immediate
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000;
        }
        control->imm = imm;
        control->branch = true;
        control->rs1 = inst.rs1;
        control->rs2 = inst.rs2;

        switch (inst.funct3) {
            case BEQ:
                TRACE(TRACE_LEVEL_DEBUG, "BEQ x%02d, x%02d, %d\n", control->rs1, control->rs2, (int32_t)control->imm);
                control->alu_op = ALUOP_SUB;
                control->branch_pol = 0;
                break;
            case BNE:
                TRACE(TRACE_LEVEL_DEBUG, "BNE x%02d, x%02d, %d\n", control->rs1, control->rs2, (int32_t)control->imm);
                control->alu_op = ALUOP_SUB;
                control->branch_pol = 1;
                break;
            case BLT:
                TRACE(TRACE_LEVEL_DEBUG, "BLT x%02d, x%02d, %d\n", control->rs1, control->rs2, (int32_t)control->imm);
                control->alu_op = ALUOP_SLT;
                control->branch_pol = 1;
                break;
            case BGE:
                TRACE(TRACE_LEVEL_DEBUG, "BGE x%02d, x%02d, %d\n", control->rs1, control->rs2, (int32_t)control->imm);
                control->alu_op = ALUOP_SLT;
                control->branch_pol = 0;
                break;
            case BLTU:
                TRACE(TRACE_LEVEL_DEBUG, "BLTU x%02d, x%02d, %d\n", control->rs1, control->rs2, (int32_t)control->imm);
                control->alu_op = ALUOP_SLTU;
                control->branch_pol = 1;
                break;
            case BGEU:
                TRACE(TRACE_LEVEL_DEBUG, "BGEU x%02d, x%02d, %d\n", control->rs1, control->rs2, (int32_t)control->imm);
                control->alu_op = ALUOP_SLTU;
                control->branch_pol = 0;
                break;
        }
        break;
    }

    default:
        TRACE(TRACE_LEVEL_ERROR, "Illegal Instruction 0x%08X\n", instruction);
        control->halt = true;
        break;
    }
}
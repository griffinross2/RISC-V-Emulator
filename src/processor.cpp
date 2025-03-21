#include "processor.h"

#include "string.h"
#include "control.h"
#include "alu.h"
#include "trace.h"

Processor::Processor(RAM *ram, uint32_t start_address)
{
    this->ram = ram;
    reset(start_address);
}

Processor::~Processor()
{
}

void Processor::reset(uint32_t start_address)
{
    // Initialize registers to zero
    registers.reset();

    // Initialize program counter to start address
    pc = start_address;

    // Clear halt flag
    halt = false;

    // Clear instruction count
    instruction_count = 0;
}

bool Processor::is_halted()
{
    return halt;
}

void Processor::execute_instruction()
{
    // Count the instruction
    instruction_count++;

    // Fetch the instruction
    uint32_t instruction = ram->load_instruction(pc);

    // Decode the instruction
    control_t ctrl;
    control(&ctrl, instruction);

    // Check for halt
    if (ctrl.halt)
    {
        halt = true;
        return;
    }

    // Read the source registers
    uint32_t rs1 = registers.get_reg(ctrl.rs1);
    uint32_t rs2 = registers.get_reg(ctrl.rs2);

    // Calculate ALU input A
    uint32_t alu_a = ctrl.alu_a_src ? pc : rs1;

    // Calculate ALU input B
    uint32_t alu_b = ctrl.alu_b_src ? ctrl.imm : rs2;

    // Get ALU output
    uint32_t alu_out = alu_execute(alu_a, alu_b, ctrl.alu_op, ctrl.mul_signed_a, ctrl.mul_signed_b, ctrl.mul_half);

    // Write to register from ALU output
    if (!ctrl.mem_read && !ctrl.jump)
    {
        registers.set_reg(ctrl.rd, ctrl.mem_to_reg ? ram->load_word(alu_out) : alu_out);
    }

    // Read from memory (address in ALU output)
    if (ctrl.mem_read == 1)
    {
        if (ctrl.mem_read_unsigned)
        {
            registers.set_reg(ctrl.rd, (uint32_t)ram->load_byte(alu_out));
        }
        else
        {
            // Sign extend
            uint32_t mem_val = (uint32_t)ram->load_byte(alu_out);
            if (mem_val & 0x80)
            {
                mem_val |= 0xFFFFFF00;
            }
            registers.set_reg(ctrl.rd, mem_val);
        }
    }
    else if (ctrl.mem_read == 2)
    {
        if (ctrl.mem_read_unsigned)
        {
            registers.set_reg(ctrl.rd, (uint32_t)ram->load_halfword(alu_out));
        }
        else
        {
            // Sign extend
            uint32_t mem_val = (uint32_t)ram->load_halfword(alu_out);
            if (mem_val & 0x8000)
            {
                mem_val |= 0xFFFF0000;
            }
            registers.set_reg(ctrl.rd, mem_val);
        }
    }
    else if (ctrl.mem_read == 3)
    {
        registers.set_reg(ctrl.rd, ram->load_word(alu_out));
    }

    // Write to register (linking)
    if (ctrl.jump)
    {
        registers.set_reg(ctrl.rd, pc + 4);
    }

    // Write to memory (address in ALU output)
    if (ctrl.mem_write == 1)
    {
        ram->store_byte(alu_out, (uint8_t)rs2);
    }
    else if (ctrl.mem_write == 2)
    {
        ram->store_halfword(alu_out, (uint16_t)rs2);
    }
    else if (ctrl.mem_write == 3)
    {
        ram->store_word(alu_out, rs2);
    }

    // PC destination
    if (ctrl.jump) {
        // Branch unconditionally
        pc = alu_out;
    } else if (ctrl.branch) {
        // Branch conditionally
        if ((alu_out == 0) ^ ctrl.branch_pol) {
            pc = ctrl.imm + pc;
            TRACE(TRACE_LEVEL_DEBUG, "Branching to 0x%08X\n", pc);
        } else {
            TRACE(TRACE_LEVEL_DEBUG, "Branch not taken\n");
            pc += 4;
        }
    } else {
        // No branch
        pc += 4;
    }
}

void Processor::dump_state()
{
    printf("\nPC:  0x%08X  ", pc);
    registers.dump_state();

    printf("%I64u instructions executed.\n", instruction_count);
}
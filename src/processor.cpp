#include "processor.h"

#include "string.h"
#include "control.h"
#include "alu.h"

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
}

bool Processor::is_halted()
{
    return halt;
}

void Processor::execute_instruction()
{
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
    if (!ctrl.mem_read && !ctrl.jump_reg)
    {
        registers.set_reg(ctrl.rd, ctrl.mem_to_reg ? ram->load_word(alu_out) : alu_out);
    }

    // Read from memory (address in ALU output)
    if (ctrl.mem_read)
    {
        registers.set_reg(ctrl.rd, ram->load_word(alu_out));
    }

    // Write to register (linking)
    if (ctrl.jump_reg)
    {
        registers.set_reg(ctrl.rd, pc + 4);
    }

    // Write to memory (address in ALU output)
    if (ctrl.mem_write)
    {
        ram->store_word(alu_out, rs2);
    }

    // Increment the program counter
    pc += 4;
}

void Processor::dump_state()
{
    printf("\nPC:  0x%08X  ", pc);
    registers.dump_state();
}
#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

typedef enum : unsigned int
{
    OP_RTYPE = 0b0110011, // R-type instruction
    OP_ITYPE = 0b0010011, // I-type instruction
    OP_STYPE = 0b0100011, // S-type instruction
    OP_BTYPE = 0b1100011, // B-type instruction
    OP_LUI = 0b0110111,   // LUI U-type instruction
    OP_AUIPC = 0b0010111, // AUIPC U-type instruction
    OP_JAL = 0b1101111,   // JAL J-type instruction
    OP_JALR = 0b1100111,  // JALR I-type instruction
    OP_LD_ITYPE = 0b0000011, // Load I-type instruction
} opcode_t;

typedef enum : unsigned int
{
    SLL_MULH = 0x1,
    SRL_SRA = 0x5,
    ADD_SUB_MUL = 0x0,
    AND = 0x7,
    OR = 0x6,
    XOR = 0x4,
    SLT_MULHSU = 0x2,
    SLTU_MULHU = 0x3,
} funct3_r_t;

typedef enum : unsigned int
{
    ADD_SRL = 0x00,
    SUB_SRA = 0x20,
    MULT = 0x01,
} funct7_r_t;

typedef enum : unsigned int
{
    ADDI = 0x0,
    XORI = 0x4,
    ORI = 0x6,
    ANDI = 0x7,
    SLLI = 0x1,
    SRLI_SRAI = 0x5,
    SLTI = 0x2,
    SLTIU = 0x3,
} funct3_i_t;

typedef enum : unsigned int
{
    LB = 0x0,
    LH = 0x1,
    LW = 0x2,
    LBU = 0x4,
    LHU = 0x5,
} funct3_ld_i_t;

typedef enum : unsigned int
{
    SB = 0x0,
    SH = 0x1,
    SW = 0x2,
} funct3_s_t;

#define OPCODE_MASK 0x7F
#define OPCODE_SHIFT 0
#define RD_MASK 0xF80
#define RD_SHIFT 7
#define FUNCT3_MASK 0x7000
#define FUNCT3_SHIFT 12
#define RS1_MASK 0xF8000
#define RS1_SHIFT 15
#define RS2_MASK 0x1F00000
#define RS2_SHIFT 20
#define FUNCT7_MASK 0xFE000000
#define FUNCT7_SHIFT 25
#define IMM_I_MASK 0xFFF00000
#define IMM_I_SHIFT 20
#define IMM_U_MASK 0xFFFFF000
#define IMM_U_SHIFT 12
#define IMM_J_MASK 0xFFFFF000
#define IMM_J_SHIFT 12
#define IMM5_S_MASK 0xF80
#define IMM5_S_SHIFT 7
#define IMM7_S_MASK 0xFE000000
#define IMM7_S_SHIFT 25

typedef struct
{
    funct7_r_t funct7 : 7; // Function 7
    uint32_t rs2 : 5;      // Source register 2
    uint32_t rs1 : 5;      // Source register 1
    funct3_r_t funct3 : 3; // Function 3
    uint32_t rd : 5;       // Destination register
    uint32_t opcode : 7;   // Opcode
} rtype_t;

typedef struct
{
    uint32_t imm : 12;     // Immediate
    uint32_t rs1 : 5;      // Source register 1
    funct3_i_t funct3 : 3; // Function 3
    uint32_t rd : 5;       // Destination register
    uint32_t opcode : 7;   // Opcode
} itype_t;

typedef struct
{
    uint32_t imm : 12;     // Immediate
    uint32_t rs1 : 5;      // Source register 1
    funct3_ld_i_t funct3 : 3; // Function 3
    uint32_t rd : 5;       // Destination register
    uint32_t opcode : 7;   // Opcode
} ld_itype_t;

typedef struct
{
    uint32_t imm : 20;   // Immediate
    uint32_t rd : 5;     // Destination register
    uint32_t opcode : 7; // Opcode
} lui_t;

typedef struct
{
    uint32_t imm7 : 7;     // Immediate[11:5]
    uint32_t rs2 : 5;      // Source register 2
    uint32_t rs1 : 5;      // Source register 1
    funct3_s_t funct3 : 3; // Function 3
    uint32_t imm5 : 5;     // Immediate[4:0]
    uint32_t opcode : 7;   // Opcode
} stype_t;

typedef struct
{
    uint32_t imm_swizz : 20;   // Immediate (Swizzled)
    uint32_t rd : 5;     // Destination register
    uint32_t opcode : 7; // Opcode
} jtype_t;

typedef enum
{
    ALUOP_ADD,  // Add
    ALUOP_SUB,  // Subtract
    ALUOP_AND,  // Bitwise AND
    ALUOP_OR,   // Bitwise OR
    ALUOP_XOR,  // Bitwise XOR
    ALUOP_SLT,  // Set less than
    ALUOP_SLTU, // Set less than unsigned
    ALUOP_SLL,  // Shift left logical
    ALUOP_SRL,  // Shift right logical
    ALUOP_SRA,  // Shift right arithmetic
    ALUOP_MUL,  // Multiply
} aluop_t;

// Defines what the processor should do based on the instruction
typedef struct
{
    bool halt;         // Halt
    bool mem_read;     // Read from memory
    bool mem_write;    // Write to memory
    bool mem_to_reg;   // Write to register from memory
    aluop_t alu_op;    // ALU operation
    bool branch;       // Branch
    bool branch_pol;   // Branch polarity
    bool jump;         // Jump to alu_out
    bool mul_signed_a; // Multiply signed a
    bool mul_signed_b; // Multiply signed b
    bool mul_half;     // Which half of the multiply
    int rd;            // Destination register
    int rs1;           // Source register 1
    int rs2;           // Source register 2
    uint32_t imm;      // Immediate value
    bool alu_a_src;    // ALU source A (false = register, true = pc)
    bool alu_b_src;    // ALU source B (false = register, true = immediate)
} control_t;

void control(control_t *control, uint32_t instruction);

#endif // CONTROL_H

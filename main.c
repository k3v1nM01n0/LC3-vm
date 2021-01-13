#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
/* unix */
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

//memory
uint16_t memory[UINT16_MAX];

//Registers
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT];

//opcodes
enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

//Conditional flags
enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

//Trap routines
enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};


uint16_t sign_extend(uint16_t x, int bit_count){
    if((x >> (bit_count - 1) & 1)){
        x |= (0xFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r){
    if(reg[r]== 0){
        reg[R_COND] = FL_ZRO;
    }else if(reg[r] >> 15){
        reg[R_COND] = FL_NEG;
    }else{
        reg[R_COND] = FL_POS;
    }
}


int main(int argc, const char* argv[]){

    if(argc < 2){
        printf("lc3 [image-file1] ...\r\n");
        exit(2);
    }

    for(int j = 1; j < argc; ++j){
        if(!read_image(argv[j])){
            printf("Failed to load image file %s\r\n", argv[j]);
        }
    }


    /*Set the Pc to starting position*/
    /*0x3000 is the default */
    enum{PC_START = 0x3000};
    reg[R_PC] = PC_START;

    int running = 1;
    while (running){
        /*Fetch*/
        uint16_t ins = mem_read(reg[R_PC]++);
        uint16_t op = ins >> 12;

        switch (op){
            case OP_ADD:
                {uint16_t r0 = (ins >> 9) & 0x7;//DR
                 uint16_t r1 = (ins >> 6) & 0x7;//SR
                 uint16_t imm_flag = (ins >> 5) & 0x1; //weherwe are on immediate flag or register mode
                if(imm_flag){
                    uint16_t imm5 = sign_extend(ins >> 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                }else{
                    uint16_t r2 = ins & 0x7;
                }
                update_flags(r0);
                }
                break;
            case OP_AND:
                {
                    uint16_t r0 = (ins >> 9) & 0x7;//DR
                    uint16_t r1 = (ins >> 6) & 0x7;
                    uint16_t imm_flag = (ins >> 1) & 0x1;

                    if(imm_flag){
                        uint16_t imm5 = sign_extend(ins & 0x1F, 5);
                        reg[r0] = reg[r1] & imm5;
                    }else{
                       uint16_t r2 = ins & 0x7;
                       reg[r0] =reg[r1] + reg[r2];
                    }
                }
                break;
            case OP_NOT:
                {
                    uint16_t r0 = (ins >> 9) & 0x7;//DR
                    uint32_t r1 = (ins >> 6) & 0x7;

                    reg[r0] = ~reg[r1];
                    update_flags(r0);
                }
                break;
            case OP_BR: 
                {
                    uint16_t pc_offset = sign_extend(ins & 0x1FF, 9);
                    uint16_t conn_flag = (ins >> 9) & 0x7;

                    if(conn_flag & reg[R_COND]){
                        reg[R_PC] += pc_offset;
                    }
                }
                break;
            case OP_JMP:
                {
                    uint16_t long_flag = (ins >> 11) & 1;
                    reg[R_R7] = reg[R_PC];

                    if(long_flag){
                        uint16_t long_pc_offset = sign_extend(ins & 0x7FF, 11);
                        reg[R_PC] += long_pc_offset; 
                    }else{
                        uint16_t r1 = (ins >> 6) & 0x7;
                        reg[R_PC] = reg[r1];
                    }
                }
                break;
            case OP_JSR:
                {}
                break;
            case OP_LD:
                {
                    uint16_t r0 = (ins >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(ins & 0x1FF, 9);
                    reg[r0] = mem_read(reg[R_PC] + pc_offset);
                    update_flags(r0);
                }
                break;
            case OP_LDI:
                {
                    uint16_t r0 = (ins >> 9)& 0x7;//DR
                    uint16_t pc_offcet = sign_extend(ins & 0x1FF, 9);

                    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offcet));
                    update_flags(r0);
                }
                break;
            case OP_LDR:
                {
                    uint16_t r0 = (ins >> 9)& 0x7;
                    uint16_t r1 = (ins >> 6)& 0x7;
                    uint16_t pc_offset = sign_extend(ins & 0x3F, 6);
                    reg[r0] = reg[R_PC] + pc_offset;
                    update_flags(r0);            
                }
                break;
            case OP_LEA:
                {
                    uint16_t r0 = (ins >> 9)& 0x7;
                    uint16_t pc_offset = sign_extend(ins & 0x1FF, 9);
                    reg[r0] = reg[R_PC] + pc_offset;
                    update_flags(r0);
                }
                break;
            case OP_ST:
                {
                    uint16_t r0 = (ins >> 9)& 0x7;
                    uint16_t pc_offset = sign_extend(ins & 0x1FF, 9);
                    mem_write(mem_read((reg[R_PC] + pc_offset), reg[r0]));
                }
                break;
            case OP_STI:
                { 
                    uint16_t r0 = (ins >> 9)& 0x7;
                    uint16_t pc_offset = sign_extend(ins & 0x1FF, 9);
                    mem_write(mem_read((reg[R_PC] + pc_offset), reg[r0]));
                }
                break;
            case OP_STR:
                {
                    uint16_t r0 = (ins >> 9)& 0x7;
                    uint16_t r1 = (ins >> 6)& 0x7;
                    uint16_t offset = sign_extend(ins & 0x3F, 6);
                    mem_write(reg[r1] + offset, reg[r0]);
                }
                break;
            case OP_TRAP:
                {
                    switch(ins & 0xFF)
                    {
                        case TRAP_GETC:
                            {TRAP GETC, 9}
                            break;
                        case TRAP_OUT:
                            {TRAP OUT, 9}
                            break;
                        case TRAP_PUTS:
                            {TRAP PUTS, 8}
                            break;
                        case TRAP_IN:
                            {TRAP IN, 9}
                            break;
                        case TRAP_PUTSP:
                            {TRAP PUTSP, 9}
                            break;
                        case TRAP_HALT:
                            {TRAP HALT, 9}
                            break;
                   }
                }
                break;
            case OP_RES:
            case OP_RTI:
                default:
                    {}
                    break;

            
        }
    }
    {}

}





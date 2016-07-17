#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "tables.h"
#include "translate_utils.h"
#include "translate.h"

/* SOLUTION CODE BELOW */
const int TWO_POW_SEVENTEEN = 131072;    // 2^17

/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and other pseudoinstructions. Your pseudoinstruction 
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are 
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number 
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into 
        a lui-ori pair.

   If you are going to use the $zero or $0, use $0, not $zero.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that 
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {
    if (strcmp(name, "li") == 0) {
        char * immStr = args[1];
        char *endc = "";
        long int imm = strtol(immStr, &endc, 0);

        if (num_args != 2 || INT32_MIN >= imm || imm >= UINT32_MAX || !output || !(*args))  {
          return 0;  
        } else  {
          // create instruction for li

          if (imm >= INT16_MIN && imm <= INT16_MAX) { // imm is 16 bits
            fprintf(output, "%s %s %s %s\n", "addiu", args[0], "$0", args[1]);
            return 1;
          } else  { // imm is 32 bits
            // split imm  into upper and lower halfs
            
            uint32_t upperImm = imm >> 16;
            uint32_t lowerImm = imm & 0x0000FFFF;
            fprintf(output, "%s %s %u\n", "lui", "$at", upperImm);
            fprintf(output, "%s %s %s %u\n", "ori", args[0], "$at", lowerImm);
            //printf("%s\n", "made it this far");
          }

          return 2;
        }
    } else if (strcmp(name, "move") == 0) {
        if (num_args != 2)  {
          return 0;  
        } else  {
          // convert:
          // move $rt,$rs to addu $rt,$rs,$zero;
          fprintf(output, "%s %s %s %s\n", "addu", args[0], args[1], "$0");
          return 1;
        }
    } else if (strcmp(name, "rem") == 0) {
        if (num_args != 3)  {
          return 0;  
        }  else {
          // convert rem $rd, $rs, $rt to div $rs, $rt; mfhi $rd;
          fprintf(output, "%s %s %s\n", "div", args[1], args[2]);
          fprintf(output, "%s %s\n", "mfhi", args[0]);
          return 2;
        }
    } else if (strcmp(name, "bge") == 0) {
        if (num_args != 3)  {
          return 0;  
        } else  {
          // convert:
          // bge $rs,$rt,Label to slt $at,$rs,$rt; beq $at,$zero, Label;
          fprintf(output, "%s %s %s %s\n", "slt", "$at", args[0], args[1]);
          fprintf(output, "%s %s %s %s\n", "beq", "$at", "$0", args[2]);
          return 2;
        }
    } else if (strcmp(name, "bnez") == 0) {
        if (num_args != 2)  {
          return 0;  
        } else  {
          // convert: 
          // bnez $rs,Label to bne $rs,$zero,Label;
          fprintf(output, "%s %s %s %s\n", "bne", args[0], "$0", args[1]);
          return 1;
        }
    }
    write_inst_string(output, name, args, num_args);
    return 1;

}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write 
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

   Some function declarations for the write_*() functions are provided in translate.h, and you MUST implement these
   and use these as function headers. You may use helper functions, but you still must implement
   the provided write_* functions declared in translate.h.

   Returns 0 on success and -1 on error. 
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args, uint32_t addr,
    SymbolTable* symtbl, SymbolTable* reltbl) {
    if (strcmp(name, "addu") == 0)       return write_rtype (0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0)    return write_rtype (0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0)   return write_rtype (0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0)  return write_rtype (0x2b, output, args, num_args);
    else if (strcmp(name, "sll") == 0)   return write_shift (0x00, output, args, num_args);
    else if (strcmp(name, "jr") == 0)    return write_jr    (0x08, output, args, num_args);
    else if (strcmp(name, "addiu") == 0) return write_addiu (0x09, output, args, num_args);
    else if (strcmp(name, "ori") == 0)   return write_ori   (0x0d, output, args, num_args);
    else if (strcmp(name, "lui") == 0)   return write_lui   (0x0f, output, args, num_args);
    else if (strcmp(name, "lb") == 0)    return write_mem   (0x20, output, args, num_args);
    // make sure write_mem diffrentiates between lb and lbu
    else if (strcmp(name, "lbu") == 0)   return write_mem   (0x24, output, args, num_args); 
    else if (strcmp(name, "lw") == 0)    return write_mem   (0x23, output, args, num_args);
    else if (strcmp(name, "sb") == 0)    return write_mem   (0x28, output, args, num_args);
    else if (strcmp(name, "sw") == 0)    return write_mem   (0x2b, output, args, num_args);
    else if (strcmp(name, "beq") == 0)   return write_branch(0x04, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "bne") == 0)   return write_branch(0x05, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "j") == 0)     return write_jump  (0x02, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "jal") == 0)   return write_jump  (0x03, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "mult") == 0)  return write_rtype (0x18, output, args, num_args);
    else if (strcmp(name, "div") == 0)   return write_rtype (0x1a, output, args, num_args);
    else if (strcmp(name, "mfhi") == 0)  return write_rtype (0x10, output, args, num_args);
    else if (strcmp(name, "mflo") == 0)  return write_rtype (0x12, output, args, num_args);
    return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?

    int rd = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int rt = translate_reg(args[2]);

    if (rd == -1 || rs == -1 || rt == -1)  {
      return -1;
    }

    // rs rt rd func
    uint32_t instruction = 0;
    // Our original shift method
    instruction = ((((((instruction + rs) << 5) + rt) << 5)  + rd) << 11) + funct;
    
    // Below is one option to fix it
    // instruction = funct + (rs << 21) + (rt << 16) + (rd << 11);

    //instruction = 
    
    // Below is the second option to fix it
    // instruction += funct;
    // instruction += (rd << 11);
    // instruction += (rt << 16);
    // instruction += (rs << 21);
    
    write_inst_hex(output, instruction);
    return 0;
}

/* A helper function for writing shift instructions. You should use 
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_shift(uint8_t funct, FILE* output, char** args, size_t num_args) {
	// Perhaps perform some error checking?

    long int shamt;
    int rd = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int err = translate_num(&shamt, args[2], 0, 31);

    if (rd == -1 || rt == -1 || err == -1) {
      return -1;
    }

    uint32_t instruction = 0;
    instruction = ((((((instruction + rt) << 5) + rd) << 5) + shamt) << 6) + funct;
    write_inst_hex(output, instruction);
    return 0;
}

/* The rest of your write_*() functions below */

int write_jr(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    int rs = translate_reg(args[0]);
    if (rs == -1) {
      return -1;
    }

    uint32_t instruction = 0;
    instruction = ((instruction + rs) << 21) + funct;
    write_inst_hex(output, instruction);
    return 0;
}

int write_addiu(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int err = translate_num(&imm, args[2], INT16_MIN, INT16_MAX);

    if (rt == -1 || rs == -1 || err == -1)  {
      return -1;
    }

    // opcode rs rt imm
    uint32_t instruction = 0;
    instruction = (instruction + opcode) << 5;
    instruction = (instruction + rs) << 5;
    instruction = (instruction + rt) << 16;
    instruction = (instruction + imm);
    write_inst_hex(output, instruction);
    return 0;
}

int write_ori(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int err = translate_num(&imm, args[2], 0, UINT16_MAX);

    if (rt == -1 || rs == -1 || err == -1)  {
      return -1;
    }

    // opcode rs rt imm
    uint32_t instruction = 0;
    instruction = (instruction + opcode) << 5;
    instruction = (instruction + rs) << 5;
    instruction = (instruction + rt) << 16;
    instruction += imm;

    write_inst_hex(output, instruction);
    return 0;
}

int write_lui(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    
    long int imm;
    int rt = translate_reg(args[0]);
    int err = translate_num(&imm, args[1], 0, UINT16_MAX);
    if (rt == -1 || err == -1)  {
      return -1;
    }
    uint32_t instruction = 0;
    instruction = (instruction + opcode) << 10;
    instruction = (instruction + rt) << 16;
    instruction += imm;
    write_inst_hex(output, instruction);
    return 0;
}

int write_mem(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[2]);
    int err = translate_num(&imm, args[1], INT16_MIN, INT16_MAX);
    if (rt == -1 || rs == -1 || err == -1)  {
      return -1;
    }

    // opcode, rs, rt, imm
    uint32_t instruction = 0;
    instruction = (instruction + opcode) << 5;
    instruction = (instruction + rs) << 5;
    instruction = (instruction + rt) << 16;
    instruction += imm & 0x0000FFFF;
    write_inst_hex(output, instruction);
    return 0;
}

/*  A helper function to determine if a destination address
    can be branched to
*/
static int can_branch_to(uint32_t src_addr, uint32_t dest_addr) {
    int32_t diff = dest_addr - src_addr;
    return (diff >= 0 && diff <= TWO_POW_SEVENTEEN) || (diff < 0 && diff >= -(TWO_POW_SEVENTEEN - 4));
}


int write_branch(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* symtbl) {
    // Perhaps perform some error checking?
    
    int rs = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int label_addr = get_addr_for_symbol(symtbl, args[2]);
    if (rs == -1 || rt == -1 || label_addr == -1) {
      return -1;
    }
    //Please compute the branch offset using the MIPS rules.
    int32_t offset = (label_addr - (addr + 4)) >> 2; // take out plus 4 if buggy
    uint32_t instruction = 0;
    instruction = (instruction + opcode) << 5;
    instruction = (instruction + rs) << 5;
    instruction = (instruction + rt) << 16;
    instruction += offset & 0x0000FFFF;
    write_inst_hex(output, instruction);
    return 0;
}

int write_jump(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* reltbl) {
    if (num_args != 1)  {
      return -1;
    }
    char * label = args[0];
    int err = add_to_table(reltbl, label, addr);
    if (err == -1)  {
      return -1;
    }
    uint32_t instruction = 0;
    instruction = (instruction + opcode) << 26;
    write_inst_hex(output, instruction);
    return 0;
}

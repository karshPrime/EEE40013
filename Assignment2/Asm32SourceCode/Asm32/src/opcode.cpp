/*
**  opcode.c - table of information for assembler
*/
#include <stddef.h>
#include "opcode.h"

typedef int class_handler(void);

class_handler do_INHERENT;
class_handler do_2or3REGISTER;
class_handler do_2REGISTER;
class_handler do_INDEXED;
class_handler do_REGISTER;
class_handler do_BRANCH;

class_handler do_ORG;
class_handler do_REG;
class_handler do_EQU;
class_handler do_DC;
class_handler do_DS;
class_handler do_ALIGN;
class_handler do_END;
class_handler do_DATA;
class_handler do_TEXT;
class_handler do_GLOBAL;
class_handler do_EXTERN;
class_handler do_XDEF;
class_handler do_XREF;
class_handler do_PUSH;
class_handler do_PULL;

op_entry op_info[]=
/*
    Information for each instruction
*/
{
/*
  <pseudo-op> <args....>
*/
/* mnemonic     class           size            opcode  ea modes */
{"ORG",      do_ORG,            NO_SIZE,        0x0000, PSEUDO_OP},
#ifdef LABELS
{"EQU",      do_EQU,            NO_SIZE,        0x0000, PSEUDO_OP},
{"REG",      do_REG,            NO_SIZE,        0x0000, PSEUDO_OP},
#endif
{"DC",       do_DC,             ANY_SIZE,       0x0000, PSEUDO_OP},
{"BYTE",     do_DC,             BYTE_SIZE,      0x0000, PSEUDO_OP},
{"WORD",     do_DC,             WORD_SIZE,      0x0000, PSEUDO_OP},
{"LONG",     do_DC,             LONG_SIZE,      0x0000, PSEUDO_OP},
{"ASCII",    do_DC,             BYTE_SIZE,      0x0000, PSEUDO_OP},
{"DS",       do_DS,             ANY_SIZE,       0x0000, PSEUDO_OP},
{"BLOCK",    do_DS,             BYTE_SIZE,      0x0000, PSEUDO_OP},
{"RMB",      do_DS,             BYTE_SIZE,      0x0000, PSEUDO_OP},
{"ALIGN",    do_ALIGN,          ANY_SIZE,       0x0000, PSEUDO_OP},
#ifdef ASM
{"EXTERN",   do_EXTERN,         NO_SIZE,        0x0000, PSEUDO_OP},
{"XREF",     do_EXTERN,         NO_SIZE,        0x0000, PSEUDO_OP},
{"GLOBAL",   do_GLOBAL,         NO_SIZE,        0x0000, PSEUDO_OP},
{"XDEF",     do_GLOBAL,         NO_SIZE,        0x0000, PSEUDO_OP},
{"TEXT",     do_TEXT,           NO_SIZE,        0x0000, PSEUDO_OP},
{"DATA",     do_DATA,           NO_SIZE,        0x0000, PSEUDO_OP},
{"END",      do_END,            NO_SIZE,        0x0000, PSEUDO_OP},
#endif

/*
  <mnemonic>
*/
/* mnemonic     class           size            opcode  ea modes */
{"RTS",      do_INHERENT,       NO_SIZE,        0x40000000+(31<<16), NOT_USED},
/*
  <mnemonic> Ra,Rb
  <mnemonic> Ra,#dddd
*/
{"MOV",      do_2REGISTER,      NO_SIZE,       0x00000000+(0x0<<26), NOT_USED},
{"MOVH",     do_2REGISTER,      NO_SIZE,       0x00000000+(0x5<<26), NOT_USED},
{"SWAP",     do_2REGISTER,      NO_SIZE,       0x00000000+(0x5<<26), NOT_USED},

{"ADD",      do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x0<<26), NOT_USED},
{"SUB",      do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x1<<26), NOT_USED},
{"AND",      do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x2<<26), NOT_USED},
{"OR",       do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x3<<26), NOT_USED},
{"EOR",      do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x4<<26), NOT_USED},
{"XOR",      do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x4<<26), NOT_USED},
{"MUL",      do_2or3REGISTER,   NO_SIZE,       0x00000000+(0x7<<26), NOT_USED},
/*
  <mnemonic> Rb,dddd(Ra)
*/
{"LD",       do_INDEXED,        ANY_SIZE,      0x40000000, NOT_USED},
{"ST",       do_INDEXED,        ANY_SIZE,      0x60000000, NOT_USED},
/*
  <mnemonic> dddd(Ra)
*/
{"JMP",      do_REGISTER,       NO_SIZE,       0x40000000, NOT_USED},
/*
  <mnemonic> <label>
*/
{"BRA",       do_BRANCH,        NO_SIZE, 0x80000000+(0x0<<23), NOT_USED},
{"BSR",       do_BRANCH,        NO_SIZE, 0x80000000+(0x1<<23), NOT_USED},
{"BCS",       do_BRANCH,        NO_SIZE, 0x80000000+(0x2<<23), NOT_USED},
{"BLO",/*BCS*/do_BRANCH,        NO_SIZE, 0x80000000+(0x2<<23), NOT_USED},
{"BCC",       do_BRANCH,        NO_SIZE, 0x80000000+(0x3<<23), NOT_USED},
{"BHS",/*BCC*/do_BRANCH,        NO_SIZE, 0x80000000+(0x3<<23), NOT_USED},
{"BEQ",       do_BRANCH,        NO_SIZE, 0x80000000+(0x4<<23), NOT_USED},
{"BNE",       do_BRANCH,        NO_SIZE, 0x80000000+(0x5<<23), NOT_USED},
{"BVS",       do_BRANCH,        NO_SIZE, 0x80000000+(0x6<<23), NOT_USED},
{"BVC",       do_BRANCH,        NO_SIZE, 0x80000000+(0x7<<23), NOT_USED},
{"BMI",       do_BRANCH,        NO_SIZE, 0x80000000+(0x8<<23), NOT_USED},
{"BPL",       do_BRANCH,        NO_SIZE, 0x80000000+(0x9<<23), NOT_USED},
{"BLT",       do_BRANCH,        NO_SIZE, 0x80000000+(0xA<<23), NOT_USED},
{"BGE",       do_BRANCH,        NO_SIZE, 0x80000000+(0xB<<23), NOT_USED},
{"BLE",       do_BRANCH,        NO_SIZE, 0x80000000+(0xC<<23), NOT_USED},
{"BGT",       do_BRANCH,        NO_SIZE, 0x80000000+(0xD<<23), NOT_USED},
{"BLS",       do_BRANCH,        NO_SIZE, 0x80000000+(0xE<<23), NOT_USED},
{"BHI",       do_BRANCH,        NO_SIZE, 0x80000000+(0xF<<23), NOT_USED},

{NULL,0,0,0,0}
};

/*
 **      asm.c
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "exprn.h"
#include "main.h"
#include "symbol.h"
#include "asm.h"
#include "opcode.h"

/****************************************************************/
/*    Global shared data                                        */
/****************************************************************/

char    *label;                                   /* label field (NULL if none)    */
char    *mnemonic;                                /* mnemonic field                */
char    *args;                                    /* argument field (NULL if none) */
char    *comment;                                 /* comment field (NULL if none)  */
static  int err_flag;                             /* true if error in current line */
static  int  err_pass1;                           /* count of total errors in pass1 pass */
static  int  err_pass2;                           /* count of total errors in pass2 pass */
static  int  war_pass1;                           /* count of total warnings in pass1 pass */
static  int  war_pass2;                           /* count of total warnings in pass2 pass */
static  int  end_of_source;                       /* set 1 on END pseudo-op */
static  char list_delimiter = '|';                /* delimiter after address in listing */
int     pass;                                     /* assembler pass 1 or 2         */
char const      *argptr;                          /* ptr to current position in args */

static constexpr unsigned MAX_OPS_A_LINE = 8;     /* max. # of bytes/line in listing */
static constexpr unsigned MAX_INSTRN_SIZE = 100;  /* Max. # of bytes in an instrn. */
static uint8_t       instrn_buf[MAX_INSTRN_SIZE];  /* instrn. bytes */
static uint8_t       *instrn_ptr;                 /* ptr into instrn_buf */
static uint32_t      initial_pc;                  /* PC value for first byte of instruction */
static uint32_t      current_pc;                  /* PC value for current byte of instruction */
static op_entry *entry;                           /* Information for current instruction */
static int      size;                             /* Size for instruction (may be default) */
static int      size_given;                       /* True if size extension given on mnemonic */
static segment_type    current_segment=TEXT_SEG;  /* segment for symbols */
static int32_t      segment_pc[LAST_SEG+1];       /* pcs for each segment */
static entry_type      seg_type[LAST_SEG+1]={TEXT_SYM,DATA_SYM};

bool exprnx(const char *&ptr, int32_t &value) {
   if (pass == 1) {
      // OK if undefined exprn in pass 1
      return exprn(ptr,value)>=0;
   }
   else {
      // Must be resolved in pass 2
      return exprn(ptr,value)>0;
   }
}

static const char * getAluOpName(uint op) {
   switch(op) {
      case 0: return "add";
      case 1: return "sub";
      case 2: return "and";
      case 3: return "or";
      case 4: return "xor";
      case 5: return "swap";
      case 6: return "--";
      case 7: return "mul";
   }
   return "unknown";
}

static const char * getBccOpName(uint op) {
   const char *names[] = {
         "Bra", "Bsr", "Bcs", "Bcc", "Beq", "Bne", "Bvs", "Bvc", "Bmi", "Bpl", "Blt", "Bge", "Ble", "Bgt", "Bls", "Bhi",
   };
   if (op>(sizeof(names)/sizeof(names[0]))) {
      return "Unknown";
   }
   return names[op];
}


/*
 **  Prints out listing line.
 **
 **  Only prints bytes generated if err_flag is false.
 **
 */
void print_line(FILE *ofile) {

   unsigned opcount;
   uint8_t  *i_ptr=instrn_buf;

   fprintf(ofile,"%8.8x %c ",initial_pc, list_delimiter);   /* address */
   for (opcount=0; opcount<MAX_OPS_A_LINE; opcount++) {
      /* 1st line of bytes */
      if ((pass == 2) && (i_ptr != instrn_ptr) && !err_flag)
         fprintf(ofile,"%2.2X",(uint8_t)*i_ptr++);
      else
         fprintf(ofile,"  ");
   }
   if ((label != NULL) || (mnemonic != NULL)) {
      fprintf(ofile," %-10s %-7s %-20s",    /* source text */
            label==NULL?"":label,
                  mnemonic==NULL?"":mnemonic,
                        args==NULL?"":args);
   }
   if (mnemonic != nullptr) {
      uint32_t opcode = (instrn_buf[0]<<24)+(instrn_buf[1]<<16)+(instrn_buf[2]<<8)+instrn_buf[3];
      uint32_t offset = 0;
      switch(opcode&(0b111<<29)) {
         case 0U<<29: // R,R,R
               fprintf(ofile,"; %-5s R%-2d,R%-2d,R%-2d",
                     getAluOpName((opcode>>26)&0b111),
                     (opcode>>21)&0b11111,
                     (opcode>>16)&0b11111,
                     (opcode>>11)&0b11111);
         break;

         case 1U<<29: // R,R,#
               fprintf(ofile,"; %-5s R%-2d,R%-2d,#%u",
                     getAluOpName((opcode>>26)&0b111),
                     (opcode>>21)&0b11111,
                     (opcode>>16)&0b11111,
                     (uint16_t)(opcode&0xFFFFU));
         break;

         case 2U<<29:
               if (((opcode>>21)&0b11111)==0) {
                  // Jmp ddd(Rb)
                  fprintf(ofile,"; Jmp %u(R%-2d)",
                        (int16_t)(opcode&0xFFFFU),
                        (opcode>>16)&0b11111);
               }
               else {
                  // Ld Ra,ddd(Rb)
                  fprintf(ofile,"; Ld R%-2d,%d(R%-2d)",
                        (opcode>>21)&0b11111,
                        (int16_t)(opcode&0xFFFFU),
                        (opcode>>16)&0b11111);
               }
         break;

         case 3U<<29: // St Rc,ddd(Rb)
               fprintf(ofile,"; St R%-2d,R%-2d,#%u",
                     (opcode>>21)&0b11111,
                     (opcode>>16)&0b11111,
                     (uint16_t)(opcode&0xFFFFU));
         break;

         case 4U<<29: // Bcc offset
               offset = (opcode&0x7FFFFFU);
               if (offset&(1<<22)) {
                  // sign extend
                  offset |= 0xFFC00000;
               }

               fprintf(ofile,"; %s #%d (%X)",
                     getBccOpName((opcode>>23)&0b1111), 4*offset, offset);
         break;

         default:
            break;
      }
   }
   if (comment != NULL) {
      fprintf(ofile,";%s\n",comment==NULL?"":comment);
   }
   else {
      fprintf(ofile,"\n");
   }
   if ((pass == 2) && !err_flag) {
      while (i_ptr != instrn_ptr) /* rest of bytes */
      {
         fprintf(ofile,"%8.8lx + ",initial_pc+(i_ptr-instrn_buf)); /* address */
         for (opcount=0; opcount<MAX_OPS_A_LINE; opcount++)
         {
            if (i_ptr != instrn_ptr)
               fprintf(ofile,"%2.2X",(uint8_t)*i_ptr++);
         }
         fprintf(ofile,"\n");
      }
   }
}

/*
   Aligns initial_pc to boundary given (1,2,4,8,16 etc)

   Returns the adjustment made to the PC.
 */
static int align(int boundary) {

   boundary -= 1;
   boundary = (~((int)initial_pc+boundary))&boundary;
   initial_pc += boundary;
   return((int)boundary);
}

/*
    Resets instruction buffer.
 */
static void clear_instrn_buf(void) {

   set_star_value(initial_pc);   /* set value of '*' to address of opcode */
   current_pc = initial_pc+4;    /* save address of 1st extension word */
   instrn_ptr = instrn_buf+4;    /* point to 1st extension word */
   err_flag = 0;                 /* no error so far */
}

/*
  Sets instruction buffer to empty (for non-instructions)
 */
static void reset_instrn_buf(void) {

#ifdef ASM
   list_delimiter = '|';   /* delimiter after address in listing */
#endif
   instrn_ptr = instrn_buf;    /* point to 1st byte in buffer */
   current_pc = initial_pc;
}

void gen_opcode(int32_t op) {

   *instrn_buf     = (op>>24) & 0xFF;
   *(instrn_buf+1) = (op>>16) & 0xFF;
   *(instrn_buf+2) = (op>>8)  & 0xFF;
   *(instrn_buf+3) = op       & 0xFF;
}

void gen_word2(int16_t value) {

   *(instrn_buf+2) = (value>>8)&0xFF;  /* 2nd word in buffer */
   *(instrn_buf+3) = value&0xFF;
}

/*
  write a byte to instrn_buf
 */
static void gen_byte(int8_t byte) {

   *instrn_ptr++ = byte;
   current_pc += 1;
}

/*
  write a word to instrn_buf
 */
static void gen_word(int16_t word) {

   gen_byte((int8_t)((word>>8) & 0xFF));
   gen_byte((int8_t)(word & 0xFF));
}

/*
  write a long word to instrn_buf but it will not be written
  to object file (e.g. EQU value)
 */
static void gen_value(int32_t long_v) {

   gen_word((uint16_t)(long_v>>16));
   gen_word((uint16_t)long_v);
   current_pc = initial_pc;
}

/*
  write a long word to instrn_buf
 */
static void gen_long(int32_t long_v) {

   gen_word((uint16_t)(long_v>>16));
   gen_word((uint16_t)long_v);
}

/*
   Writes bytes in instruction buffer to object file (screen).
 */
static void flush_instrn_buf(void) {

#ifdef debug
   int8_t *i_ptr = instrn_buf+1;

   printf("%8.8lx | ",initial_pc);    /* address */
   printf("%4.4x ",*instrn_buf);      /* opcode word */
   while (i_ptr != instrn_ptr)        /* extension words */
      printf("%4.4X ",*i_ptr++);
   putchar('\n');
#else
   uint8_t *i_ptr = instrn_buf;
   int32_t address = initial_pc;

   while (i_ptr != instrn_ptr)         /* opcode & extension words */
#ifdef ASM
      out_objfile(address++,*i_ptr++); /* write byte in object code file */
#endif // ASM
#ifdef SIM
   set_MEM(address++,*i_ptr++); /* write byte directly to memory */
#endif // SIM
#ifdef MON
   set_mem(address++,*i_ptr++); /* write byte directly to memory */
#endif // SIM
#endif
}

/****************************************************************/
#define WARNING (0x80)

enum {ERR_UNKNOWN,
   ERR_ILL_OPS,
   ERR_OP_TOO_LARGE,
   ERR_ILL_SIZE,
   ERR_VAL_OUT_OF_RANGE,
   ERR_UNKNOWN_MNEMONIC,
   ERR_LABEL_REQUIRED,
   ERR_ILLEGAL_EXPRESSION,
   ERR_COMMA_EXPECTED,
   ERR_UNTERMINATED_STRING_OR_CHAR,
   ERR_LABEL_NOT_ALLOWED,
   ERR_ILLEGAL_LABEL,
   ERR_BRANCH_TOO_FAR,
   LAST_ERROR,
};

enum {ERR_LABEL_MULTIPLY_DEFINED=LAST_ERROR+WARNING,
   ERR_ALIGNMENT,
   ERR_PHASING,
};

static const char *err_messages[]=
{
      "Unknown assembler error",
      "Illegal operands",
      "Operand too large",
      "Illegal size",
      "Value too large",
      "Unknown mnemonic",
      "Label required on SET or EQU",
      "Illegal expression",
      "Comma expected",
      "Unterminated string or character constant",
      "Label not allowed",
      "Illegal label",
      "Branch too far",
      /* warnings last */
      "Label multiply defined",
      "Instruction realigned on word address",
      "Phasing Error (label has different value in pass 2)",
      NULL
};

#define MAX_ERROR_MESSAGE ((sizeof(err_messages) / sizeof(char *)) - 1)

static void asm_error(unsigned err_num) {
   int warning;

   warning = err_num & WARNING;
   err_num &= ~WARNING;
   if (err_num > MAX_ERROR_MESSAGE)
      err_num = 0;
   if (warning || !err_flag) /* only report first error on each line */
   {
      if (!warning) /* only flag 1 error but multiply warnings */
         err_flag = 1;
      fprintf(stderr,"%c***** : %s\n", warning?'W':'E',
            err_messages[err_num]);
      fprintf(listfile,"%c***** : %s\n", warning?'W':'E',
            err_messages[err_num]);
      if (pass==1)          /* print out offending line in pass 1 */
      {
         print_line(listfile);
         warning?war_pass1++:err_pass1++;
      }
      else
         warning?war_pass2++:err_pass2++;
      print_line(stderr);
   }
}

/****************************************************************/
/*    General Parsing routines                                  */
/****************************************************************/

#define is_uword_size(x)  (((uint32_t)(x)) <= 65535UL)
#define is_sword_size(x) ((((int32_t)(x)) >= -32768L) && (((int32_t)(x)) <= 32767L))
#define is_ubyte_size(x)  (((uint32_t)(x)) <= 255UL)
#define is_sbyte_size(x) ((((int32_t)(x)) >= -128L) && (((int32_t)(x)) <= 127L))


static int isS16Size( int32_t value ) {

   return value == (int32_t)((int16_t) value);
}


/**
 *  This routine accepts a template consisting of the following control
 *  sequences preceded by a percentage character '%' (in a form that
 *  resembles scanf).  Other characters are matched literally (but is
 *  not case sensitive).
 *
 *  R => Rn                  opcode |= Register #
 *  V => expression          number  = value of expression
 *
 *  The R alternative also accept a shift value between the % and
 *  the 'R' e.g. '%9R'.  This will cause a register number to be
 *  inserted into the opcode field as bits 9..11.
 *
 *  In pass 1 undefined expressions (i.e. those involving forward
 *  references) are not treated as errors.  Undefined absolute expressions
 *  return a value 0.  PC relative expressions return a value of 1. In pass
 *  two they are flagged as errors
 *
 */
static int parse_ea(
      char const *&operand,
      char const *templ,
      uint32_t   &opcode,
      int32_t    &number) {

   uint32_t tOpcode = opcode;
   int32_t  tNumber = 0;

   int32_t  value;

   /* indicates template match but operand size too big
      this generates an error message */
   bool  fail = false;

   unsigned shift;
   int regNum;

   char const *b_ptr = operand;

   for (char const *t_ptr = templ; *t_ptr != '\0';)
   {
      if (*t_ptr == '%')
      {
         if (isdigit(*++t_ptr))     /* shift value ? */
            shift=parse_num(t_ptr); /* get it */
         else
            shift = 0; /* no shift */

         switch(*t_ptr++) {
            case 'R' :  /* Register reg */
               if (toupper(*b_ptr++) != 'R')
                  return(0);
               if (!isdigit(*b_ptr))
                  return(0);
               regNum = *b_ptr++ - '0';
               if (isdigit(*b_ptr)) {/* 2-digit register # */
                  regNum *= 10;
                  regNum += *b_ptr++ - '0';
               }
               if (regNum > 31) {
                  return(false);
               }
               tOpcode |= regNum<<shift; // encode register field
               break;
            case 'V' :  /* value. */
               if (!exprnx(b_ptr,value))
                  return(0);
               tNumber = value;
               break;
            default  :
               exit(EXIT_FAILURE);
               break;
         }
      }
      else { /* match literal character */
         if (toupper(*b_ptr++) != *t_ptr++)
            return(0); /* failed match */
      }
   }
   if (fail) {
      asm_error(ERR_OP_TOO_LARGE);
      return(0);
   }

   // Update results on success
   opcode = tOpcode;
   number = tNumber;

   // Advance to next operand
   operand=b_ptr;

   return(true);
}


/*
  Parses a general register r0 - r7.

  returns : 0 => invalid register
            1 => valid register
                 reg = 0-7  => r0-r7
 */
static int parse_reg(const char **args, uint16_t *reg_num) {

   const char *aptr = *args;
   int regNum;

   if (toupper(*aptr++) != 'R')
      return(0);
   if (!isdigit(*aptr))
      return(0);
   regNum = *aptr++ - '0';
   if (isdigit(*aptr)) {/* 2-digit register # */
      regNum *= 10;
      regNum += *aptr++ - '0';
   }
   if (regNum > 31)
      return(0);
   *reg_num = regNum;
   *args = aptr;
   return (1);
}

/*
  Parses a register list.  This will accept register names (r0, r3 etc)
  or a range of registers (r3-r8) or an immediate value.  Elements in the
  list are separated by slashes.

  Returns : false => failed;
            true  => success;
               Regs is set to a register list mask
                 (as for push instruction).
 */
static int parse_reg_list(const char **args, uint16_t *Regs) {

   const char *aptr = *args;
   uint16_t  reg, reg2;
   int32_t data;
   uint32_t dummy;

   if (parse_ea(aptr,"#%V",dummy,data)) { /* #value */
      if (data & 0xFFFF0000L) { /* mask to large ? */
         asm_error(ERR_VAL_OUT_OF_RANGE);
         return(false);
      }
      *Regs = (int)data & 0xFFFF;
      *args = aptr;
      return(true);
   }

   *Regs = 0;

   do {
      if (!parse_reg(&aptr,&reg)) /* get register */
         return(false);
      if (*aptr == '-') { /* range specifier ? */
         aptr++;
         if (!parse_reg(&aptr,&reg2)) /* get end of range ? */
            return(false);
         for (;reg <= reg2; reg++)
            *Regs |= 1<<reg;
      }
      else
         *Regs |= 1<<reg; /* single reg */
   }
   while (*aptr++=='/');

   aptr--;

   if (*aptr==',')
      aptr++;

   *args = aptr;

   return(true);
}

/****************************************************************/
/*     Instruction specific routines                            */
/****************************************************************/

/*
 <mnemonic>
 */
int do_INHERENT(void) {

   if (size != NO_SIZE_IDX) {
      asm_error(ERR_ILL_SIZE);
      return(0);
   }

   gen_opcode(entry->opcode);
   return(2);
}

/*
  <mnemonic> Rb,dddd(Ra)
  <mnemonic> Rb,dddd
  <mnemonic> Rb,(Ra)
 */
int do_INDEXED(void) {

   int32_t value = 0;

   uint32_t opcode = entry->opcode;
   if (parse_ea( argptr, "%21R,%V(%16R)", opcode, value )) {    // Ra,dddd(Rb)
   }
   else if (parse_ea( argptr, "%21R,(%16R)", opcode, value )) { // Ra,(Rb) = Ra,0(Rb)
   }
   else if (parse_ea( argptr, "%21R,%V", opcode, value )) {     // Ra,dddd = Ra,(R0)
   }
   else {
      asm_error(ERR_ILL_OPS);
      return(0);
   }
   if (!isS16Size(value)) {
      asm_error(ERR_VAL_OUT_OF_RANGE);
      return(0);
   }
   gen_opcode(opcode|(uint16_t)value);
   return(4);

}


/*
  <mnemonic> Ra,Rb
  <mnemonic> Ra,#dddd
 */
int do_2REGISTER(void) {
   int32_t value = 0;

   uint32_t opcode = entry->opcode;

   if (parse_ea( argptr, "%21R,%11R", opcode, value )) { // Ra,Rb
      gen_opcode(opcode);
      return(4);
   }
   else if (parse_ea( argptr, "%21R,#%V", opcode, value )) { // Ra,#dddd
      if (!isS16Size(value)) {
         asm_error(ERR_VAL_OUT_OF_RANGE);
         return(0);
      }
      // # immediate
      opcode |= (1<<29);
      // R21 = R16
      gen_opcode(opcode|(uint16_t)value);
      return(4);
   }
   else {
      asm_error(ERR_ILL_OPS);
      return(0);
   }
}

/*
  <mnemonic> Ra,Rb
  <mnemonic> Ra,#dddd
  <mnemonic> Ra,Rb,Rc
  <mnemonic> Ra,Rb,#dddd
 */
int do_2or3REGISTER(void) {
   int32_t value = 0;

   uint32_t opcode = entry->opcode;

   if (parse_ea( argptr, "%21R,%16R,%11R", opcode, value )) { // Ra,Rb,Rc
      gen_opcode(opcode);
      return(4);
   }
   else if (parse_ea( argptr, "%21R,%16R,#%V", opcode, value )) { // Ra,Rb,#dddd
      if (!isS16Size(value)) {
         asm_error(ERR_VAL_OUT_OF_RANGE);
         return(0);
      }
      // # immediate
      opcode |= (1<<29);
      gen_opcode(opcode|(uint16_t)value);
      return(4);
   }
   else if (parse_ea( argptr, "%16R,%11R", opcode, value )) { // Ra,Rb == Ra,Rb,Rb
      // R21 = R16
      uint32_t repeatedReg = ((0b11111<<21)&(opcode<<(21-16)));
      gen_opcode(opcode|repeatedReg);
      return(4);
   }
   else if (parse_ea( argptr, "%16R,#%V", opcode, value )) { // Ra,#dddd = Ra,Ra,#dddd
      if (!isS16Size(value)) {
         asm_error(ERR_VAL_OUT_OF_RANGE);
         return(0);
      }
      // # immediate
      opcode |= (1<<29);
      // R21 = R16
      uint32_t repeatedReg = ((0b11111<<21)&(opcode<<(21-16)));
      gen_opcode(opcode|repeatedReg|(uint16_t)value);
      return(4);
   }
   else {
      asm_error(ERR_ILL_OPS);
      return(0);
   }
}

/*
  <mnemonic> dddd(Ra)
  <mnemonic> dddd
  <mnemonic> (Ra)
 */
int do_REGISTER(void) {

   int32_t value = 0;

   uint32_t opcode = entry->opcode;
   if (parse_ea( argptr, "%V(%16R)", opcode, value )) {    // dddd(Rb)
   }
   else if (parse_ea( argptr, "(%16R)", opcode, value )) { // (Rb) = 0(Rb)
   }
   else if (parse_ea( argptr, "%V", opcode, value )) {     // dddd = dddd(R0)
   }
   else {
      asm_error(ERR_ILL_OPS);
      return(0);
   }
   if (!isS16Size(value)) {
      asm_error(ERR_VAL_OUT_OF_RANGE);
      return(0);
   }
   gen_opcode(opcode|(uint16_t)value);
   return(4);
}

/*
  <mnemonic> dddd
 */
int do_BRANCH(void) {

   int value;

   uint32_t opcode = entry->opcode;
   if (parse_ea( argptr, "%V", opcode, value )) { // dddd
      value -= initial_pc+4;
      value /= 4;
      if (pass == 2)
         if (value<(int32_t)0xFF800000L) {
            asm_error(ERR_VAL_OUT_OF_RANGE);
            return(0);
         }
      if (value>0x7FFFFF) {
         asm_error(ERR_VAL_OUT_OF_RANGE);
         return(0);
      }
      value &= 0x7FFFFF;
      gen_opcode( opcode|value );
      return(4);
   }
   else {
      asm_error(ERR_ILL_OPS);
      return(0);
   }
}


//==============================================================================
//==============================================================================

int do_ALIGN(void) {

   int32_t  value;
   int  sizes[]={1,2,4};
   int  mask;

#ifdef LABELS
   if (label != NULL) /* no labels allowed on align */
      asm_error(ERR_LABEL_NOT_ALLOWED);
#endif //  LABELS

   if (argptr==NULL) /* no argument - use size field */
      mask=sizes[size];
   else if ((exprn(argptr,value)<=0) || (value < 1) || (value > 5))
   {
      asm_error(ERR_ILLEGAL_EXPRESSION);
      return(0);
   }
   else
      mask = (1<<(int)value);

   align(mask);
   reset_instrn_buf();

   return(0);
}

int do_DS(void) {

   int32_t value;
   int sizes[]={1,2,4};

#ifdef LABELS
   if ((label != NULL) &&          /* label and */
         (pass == 1) &&              /* pass 1, but */
         !enter_symbol(label, initial_pc, seg_type[current_segment]))
      /* failed add to symbol table */
      asm_error(ERR_LABEL_MULTIPLY_DEFINED);
#endif //  LABELS

   if ((exprn(argptr,value)<=0) || value < 0)
   {
      asm_error(ERR_ILLEGAL_EXPRESSION);
      return(0);
   }

   reset_instrn_buf();
   current_pc = initial_pc + value*sizes[size];

   return(0);
}

int do_DC(void) {

   int32_t value;
   int length=0;

   reset_instrn_buf();

#ifdef LABELS
   if ((label != NULL) &&                 /* label and */
         (pass == 1) &&                     /* pass 1 */
         !enter_symbol(label,initial_pc,seg_type[current_segment]))
      /* failed add to symbol table */
      asm_error(ERR_LABEL_MULTIPLY_DEFINED);
#endif

   do
   {
      if (*argptr=='\"') /* string constant ? */
      {
         argptr++; /* discard " */
         while ((*argptr != '\"') && (*argptr != '\0'))
         {
            if (*argptr == '\\')
            {
               argptr++;
               gen_byte((int8_t)esc_char(*argptr++));
            }
            else
               gen_byte((int8_t)(*argptr++));
            length++;
         }
         if (*argptr=='\"') /* eos found */
            argptr++;
         continue;
      }

      if (!exprnx(argptr,value))
      {
         asm_error(ERR_ILLEGAL_EXPRESSION);
         break;
      }
      switch (size)
      {
         case BYTE_SIZE_IDX : gen_byte((int8_t)value);
         length++;
         break;
         case WORD_SIZE_IDX : gen_word((int16_t)value);
         length += 2;
         break;
         case LONG_SIZE_IDX : gen_long(value);
         length += 4;
         break;
      }
   }
   while(*argptr++ == ',');

   return(length);
}

int do_EQU(void) {

   int32_t value;

   reset_instrn_buf();

   list_delimiter = '=';

   if (label == NULL) /* no label */
      asm_error(ERR_LABEL_REQUIRED);

   if (!exprnx(argptr,value)) /* illegal exprn */
   {
      asm_error(ERR_ILLEGAL_EXPRESSION);
      return(0);
   }

   gen_value(value);

   if ((pass == 1) &&                /* pass 1, but */
         !enter_symbol(label,value,ABS_SYM))
      /* failed add to symbol table */
      asm_error(ERR_LABEL_MULTIPLY_DEFINED);
   return(0);
}

int do_REG(void) {

   uint16_t Regs;

   reset_instrn_buf();

   list_delimiter = '=';

   if (label == NULL) /* no label */
      asm_error(ERR_LABEL_REQUIRED);

   if (!parse_reg_list(&argptr,&Regs))
   {
      asm_error(ERR_ILLEGAL_EXPRESSION);
      return(0);
   }

   gen_value(Regs);

   if ((pass == 1) &&                /* pass 1, but */
         !enter_symbol(label,Regs,ABS_SYM))
      /* failed add to symbol table */
      asm_error(ERR_LABEL_MULTIPLY_DEFINED);

   return(0);
}

int do_GLOBAL(void) {

   char *name;

   reset_instrn_buf();

   if (label != NULL) /* no label */
      asm_error(ERR_LABEL_NOT_ALLOWED);

   if (pass == 1)
      do
      {
         if ((name=parse_symbol(argptr)) == NULL)
         {
            asm_error(ERR_ILLEGAL_LABEL);
            return(0);
         }
         make_extern_symbol(name);
      }
      while (*argptr++ == ',');
   return(0);
}

int do_EXTERN(void) {

   char *name;

   reset_instrn_buf();

   if (label != NULL) /* no label */
      asm_error(ERR_LABEL_NOT_ALLOWED);

   if (pass == 1)
      do
      {
         if ((name=parse_symbol(argptr)) == NULL)
         {
            asm_error(ERR_ILLEGAL_LABEL);
            return(0);
         }
         make_extern_symbol(name);
      }
      while (*argptr++ == ',');

   return(0);
}

int do_END(void) {

   int32_t value;

   reset_instrn_buf();

   end_of_source = true;       /* don't go beyond END */

   value=0;

   if ((argptr != NULL) && (exprn(argptr,value)<=0))
   {
      asm_error(ERR_ILLEGAL_EXPRESSION);
      return(0);
   }

   if (pass==2)     /* write start address load record */
      f_start(value);

#ifdef LABELS
   if ((label != NULL) &&             /* label and */
         (pass == 1) &&                     /* pass 1, but */
         !enter_symbol(label,initial_pc,seg_type[current_segment]))
      /* failed add to symbol table */
      asm_error(ERR_LABEL_MULTIPLY_DEFINED);
#endif //  LABELS

   return(0);
}

int do_ORG(void) {

   int32_t value;

   if (exprn(argptr,value)<=0)
   {
      asm_error(ERR_ILLEGAL_EXPRESSION);
      return(0);
   }

   initial_pc = value;
   reset_instrn_buf();

#ifdef LABELS
   if ((label != NULL) &&            /* label and */
         (pass == 1) &&                     /* pass 1, but */
         !enter_symbol(label,initial_pc,seg_type[current_segment]))
      /* failed add to symbol table */
      asm_error(ERR_LABEL_MULTIPLY_DEFINED);
#endif //  LABELS

   return(0);
}

int do_TEXT(void) {

   if (current_segment != TEXT_SEG)
   {
      segment_pc[current_segment] = initial_pc;
      current_segment = TEXT_SEG;
      initial_pc = segment_pc[TEXT_SEG];
   }

   reset_instrn_buf();

   if (label != NULL) /* no label */
      asm_error(ERR_LABEL_NOT_ALLOWED);

   return(0);
}

int do_DATA(void) {

   if (current_segment != DATA_SEG)
   {
      segment_pc[current_segment] = initial_pc;
      current_segment = DATA_SEG;
      initial_pc = segment_pc[DATA_SEG];
   }

   reset_instrn_buf();

   if (label != NULL) /* no label */
      asm_error(ERR_LABEL_NOT_ALLOWED);

   return(0);
}

/*
  Looks up mnemonic in mnemonic table after stripping off size.
  Assumes mnemonic is in upper case.
 */
static op_entry *look_up_mnemonic(char *mnemonic, int &size) {

   op_entry *entry;
   char *mn_size;

   if (strcmp(mnemonic,"rts") == 0) {
      printf("Found it\n");
   }
   mnemonic = strtok(mnemonic,"."); /* base mnemonic */
   mn_size = strtok(NULL,"");       /* rest is size */

   if (mn_size == NULL)
   {
      size_given = false; /* flag size extension given */
      size = DEF_SIZE;   /* no size given - use default */
   }
   else
   {
      size_given = true; /* flag size extension given */
      if (strlen(mn_size) != 1) /* size must be a single letter */
      {
         asm_error(ERR_ILL_SIZE);
         return(NULL);
      }
      switch(*mn_size)
      {
         case 'b' :
         case 'B' : size = BYTE_SIZE;
         break;
         case 'w' :
         case 'W' : size = WORD_SIZE;
         break;
         case 'l' :
         case 'L' : size = LONG_SIZE;
         break;
         default  : asm_error(ERR_ILL_SIZE);
         return(NULL);
      }
   }
   /*printf("%s = %s\n", entry->mnemonic,mnemonic);*/
   for (entry=op_info; entry->mnemonic != NULL; entry++)
      if (strcasecmp(entry->mnemonic,mnemonic) == 0) {/* found it ! */
         if (mn_size != NULL) /* size given ? */
            *(mn_size-1)='.';  /* put back size */
         if (size == DEF_SIZE) { /* determine default size */
            if (entry->size & WORD_SIZE)      /* try word size */
               size = WORD_SIZE;
            else if (entry->size & BYTE_SIZE) /* try byte size */
               size = BYTE_SIZE;
            else if (entry->size & LONG_SIZE) /* try byte size */
               size = LONG_SIZE;
            else
               size = NO_SIZE; /* unsized */
         }
         else
            if (!(size & entry->size)) /* check if allowed size */
            {
               asm_error(ERR_ILL_SIZE);
               return(NULL);
            }
         switch (size) /* change size to size index */
         {
            case BYTE_SIZE : size = BYTE_SIZE_IDX;
            break;
            case WORD_SIZE : size = WORD_SIZE_IDX;
            break;
            case LONG_SIZE : size = LONG_SIZE_IDX;
            break;
            case NO_SIZE   : size = NO_SIZE_IDX;
            break;
            default        : abort(); /* abort prog */
         }
         return(entry);
      }

   return(NULL);
}

/*
 Parse the line into global variables

 Variable  : Pointer to
  ============================
 label     : label field
 mnemonic  : mnemonic field
 args      : argument field
 comment   : comment field

 Variables are NULL if field is not present
 */
void parse_line(char *line) {

   char *tmp;
   int  in_string=0;
   int  in_char=0;
   int  esc_found=0;

   label=mnemonic=args=comment=NULL;

   const char *linePtr = line;

#ifdef LABELS
   if (!isspace(*line) && (*line != ';')) /* must be label */
   {
      int ok_label;

      label = line;
      ok_label = (parse_symbol(linePtr) != NULL);
      line = const_cast<char *>(linePtr);
      if (*line == ':')         /* ignore ':' after label */
         *line++ = '\0';
      if (!ok_label ||                        /* illegal label or */
            (!isspace(*line) && (*line != ';'))) /* not followed by WS, ... ? */
      {
         label = NULL;
         asm_error(ERR_ILLEGAL_LABEL);
      }
      while (!isspace(*line) &&  /* find white space, eol or comment */
            (*line != '\0') && (*line != ';'))
         line++;
      if ((*line != '\0') && (*line != ';'))
         *line++ = '\0'; /* terminate label */
   }
#endif //  LABELS

   while (isspace(*line) && (*line != '\0'))  /* skip white space */
      line++;

   if (*line == '\0')
      return;

   if (*line == ';')
   {
      *line++ = '\0';
      comment = strtok(line,"\r\n"); /* rest if comment */
      return;
   }

   mnemonic = line; /* next field if mnemonic */

   while (!isspace(*line) && (*line != '\0')) /* skip to end of mnemonic */
      line++;

   if (*line != '\0')
      *line++ = '\0'; /* terminate mnemonic */

   while (isspace(*line) && (*line != '\0'))  /* skip white space */
      line++;

   if (*line == '\0')
      return;

   if (*line == ';')
   {
      *line++ = '\0';
      comment = strtok(line,"\r\n"); /* rest is comment */
      return;
   }

   args = line; /* start of args */

   /*
    **  Locate end of arg field i.e EOL or start of comment.
    **  Handles string and character constants.
    */
   for(tmp=line;; tmp++)
   {
      switch (*tmp)
      {
         case '\\' :  /* escape char if in string or char */
            if (in_string || in_char)
            {
               esc_found = 1; /* escape following char */
               continue;
            }
            break;

         case '\"' :  /* start/end of string if not in character or escaped */
            if (!in_char && !esc_found)
               in_string = !in_string;
            break;

         case '\'' :  /* start/end of character if not in string or escaped */
            if (!in_string && !esc_found)
               in_char = !in_char;
            break;

         case ';' :  /* start of comment */
            if (!in_string && !in_char)
            {
               *tmp = '\0';  /* terminate args */
               comment = strtok(tmp+1,"\r\n"); /* rest is comment */
            }
            break;

         case '\r' :
         case '\n' :  /* end of line */
            *tmp = '\0';
            break;
      }
      esc_found = 0;
      if (*tmp == '\0')
         break;
   }

   if (in_string || in_char) /* unterminated string or character constant */
   {
      asm_error(ERR_UNTERMINATED_STRING_OR_CHAR);
      return;
   }
}

void set_pass1(void) {

   int seg;

   for (seg=0; seg<=LAST_SEG; seg++)
      segment_pc[seg]=0; /* pcs for each segment */
   current_segment=TEXT_SEG;
   clear_symbol_table();
   initial_pc = 0;
   pass = 1;
   err_pass1 = 0;
   end_of_source = false;
}

void set_pass2(void) {

   int seg;

   for (seg=0; seg<=LAST_SEG; seg++)
      segment_pc[seg]=0; /* pcs for each segment */
   current_segment=TEXT_SEG;
   initial_pc = 0;
   pass = 2;
   err_pass2 = 0;
   end_of_source = false;
}

int report_error_count(void) {

   if (err_pass1>0)
      fprintf(stderr,"%d Errors detected in pass 1\n",err_pass1);
   if (war_pass1>0)
      fprintf(stderr,"%d Warnings detected in pass 1\n",war_pass1);
   if (err_pass2>0)
      fprintf(stderr,"%d Errors detected in pass 2\n",err_pass2);
   if (war_pass2>0)
      fprintf(stderr,"%d Warnings detected in pass 2\n",war_pass2);

   fprintf(listfile,"\n\n%d Errors detected in pass 1\n",err_pass1);
   fprintf(listfile,"%d Warnings detected in pass 1\n",war_pass1);
   fprintf(listfile,"%d Errors detected in pass 2\n",err_pass2);
   fprintf(listfile,"%d Warnings detected in pass 2\n",war_pass2);

   return(err_pass1+err_pass2+war_pass1+war_pass2);
}

/**
 *  Assembles the instruction in 'line'
 *
 *  Pass 1.
 *
 *  @return == 0  => asm_error or no code generated
 *  @return != 0  => length of instruction generated
 *  @return == -1 => END pseudo op detected
 *
 *  @note Return value does not include alignment bytes.
 */
int assem1(char *line) {
   int instrn_length = 0;

   clear_instrn_buf();
   size = DEF_SIZE;

   /*
     break line into label, mnemonic & args
    */
   parse_line(line);
   argptr = args;

   if (mnemonic == NULL) /* empty line ? */
   {
      if ((label != NULL) &&                 /* label but */
            !enter_symbol(label,initial_pc,seg_type[current_segment]))
         /* failed add to symbol table */
         asm_error(ERR_LABEL_MULTIPLY_DEFINED);
      return(0);
   }

   entry = look_up_mnemonic(mnemonic, size);

   if (entry != NULL) {/* found in mnemonic table ? */
      if (entry->ea_mask != PSEUDO_OP) { /* force word alignment for ins'ns */
         align(2);
         clear_instrn_buf();
      }

      if ((label != NULL) &&                 /* label and */
            (entry->ea_mask != PSEUDO_OP) &&   /* not pseudo-op and */
            !enter_symbol(label,initial_pc,seg_type[current_segment]))
         /* failed add to symbol table */
         asm_error(ERR_LABEL_MULTIPLY_DEFINED);

      instrn_length = entry->clazz(); /* assemble line */
   }
   else {
      if ((label != NULL) &&                 /* label and */
            !enter_symbol(label,initial_pc,seg_type[current_segment]))
         /* failed add to symbol table */
         asm_error(ERR_LABEL_MULTIPLY_DEFINED);
      return(0);
   }

   if (!err_flag)
      initial_pc = current_pc;

   return(end_of_source?-1:instrn_length);
}

/**
 *  Assembles the instruction in 'line'
 *
 *  If a syntax error is detected then no code is generated.
 *
 *  @return == 0  => error or no code generated
 *  @return != 0  => length of instruction generated
 *  @return == -1 => END pseudo op detected (only asm)
 *
 *  @note Return value does not include alignment bytes.
 */
int assem2(char *line) {
   int instrn_length = 0;
   int32_t value;

   clear_instrn_buf();
   size = DEF_SIZE;
   /*
     break line into label, mnemonic & args
    */
   parse_line(line);
   argptr = args;

   if (mnemonic == NULL) /* empty line */
   {
      if (label != NULL)                /* label */
      {
         symbol_value(label, value);  /* check value == initial_pc */
         if ((uint32_t)value != initial_pc)
         {
            asm_error(ERR_PHASING);             /* phasing error */
            initial_pc = value;     /* resynch to avoid multiple reporting */
         }
      }
      reset_instrn_buf();
      print_line(listfile);
      return(0);
   }

   entry = look_up_mnemonic(mnemonic, size);

   if (entry != NULL) /* found in mnemonic table ? */
   {
      if ((entry->ea_mask != PSEUDO_OP) && /* check word alignment for ins'ns */
            (initial_pc & 0x1))
      {
         asm_error(ERR_ALIGNMENT);
         align(2); /* force alignment */
         clear_instrn_buf();
      }
      if ((label != NULL) &&                /* label and */
            (entry->ea_mask != PSEUDO_OP))    /* not pseudo op and */
      {
         symbol_value(label, value);  /* check value == initial_pc */
         if ((uint32_t)value != initial_pc)
         {
            asm_error(ERR_PHASING);             /* phasing error */
            initial_pc = value;     /* resynch to avoid multiple reporting */
            clear_instrn_buf();
         }
      }

      instrn_length = entry->clazz();
   }
   else
      asm_error(ERR_UNKNOWN_MNEMONIC);

   print_line(listfile);

   if (!err_flag && (instrn_length > 0)) /* any bytes generated ? */
      flush_instrn_buf();  /* write them */

   if (!err_flag)
      initial_pc = current_pc;

   return(end_of_source?-1:instrn_length);
}

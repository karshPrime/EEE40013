/*
 **  symbol.c
 */
#include <stdlib.h>     /* qsort */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "symbol.h"

#define MAX_SYMBOL (400)

/**
 * Symbol table entry
 */
struct sym_entry {
   char       *name;       ///< Symbol name
   int32_t     value;      ///< Symbol value
   entry_type  type;       ///< Symbol type
};

static sym_entry symbol_table[MAX_SYMBOL];

static int sym_count = 0;

/*
   Table of reserved words
 */
static const char *res_words[] = {
      "SP","CCR",
      "R0","R1","R2","R3","R4","R5","R6","R7",
      "R8","R9","R10","R11","R12","R13","R14","R15",
      "R16","R17","R18","R19","R20","R21","R22","R23",
      "R24","R25","R26","R27","R28","R29","R30","R31",

      nullptr,
};

/*
 * Checks if name is a reserved word
 *
 * @param name Name to check
 *
 * @return true if reserved word
 */
static int is_resword(const char *name) {

   const char **symbol_ptr;

   for (symbol_ptr = res_words; *symbol_ptr != NULL; symbol_ptr++)
      if (strcasecmp(*symbol_ptr, name)==0)
         return(1);
   return(0);
}

/*
   Parses a symbol.

   1st char in    [A-Z,a-z,_,]
   later chars in [A-Z,a-z,_,$,%,0-9]

   Returns :  == NULL : illegal symbol (illegal char or reserved word)
                        arg is unaffected
              != NULL : valid symbol ptr
                        arg advanced
 */
char *parse_symbol(const char *&arg) {

   static constexpr unsigned MAX_IDENTIFIER = 100;

   static char buff[MAX_IDENTIFIER];
   char *bptr=buff;
   const char *ptr=arg;

   if (!isalpha(*ptr) && (*ptr != '_'))
      return(NULL);

   while (isalnum(*ptr) || (*ptr == '_') ||
         (*ptr == '$') || (*ptr == '%'))
      *bptr++ = *ptr++;
   *bptr = '\0';

   if (is_resword(buff)) {
      /* reserved word ? */
      return(NULL);
   }
   arg=ptr;
   return(buff);
}

/**
 * Check for symbol equality
 *
 * @param s1
 * @param s2
 *
 * @return
 */
int sym_comp(const void *s1, const void *s2) {

   return(strcmp(((sym_entry*)s1)->name,((sym_entry*)s2)->name));
}

/**
 *   Prints out a sorted symbol table
 *
 * @param lstfile
 */
void print_symbol_table(FILE *lstfile) {
   sym_entry *symbol_ptr;
   char type[10];

   qsort(symbol_table,sym_count,sizeof(sym_entry),sym_comp);

   fprintf(lstfile, "\n\n  Symbol Table\n"
         "*******************************************************\n"
         "  Value  : Type  : Symbol \n");
   for (symbol_ptr = symbol_table; symbol_ptr->name != NULL; symbol_ptr++)
   {
      switch ((symbol_ptr->type)&0xFFFE)
      {
         case UND_SYM   : strcpy(type,"Und "); break;
         case ABS_SYM   : strcpy(type,"Abs "); break;
         case TEXT_SYM  : strcpy(type,"Text"); break;
         case DATA_SYM  : strcpy(type,"Data"); break;
         case BSS_SYM   : strcpy(type,"Bss "); break;
         case COM_SYM   : strcpy(type,"Comm"); break;
         default        : strcpy(type,"Und "); break;
      }
      if (symbol_ptr->type & EXTERN_SYM) /* extern */
         strcat(type,"_E");
      fprintf(lstfile,"%8.8X : %5.5s : %s\n",
            symbol_ptr->value,
            type,
            symbol_ptr->name
      );
   }
   fprintf(lstfile, "*******************************************************\n");
}

void clear_symbol_table(void)
{
}

/**
 *  Looks up a given symbol by name.
 *
 * @param name
 *
 * @return Ptr to a symbol entry. A new one will be created if necessary.
 */
static sym_entry *lookup_symbol(const char *name) {
   sym_entry *symbol_ptr;

   for (symbol_ptr = symbol_table; symbol_ptr->name != NULL; symbol_ptr++)
      if (strcmp(symbol_ptr->name,name)==0)
         break;

   if (symbol_ptr->name==NULL) /* not found ? - create new entry */
   {
      sym_count++;
      symbol_ptr->name=strdup(name);
      symbol_ptr->value=0;
      symbol_ptr->type=UND_SYM;
   }

   return(symbol_ptr);
}

/*
 */
/**
 * Enter symbol in symbol table
 *
 * @param name
 * @param value
 * @param type
 *
 * @return false : Previously defined symbol
 * @return true  : Newly defined symbol, value has value
 */
int enter_symbol(const char *name, int32_t value, entry_type type) {
   sym_entry *symbol_ptr;

   symbol_ptr = lookup_symbol(name);

   if (((symbol_ptr->type)&SYM_CLASS) != UND_SYM) {
      /* already defined ? */
      return (false);
   }

   symbol_ptr->value = value; /* enter data fields */
   symbol_ptr->type  = type;

   return(true);
}

/**
 *
 * @param name
 *
 * @return 1 : defined symbol, value has value
 */
bool make_extern_symbol(const char *name) {

   sym_entry *symbol_ptr;

   symbol_ptr = lookup_symbol(name);

   symbol_ptr->type  = (entry_type)(symbol_ptr->type|EXTERN_SYM);

   return(true);
}

/**
 *  @return   false : undefined symbol
 *  @return   true  : defined symbol, value has value
 */
bool symbol_value(const char *name, int32_t &value) {
   sym_entry *symbol_ptr;

   symbol_ptr = lookup_symbol(name);

   if (((symbol_ptr->type)&SYM_CLASS) == UND_SYM) /* undefined ? */
   {
      value = 1;
      return (false);
   }

   value = symbol_ptr->value;
   return(true);
}

/*
   symbol.h
*/
#include <stdint.h>

void  clear_symbol_table(void);

/**
 *   Prints out a sorted symbol table
 *
 * @param lstfile
 */
void  print_symbol_table(FILE *);

/**
   Parses a symbol.

   1st char in    [A-Z,a-z,_,]
   later chars in [A-Z,a-z,_,$,%,0-9]

   Returns :  == NULL : illegal symbol (illegal char or reserved word)
                        arg is unaffected
              != NULL : valid symbol ptr
                        arg advanced
 */
char  *parse_symbol(const char *&arg);

/**
 *
 * @param name
 *
 * @return 1 : defined symbol, value has value
 */
bool   make_extern_symbol(const char *name);

/**
 *  @return   false : undefined symbol
 *  @return   true  : defined symbol, value has value
 */
bool   symbol_value(const char *name, int32_t &value);

typedef enum {UND_SYM=0,
	      ABS_SYM=2,
	      TEXT_SYM=4,
	      DATA_SYM=6,
	      BSS_SYM=8,
	      COM_SYM=12,

	      EXTERN_SYM=1}  entry_type;

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
int   enter_symbol(const char *name, int32_t value, entry_type type);

static constexpr unsigned SYM_CLASS = 0xFFFE;

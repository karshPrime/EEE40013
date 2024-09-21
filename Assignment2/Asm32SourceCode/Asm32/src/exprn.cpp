/*
**   exprn.c
*/
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "exprn.h"
#include "symbol.h"

#undef DEBUG /* define for standalone testing */

static int32_t star_value=0;              /* value '*' has in expressions */

static unsigned default_radix=10;     /* default radix for numbers */

static bool defined_expression=true; /* set false if undefined ident found */

char esc_char(char ch) {
  switch(ch)
    {
    case 'n' : return('\n');
    case 't' : return('\t');
    case 'v' : return('\v');
    case 'b' : return('\b');
    case 'r' : return('\r');
    case 'f' : return('\f');
    case 'a' : return('\a');
    case '0' : return('\0');
    default  : return(ch);
    }
}

static int expression(const char *&ptr, int32_t &value);

/*
  Sets value '*' has in expressions.

  Usually means the current location (not the same as the current PC)
*/
void set_star_value(int32_t address) {

  star_value = address;
}

/*
   Converts a digit to its numeric equivalent.

   Accepts digits or hexadecimal characters.

   Input : ch = char to convert

   Return : >=0 => numeric value of character
	    -1  => invalid character
*/
static int convert_digit(char ch) {

int digit=-1;

  if ((ch >= '0') && (ch <= '9'))
    digit = (ch - '0');
  else if ((ch >= 'a') && (ch <= 'f'))
    digit = (ch - 'a' + 10);
  else if ((ch >= 'A') && (ch <= 'F'))
    digit = (ch - 'A' + 10);
  return(digit);
}

/*
   Parses a simple decimal number.
*/
int parse_num(const char *&arg) {
int value = 0;

  while (isdigit(*arg))
    {
    value *= 10;
    value += *arg++ - '0';
    }
  return(value);
}

/*
  Parses an unsigned number
*/
static int unsigned_number(const char *&ptr, int32_t &value) {
unsigned radix = default_radix;
int32_t digit;
int digit_found=false;
int ch_count;
int ch;

  switch (*ptr) /* trap special cases */
    {
    case '(' : /* sub expression */
	       ptr++;               /* discard '(' */
	       if (!expression(ptr,value))  /* get sub expression */
		 return(0);            /* failed ! */
	       if (*ptr++ != ')')   /* check & discard matching ')' */
		 return(0);            /* failed ! */
	       return (1);             /* ok - thats it ! */
    case '\'' : /* character */
	       ptr++;               /* discard ' */
	       value = 0;
	       for(ch_count=0; ((ch=*ptr++) != '\'') && (ch_count++<4); ) {
             if (ch == '\\')     /* escape character */
                ch = esc_char(*ptr++);
             value <<= 8;
             value += ch;       /* get the character */
             }
	       return ((ch_count>0)?1:0); /* ok - that's it ! */
    case '*' : /* current location */
	       /* or whatever that means for a particular command */
	       ptr++;
	       value = star_value;
	       return (1);          /* OK - got a valid number */
    case '$' : /* radix 16 */
	       ptr++;
	       radix = 16;
	       digit_found = true;
	       break;
    case '&' : /* radix 10 */
	       ptr++;
	       radix = 10;
	       digit_found = true;
	       break;
    case '@' : /* radix 8 */
	       ptr++;
	       radix = 8;
	       digit_found = true;
	       break;
    case '%' : /* radix 2 */
	       ptr++;
	       radix = 2;
	       digit_found = true;
	       break;
    case '0' : /* hex number ? i.e. 0xddd */
	       if ((*(ptr+1) == 'x') || (*(ptr+1) == 'X')) {
             radix = 16;
             ptr += 2;
             digit_found = true;
             }
          break;
     }

#ifdef LABELS
  if (!digit_found && !isdigit(*ptr)) /* not a number */
    {
    char *name = parse_symbol(ptr);   /* try a symbol */
    if (name == NULL) /* invalid symbol */
      return(0);
    defined_expression &=symbol_value(name,value);
    return(1); /* valid expression even if undefined */
    }
#endif

  if (((digit = convert_digit(*ptr)) < 0) || /* must have at least 1 digit */
      (digit >= (int32_t)radix))
    return(0);

  value = 0;

  while (((digit = convert_digit(*ptr)) >= 0) && /* get number */
	 (digit < (int32_t)radix))
    {
    value *= radix;
    value += digit;
    ptr++;        /* discard digit */
    }

  return(1);         /* OK got a valid number */
}

/*
  Parses a signed number

    number ::= '-' number |
          unsigned_number
*/
static int number(const char *&ptr, int32_t &value) {

  value = 0;
  if (*ptr == '-') /* check for sign */
    {
    ptr++;                /* discard '-' */
    if (!number(ptr,value))  /* recursively handle other '-'s */
      return(0);             /* failed ! */
    value = -value;        /* flip sign */
    return(1);
    }

  return(unsigned_number(ptr,value)); /* must be unsigned number */
}

/*
  Parses a term

	 term ::= number ['*' number | '/' number]*
*/
static int term(const char *&ptr, int32_t &value) {

int32_t left_value;
int32_t right_value;
char op;

  if (!number(ptr,left_value)) /* get left value */
    return(0);

  while ((*ptr == '*') || (*ptr == '/')) /* mulop ? */
    {
    op = *ptr++;                /* save it and advance */
    if (!number(ptr,right_value)) /* get right value */
      return(0);                   /* failed ! */
    if (op == '*')                 /* evaluate 'value mulop value' */
      left_value *= right_value;
    else
      {
      if (right_value == 0) /* divide by zero */
	return(0);
      left_value /= right_value;
      }
    }

  value = left_value;
  return(1);
}

/*
  Parses an expression

  Note : spaces are not skipped.

	 expression ::= term ['+' term | '-' term]*

  Returns : 0 => failed (illegal expression)
	    1 => success : value = expression value
			   *ptr is advanced past expression
*/
static int expression(const char *&ptr, int32_t &value) {

int32_t left_value;
int32_t right_value;
char op;

  if (!term(ptr,left_value)) /* get left term */
    return(0);                /* failed ! */

  while ((*ptr == '+') || (*ptr == '-')) /* check for addop */
    {
    op = *ptr++;               /* save it and advance */
    if (!term(ptr,right_value))  /* get right term */
      return(0);                  /* failed */
    if (op == '+')                /* evaluate 'term addop term' */
      left_value += right_value;
    else
      left_value -= right_value;
    }

  value = left_value;
  return(1);
}

/*
  Parses an expression

  Note : leading spaces are skipped.

  Returns :
    == -1  => failed  : (illegal expression)
		       *ptr is advanced to error location
		       *value = expression value;
		       *value
    == 0  => failed  : (valid expression that contains
			forward reference)
		       *ptr is advanced past expression
		       *value = 0;
    == 1  => success : value = expression value
		       *ptr is advanced past expression
*/
int exprn(const char *&ptr, int32_t &value) {

  while (isspace(*ptr)) /* skip leading white space */
    ptr++;
  if (*ptr == '\0') /* empty line ? */
    return(-1);

  defined_expression = true;  /* set up for defined expression */
  return(expression(ptr, value)?(defined_expression?1:0):-1);
}

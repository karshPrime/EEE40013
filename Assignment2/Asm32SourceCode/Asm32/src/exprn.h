/************************/
/******  exprn.h  *******/
/************************/

#include <stdint.h>

/*
  Converts an escape char to correct value.

  e.g. used for '\n' ==> carriage return char

  Entry : character following '\'
  Exit  : converted character
*/
extern char esc_char(char ch);

/*
   Parses a simple decimal number.
*/
extern int parse_num(char const *&arg);

/*
  Sets value '*' has in expressions.

  Usually means the current location (not the same as the current PC)
*/
extern void set_star_value(int32_t address);

/*
  Evaluates an expression

  Note : leading spaces are skipped.


    Expressions may contain the following operators :

                                          precedence  associativity
                ()  subexpression         highest
                -   unary minus                       right-to-left
                *   multiplication                    left-to-right
                /   division                          left-to-right
                +   addition                          left-to-right
                -   subtraction           lowest      left-to-right

    The following values may be used :

               *      value of star (usually the current location)
               nnn    decimal value
               $hhh   hexadecimal value
               %bbb   binary value
               @ooo   octal value
               &ddd   decimal value

    All arithmetic is done with signed 32 bit values.

  Entry   : *arg = ptr to expression string

  Returns : == -1  => failed (illegal expression)
                        *ptr  = advanced to error location
 The following only occurs if LABELS is #defined
            == 0 => failed - undefined expression
                        *ptr  = advanced to end of expression
                        value = expression value
            == 1  => success
                        *ptr  = advanced to end of expression
                        value = expression value
*/
extern int exprn(char const *&ptr, int32_t &value);
/*
  Parses an (optional) expression

  Note : leading spaces are skipped.

  Returns :
    == -1 => failed  : (illegal expression)
                       *ptr is advanced to error location
                       *value = expression value;
    == 0  => failed : empty line
    == 1  => success : value = expression value
                       *ptr is advanced past expression
*/
int optexprn(const char *&ptr, int32_t &value);

/*
  Sets default radix for numbers in expressions
*/
int set_radix(int r);


#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#include "poly.h"
#include "poly_lib.h"
#include "parse.h"


/*
 * <poly> ::= <mono-list> | <coeff>
 * <mono-list> ::= <mono> { + <mono-list> }
 * <mono> ::= ( <poly>, <exp> )
 *
 * <coeff> ::= -9223372036854775808..9223372036854775807
 * <exp> ::= 0..2147483647
 */

/* static long ReadNumber(const char* s) {
 *   char* err;
 *   long res = strtol(src, &strto_err, 10);
 * } */

void RangeError(void);

/* to na razie czyta jedynie listę, jak odróżnić koef? */
bool ParsePoly(char* src, char** err, Poly* p)
{

  char* strto_err;
  poly_coeff_t c;
  bool mono_read;
  Mono m;

  /* przypadek pierwszy -- wielomian współczynnny */
  if (isdigit(*src) || *src == '-') {
    c = strtol(src, &strto_err, 10);

    if (errno == ERANGE) {
      RangeError();
      return false;
    }

    if (*strto_err != '\0' && *strto_err != ',' && *strto_err != '\n')
      return false;

    *p = PolyFromCoeff(c);
    return true;
  }

  *p = PolyZero();

  while (*src != '\n' && *src != ',' && *src != '\0') {
    if (*src == '+' || isspace(*src) || *src != '(') {
      ++*src;
    }

    mono_read = ParseMono(src, err, &m);

    if (!mono_read) {
      PolyDestroy(p);
      MonoDestroy(&m);
      return false;
    }

    MonoListInsert(&p->list, &m);
    src = *err;
  }

  return true;
}

bool ParseMono(char* src, char** err, Mono* m)
{
  bool poly_read;
  Poly p;
  long e;
  char* strto_err;
  assert(*src == '(');

  poly_read = ParsePoly(src + 1, err, &p);

  if (!(**err == ',' && poly_read)) {
    /* błond */
    return false;
  }

  src = *err + 1;
  e = strtol(src, &strto_err, 10);
  if (errno == ERANGE || e > 2147483647 || e < 0) {
    RangeError();
    PolyDestroy(&p);
    return false;
  }

  *m = MonoFromPoly(&p, e);
  return true;
}

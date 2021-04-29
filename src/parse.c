
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

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

void RangeError(void)
{
  fprintf(stderr, "range!\n");
  errno = 0;
}

/* TODO -- self ewidentne */
static bool ParsePolyCoeff(char* src, char** err, Poly* p)
{
  assert(isdigit(*src) || *src == '-');

  char* strto_err;
  poly_coeff_t c;

  c = strtol(src, &strto_err, 10);

  if (errno == ERANGE) {
    RangeError();
    return false;
  }

  if (*strto_err != '\0' && *strto_err != ',' && *strto_err != '\n')
    return false;

  *err = strto_err;
  *p = PolyFromCoeff(c);
  return true;
}

/* to na razie czyta jedynie listę, jak odróżnić koef? */
bool ParsePoly(char* src, char** err, Poly* p)
{
  bool mono_read;
  Mono m;
  *err = src;

  /* przypadek pierwszy -- wielomian współczynnny */
  if (isdigit(*src) || *src == '-')
    return ParsePolyCoeff(src, err, p);

  *p = PolyZero();

  while (*src != '\n' && *src != ',' && *src != '\0') {
    if (*src == '+' || isspace(*src) || *src != '(') {
      ++src;
      continue;
    }

    mono_read = ParseMono(src, err, &m);

    if (!mono_read) {
      PolyDestroy(p);
      return false;
    }

    MonoListInsert(&p->list, &m);

    assert(**err == ')');
    src = ++*err;
  }

  if (PolyIsPseudoCoeff(p->list))
    Decoeffise(p);

  return true;
}

bool ParseMono(char* src, char** err, Mono* m)
{
  bool poly_read;
  Poly p;
  long e;
  char* strto_err;

  *err = src;                   /* to wywołane z ParsePoly so no need */
  assert(*src == '(');

  poly_read = ParsePoly(src + 1, err, &p);

  if (!(**err == ',' && poly_read)) {
    /* błond */
    PolyDestroy(&p);
    return false;
  }

  src = *err + 1;
  e = strtol(src, &strto_err, 10);

  if (errno == ERANGE || e > 2147483647 || e < 0) {
    RangeError();
    PolyDestroy(&p);
    return false;
  }

  *err = strto_err;
  *m = MonoFromPoly(&p, e);
  return true;
}

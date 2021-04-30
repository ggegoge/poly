
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "poly.h"
#include "poly_lib.h"
#include "stack_op.h"
#include "parse.h"


/*
 * <poly> ::= <mono-list> | <coeff>
 * <mono-list> ::= <mono> { + <mono-list> }
 * <mono> ::= ( <poly>, <exp> )
 *
 * <coeff> ::= -9223372036854775808..9223372036854775807
 * <exp> ::= 0..2147483647
 */

/* TODO -- self ewidentne */
static bool ParsePolyCoeff(char* src, char** err, Poly* p)
{
  if (!(isdigit(*src) || *src == '-'))
    return false;


  char* strto_err;
  poly_coeff_t c;

  c = strtol(src, &strto_err, 10);

  if (errno == ERANGE)
    return false;


  if (*strto_err != '\0' && *strto_err != ',' && *strto_err != '\n')
    return false;

  *err = strto_err;
  *p = PolyFromCoeff(c);
  return true;
}

static bool ParseMono(char* src, char** err, Mono* m);

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

static bool ParseMono(char* src, char** err, Mono* m)
{
  bool poly_read;
  Poly p;
  long e;
  char* strto_err;

  *err = src;
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
    PolyDestroy(&p);
    return false;
  }

  *err = strto_err;

  if (**err != ')') {
    PolyDestroy(&p);
    return false;
  }

  *m = MonoFromPoly(&p, e);
  return true;
}


#define WHITE " \t\n\v\f\r"

void ErrorTraceback(size_t linum, char* s)
{
  fprintf(stderr, "ERROR %lu %s\n", linum, s);
}

static void ParseCommand(char* cmnd, char* arg, size_t linum,
                         struct Stack* stack);

void ParseLine(char* src, size_t linum, struct Stack* stack)
{
  Poly p;
  char* err;
  char* cmnd;
  char* arg;
  char* rest;
  size_t word_cnt;
  bool single_arg = true;

  if (isdigit(*src) || *src == '-' || *src == '(') {
    if (ParsePoly(src, &err, &p))
      PushPoly(stack, &p);
    else
      ErrorTraceback(linum, "WRONG POLY");

    return;
  }

  word_cnt = 0;
  cmnd = strtok(src, WHITE);
  arg = strtok(NULL, WHITE);
  rest = strtok(NULL, WHITE);

  if (strcmp(cmnd, "deg_by") == 0 || strcmp(cmnd, "deg_by") == 0)
    single_arg = false;

  if (rest || (single_arg && arg)) {
    ErrorTraceback(linum, "WRONG COMMAND");
    return;
  }

  ParseCommand(cmnd, arg, linum, stack);
}

static void ParseCommand(char* cmnd, char* arg, size_t linum,
                         struct Stack* stack)
{
  unsigned long long idx;
  poly_coeff_t x;
  char* err;
  
  if (strcmp(cmnd, "add") == 0) {
    Add(stack, linum);
  } else if (strcmp(cmnd, "mul") == 0) {
    Mul(stack, linum);
  } else if (strcmp(cmnd, "clone") == 0) {
    Clone(stack, linum);
  } else if (strcmp(cmnd, "neg") == 0) {
    Neg(stack, linum);
  } else if (strcmp(cmnd, "zero") == 0) {
    Zero(stack);
  } else if (strcmp(cmnd, "is_coeff") == 0) {
    IsCoeff(stack, linum);
  } else if (strcmp(cmnd, "is_zero") == 0) {
    IsZero(stack, linum);
  } else if (strcmp(cmnd, "sub") == 0) {
    Sub(stack, linum);
  } else if (strcmp(cmnd, "is_eq") == 0) {
    IsEq(stack, linum);
  } else if (strcmp(cmnd, "deg") == 0) {
    Deg(stack, linum);
  } else if (strcmp(cmnd, "print") == 0) {
    Print(stack, linum);
  } else if (strcmp(cmnd, "pop") == 0) {
    Pop(stack, linum);
  } else if (strcmp(cmnd, "deg_by") == 0) {
    idx = strtoull(arg, &err, 10);
    if (errno == ERANGE || *err != '\0') {
      errno = 0;
      ErrorTraceback(linum, "DEG BY WRONG VARIABLE");
    } else {
      DegBy(stack, idx, linum);
    }    
  } else if (strcmp(cmnd, "at") == 0) {
    x = strtol(arg, &err, 10);
    if (errno == ERANGE || *err != '\0') {
      errno = 0;
      ErrorTraceback(linum, "AT WRONG VALUE");
    } else {
      At(stack, x, linum);
    }
  } else {
    ErrorTraceback(linum, "WRONG COMMAND");
  }
}

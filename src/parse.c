/** @file
  Wczytywanie wielomianów bądź komend z postaci tekstowej.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/

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
 * BNF wielomianów:
 *
 * <poly> ::= <mono-list> | <coeff>
 * <mono-list> ::= <mono> { + <mono-list> }
 * <mono> ::= ( <poly>, <exp> )
 *
 * <coeff> ::= -9223372036854775808..9223372036854775807
 * <exp> ::= 0..2147483647
 */

/**
 * Wczytanie ze stringa @p src wielomianu stałego.
 * @param[in] src : string wejściowy
 * @param[out] err : wskazuje na pierwszy niewczytany znak z @p src
 * @param[out] p : wielomian pod którym zapisany zostaje ewentualny wynik
 * @return powodzenie wczytania -- czy na wejściu obecne były jedynie właściwe
 * znaki.
 */
static bool ParsePolyCoeff(char* src, char** err, Poly* p)
{
  if (!(isdigit(*src) || *src == '-'))
    return false;


  char* strto_err;
  poly_coeff_t c;

  c = strtol(src, &strto_err, 10);

  if (errno == ERANGE) {
    errno = 0;
    return false;
  }

  if (*strto_err != '\0' && *strto_err != ',' && *strto_err != '\n')
    return false;

  while (*strto_err == '\n') {
    ++strto_err;
  }

  *err = strto_err;
  *p = PolyFromCoeff(c);
  return true;
}

static bool ParseMono(char* src, char** err, Mono* m);

bool ParsePoly(char* src, char** err, Poly* p)
{
  size_t pluses = 0;
  Mono m;
  *err = src;

  /* przypadek pierwszy -- wielomian współczynnny */
  if (isdigit(*src) || *src == '-')
    return ParsePolyCoeff(src, err, p);

  if (*src != '(')
    return false;

  *p = PolyZero();

  while (*src != ',' && *src != '\0') {
    if (*src == '+')
      ++pluses;

    if (*src == '+' || *src == '\n') {
      ++src;
      continue;
    }

    /* po plusie musi nastąpić jednomian */

    if ((pluses != 0  && *src != '(') || pluses > 1 || !ParseMono(src, err, &m)) {
      PolyDestroy(p);
      return false;
    }

    if (!PolyIsZero(&m.p))
      MonoListInsert(&p->list, &m);

    assert(**err == ')');
    src = ++*err;
    pluses = 0;
  }

  if (pluses != 0) {
    PolyDestroy(p);
    return false;
  }

  *err = src;

  if (PolyIsPseudoCoeff(p->list))
    Decoeffise(p);

  return true;
}

/**
 * Wczytanie jednomianu z tekstu.
 * @param[in] src : tekst wejściowy
 * @param[out] err : wskazuje pierwszy niewczytany znak
 * @param[out] m : zapisany jednomian
 * @return czy udało się wczytać poprawny jednomian
 */
static bool ParseMono(char* src, char** err, Mono* m)
{
  Poly p;
  long e;
  char* strto_err;

  *err = src;
  assert(*src == '(');

  if (!(ParsePoly(src + 1, err, &p) &&** err == ',')) {
    /* błond */
    PolyDestroy(&p);
    return false;
  }

  src = *err + 1;

  /* po przecinku następuje dodatni wykładnik */
  if (!isdigit(*src)) {
    PolyDestroy(&p);
    return false;
  }

  e = strtol(src, &strto_err, 10);

  if (errno == ERANGE || e > 2147483647 || e < 0) {
    errno = 0;
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

/**
 * Wypisanie wyjątku wczytywaniowego na wyjście diagnostyczne.
 * @param[in] linum : numer wiersza, na którym znajduje się błąd
 * @param[in] s : treść błędu
 */
static void ErrorTraceback(size_t linum, char* s)
{
  fprintf(stderr, "ERROR %lu %s\n", linum, s);
}

static void ParseCommand(char* cmnd, char* arg, size_t linum,
                         struct Stack* stack);


/**
 * Sprawdza, czy dana komenda jest jedną z komend argumentowych. Sprawdza napis
 * @p src pod kątem jakichś spacji. W przypadku ich znalezienia zwraca `true`
 * i ustawia odpowiednio wskaźnik @p cmnd na początek komendy i @p arg na tenże
 * argument w stylu quasi-`strtok`owym -- ustawia znak '`\0`' na pozycji spacji
 * aby następna obróbka argumentu była prosta. Do tego dodaje taki null bajt
 * w miejscu `\n` co również ułatwia.
 * @param[in] src : napis
 * @param[in] len : jego długość
 * @param[out] arg : wskazuje na argument
 * @return czy znaleziono argument?
 */
static bool FindArg(char* src, size_t len, char** arg)
{
  bool res = false;

  for (size_t i = 0; i < len; ++i) {
    if (src[i] == ' ') {
      src[i] = '\0';
      /* komenda zaczyna się tuż po spacji */
      *arg = src + i + 1;
      res =  true;
    } else if (src[i] == '\n') {
      src[i] = '\0';
    }
  }

  return res;
}

void ParseLine(char* src, size_t len, size_t linum, struct Stack* stack)
{
  Poly p;
  char* err;
  char* cmnd = src;
  char* arg;

  if (isdigit(*src) || *src == '-' || *src == '(') {
    if (ParsePoly(src, &err, &p) && *err == '\0')
      PushPoly(stack, &p);
    else
      ErrorTraceback(linum, "WRONG POLY");

    return;
  }

  if (FindArg(src, len, &arg) && strcmp(cmnd, "deg_by") != 0 &&
      strcmp(cmnd, "at") != 0) {
    ErrorTraceback(linum, "WRONG COMMAND -- no args for this one");
    return;
  }

  ParseCommand(cmnd, arg, linum, stack);
}

/**
 * Wczytanie komendy @p cmnd z opcjonalnymi argumentami @p arg i wywołanie
 * stosownej operacji na stosie @p stack.
 * @param[in] cmnd : treść komendy
 * @param[in] arg : opcjonalnie -- argument komendy @p cmnd
 * @param[in] linum : numer obecnego wiersza
 * @param[in] stack : stos kalkulacyjny
 */
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

    if (errno == ERANGE || *err != '\0' || *arg == '\0') {
      errno = 0;
      ErrorTraceback(linum, "DEG BY WRONG VARIABLE");
    } else {
      DegBy(stack, idx, linum);
    }
  } else if (strcmp(cmnd, "at") == 0) {
    x = strtol(arg, &err, 10);

    if (errno == ERANGE || *err != '\0' || *arg == '\0') {
      errno = 0;
      ErrorTraceback(linum, "AT WRONG VALUE");
    } else {
      At(stack, x, linum);
    }
  } else {
    ErrorTraceback(linum, "WRONG COMMAND");
  }
}

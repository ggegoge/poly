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
  poly_coeff_t c;

  c = strtol(src, err, 10);

  if (errno == ERANGE) {
    errno = 0;
    return false;
  }

  if (**err != '\0' && **err != ',' && **err != '\n')
    return false;

  if (**err == '\n') {
    ++*err;
  }

  *p = PolyFromCoeff(c);
  return true;
}

static bool ParseMono(char* src, char** err, Mono* m);

/**
 * Próba sparsowania __wielomianu__ ze stringa @p src. Ustawia @p err na pierwszy
 * niewczytany znak (coś jak funkcja biblioteczna `strtod`) i pod @p p ładuje
 * wczytany wielomian.
 * @param[in] src : źródło do wczytania wielomianu zeń
 * @param[out] err : wskazuje pierwszy niewczytany znak
 * @param[out] p : miejsce na wynikowy wielomian
 * @return czy udało się wczytać wielomian
 */
static bool ParsePoly(char* src, char** err, Poly* p)
{
  /* kontrolowanie czy plusy pojawiły się tam gdzie trzeba */
  bool plus_awaited = false;
  Mono m;
  *err = src;
  *p = PolyZero();

  if (isdigit(*src) || *src == '-')
    return ParsePolyCoeff(src, err, p);

  if (*src != '(')
    return false;

  while (*src != ',' && *src != '\0') {
    if (*src == '+' && !plus_awaited) {
      PolyDestroy(p);
      return false;
    }

    if (*src == '+' || *src == '\n') {
      plus_awaited = *src != '+' ? plus_awaited : false;
      ++src;
      continue;
    }

    if (*src != '(' || plus_awaited || !ParseMono(src, err, &m)) {
      PolyDestroy(p);
      return false;
    }

    if (!PolyIsZero(&m.p))
      MonoListInsert(&p->list, &m);

    assert(**err == ')');
    src = ++*err;
    plus_awaited = true;
  }

  if (!plus_awaited) {
    PolyDestroy(p);
    return false;
  }

  if (PolyIsPseudoCoeff(p->list))
    Decoeffise(p);

  *err = src;
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
  Poly p = PolyZero();
  long e;

  *err = src;
  assert(*src == '(');

  if (!ParsePoly(++src, err, &p))
    return false;
  else if (**err != ',') {
    /* jeśli udało się wczytać, ale brakuje przecinka, tzn, że musimy sami
     * usunąć ten wielomian ponieważ ParsePoly tego nie zrobiło */
    PolyDestroy(&p);
    return false;
  }

  src = ++*err;

  /* po przecinku następuje dodatni wykładnik */
  if (!isdigit(*src)) {
    PolyDestroy(&p);
    return false;
  }

  e = strtol(src, err, 10);

  if (errno == ERANGE || e > 2147483647 || e < 0) {
    errno = 0;
    PolyDestroy(&p);
    return false;
  }

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

/**
 * Sprawdza, czy dana komenda jest jedną z komend argumentowych. Sprawdza napis
 * @p src pod kątem jakichś spacji. W przypadku ich znalezienia zwraca `true`
 * i ustawia odpowiednio wskaźnik @p cmnd na początek komendy i @p arg na tenże
 * argument w stylu quasi-`strtok`owym -- ustawia znak '`\0`' na pozycji spacji
 * aby następna obróbka komendy i argumentu była prosta (`strcmp`). Do tego
 * dodaje taki null bajt w miejscu `\n`.
 * @param[in,out] src : napis
 * @param[in] len : jego długość
 * @param[out] arg : wskazuje na argument
 * @return czy znaleziono argument?
 */
static bool FindArg(char* src, size_t len, char** arg)
{
  bool res = false;
  *arg = "";

  for (size_t i = 0; i < len; ++i) {
    if (!res && src[i] == ' ') {
      src[i] = '\0';
      /* komenda zaczyna się tuż po spacji */
      *arg = src + i + 1;
      res =  true;
    } else if (src[i] == '\n') {
      src[i] = '\0';
    } else if (!res  && isspace(src[i]) && i != 0) {
      /* inny whitespace niż spacja to wciąż rozdział na argument, ale wiemy, że
       * ze złym argumentem -- ustawiam więc "\t" */
      *arg = "\t";
      src[i] = '\0';
      res = true;
    }
  }

  return res;
}

static void ParseCommand(char* cmnd, char* arg, size_t linum,
                         struct Stack* stack);

/**
 * Mówi czy komenda @p cmnd jest jedną z wymagających argumentu.
 * @param[in] cmnd : komenda
 * @return czy to komenda argumentowa */
static bool IsArgd(char* cmnd)
{
  return strcmp(cmnd, "DEG_BY") == 0 || strcmp(cmnd, "AT") == 0;
}

void ParseLine(char* src, size_t len, size_t linum, struct Stack* stack)
{
  Poly p = PolyZero();
  bool parsed;
  char* err;
  char* cmnd = src;
  char* arg;

  if (!isalpha(*src)) {
    if ((parsed = ParsePoly(src, &err, &p)) && *err == '\0') {
      PushPoly(stack, &p);
    } else {
      /* jeśli udało się wczytać wielomian, ALE brakuje \n, to nie usunął
       * go ParsePoly, a jest do wyrzucenia */
      if (parsed && *err != '\0')
        PolyDestroy(&p);

      ErrorTraceback(linum, "WRONG POLY");
    }

    return;
  }

  /* szukam argumentu i ustawiam odpowiednio arg. jeśli znajdę argument, to
   * sprawdzam czy cmnd jest dwuargumentowa, inaczej istnienie arg to błąd */
  if (FindArg(src, len, &arg) && !IsArgd(cmnd)) {
    ErrorTraceback(linum, "WRONG COMMAND");
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
 * @param[in,out] stack : stos kalkulacyjny
 */
static void ParseCommand(char* cmnd, char* arg, size_t linum,
                         struct Stack* stack)
{
  unsigned long long idx;
  poly_coeff_t x;
  char* err;
  /* czy nie nastąpiło niedopełnienie stosu */
  bool stacked = true;

  if (strcmp(cmnd, "ADD") == 0) {
    stacked = Add(stack);
  } else if (strcmp(cmnd, "MUL") == 0) {
    stacked = Mul(stack);
  } else if (strcmp(cmnd, "CLONE") == 0) {
    stacked = Clone(stack);
  } else if (strcmp(cmnd, "NEG") == 0) {
    stacked = Neg(stack);
  } else if (strcmp(cmnd, "ZERO") == 0) {
    Zero(stack);
  } else if (strcmp(cmnd, "IS_COEFF") == 0) {
    stacked = IsCoeff(stack);
  } else if (strcmp(cmnd, "IS_ZERO") == 0) {
    stacked = IsZero(stack);
  } else if (strcmp(cmnd, "SUB") == 0) {
    stacked = Sub(stack);
  } else if (strcmp(cmnd, "IS_EQ") == 0) {
    stacked = IsEq(stack);
  } else if (strcmp(cmnd, "DEG") == 0) {
    stacked = Deg(stack);
  } else if (strcmp(cmnd, "PRINT") == 0) {
    stacked = Print(stack);
  } else if (strcmp(cmnd, "POP") == 0) {
    stacked = Pop(stack);
  } else if (strcmp(cmnd, "DEG_BY") == 0) {
    idx = strtoull(arg, &err, 10);

    if (!isdigit(*arg) || errno == ERANGE || *err != '\0' || *arg == '\0') {
      errno = 0;
      ErrorTraceback(linum, "DEG BY WRONG VARIABLE");
    } else {
      stacked = DegBy(stack, idx);
    }
  } else if (strcmp(cmnd, "AT") == 0) {
    x = strtol(arg, &err, 10);

    if (!(isdigit(*arg) || *arg == '-') || errno == ERANGE || *err != '\0' ||
        *arg == '\0') {
      errno = 0;
      ErrorTraceback(linum, "AT WRONG VALUE");
    } else {
      stacked = At(stack, x);
    }
  } else {
    ErrorTraceback(linum, "WRONG COMMAND");
  }

  if (!stacked)
    ErrorTraceback(linum, "STACK UNDERFLOW");
}

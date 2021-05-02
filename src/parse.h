/** @file
  Konieczne funkcje do wczytywania wielomianów i operacji stosowych na nich
  z tekstu.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/


#ifndef _PARSE_H_
#define _PARSE_H_

#include "stack_op.h"
#include "poly.h"

/**
 * Próba sparsowania __wielomianu__ ze stringa @p src. Ustawia @p err na pierwszy
 * niewczytany znak (coś jak funkcja biblioteczna `strtod`) i pod @p p ładuje
 * wczytany wielomian.
 * @param[in] src : źródło do wczytania wielomianu zeń
 * @param[in] err : wskazuje pierwszy niewczytany znak
 * @param[out] p : miejsce na wynikowy wielomian
 * @return czy udało się wczytać wielomian
 */
bool ParsePoly(char* src, char** err, Poly* p);

/**
 * Odpowiednie wczytanie komendy z pojedynczej linii @p src i wywołanie
 * stosownych operacji na stosie kalkulacyjnym.
 * @param[in] src : tekst do parsunku
 * @param[in] len : długość rzeczonej linii
 * @param[in] linum : numer obecnego wiersza
 * @param[in] stack : stos kalkulacyjny
 */
void ParseLn(char* src, size_t len, size_t linum, struct Stack* stack);

#endif

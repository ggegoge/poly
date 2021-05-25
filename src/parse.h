/** @file
  Konieczne funkcje do wczytywania wielomianów i operacji stosowych z tekstu.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/


#ifndef _PARSE_H_
#define _PARSE_H_

#include "stack_op.h"

/**
 * Odpowiednie wczytanie komendy z @p linum-tej linii @p src i wywołanie
 * stosownych operacji na stosie kalkulacyjnym @p stack.
 * @param[in] src : tekst do parsunku
 * @param[in] len : długość rzeczonej linii
 * @param[in] linum : numer obecnego wiersza
 * @param[in,out] stack : stos kalkulacyjny
 */
void ParseLine(char* src, size_t len, size_t linum, struct Stack* stack);

#endif  /* _PARSE_H_ */

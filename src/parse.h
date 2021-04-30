#ifndef _PARSE_H_
#define _PARSE_H_

#include "stack_op.h"
#include "poly.h"

/* err jak w strtodzie */
/**
 * Próba sparsowania __wielomianu__ ze stringa @p src. Ustawia @p err na pierwszy
 * niewczytany znak (coś jak funkcja biblioteczna `strtod`) i pod @p p ładuje
 * wczytany wielomian.
 * @param[in] src : źródło do wczytania wielomianu zeń
 * @param[in,out] err : err, gdzie skończył wczytywanie
 * @param[out] p : miejsce na wynikowy wielomian
 * @return czy udało się wczytać wielomian
 */
bool ParsePoly(char* src, char** err, Poly* p);

/* idk. myślałem by to zrobić jakoś przy użyciu PolyAddMonos tj, że dostaje
 * tablicę i na nią ładuje kolejne i następnie je dodaje, ale chyba prościej
 * robić kolejne MonoListInserty */
void ParsePolyMonos(char* src, char* err, void* thunk, size_t size);

void ParseLine(char* src, size_t linum, struct Stack* stack);

#endif

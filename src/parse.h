#ifndef _PARSE_H_
#define _PARSE_H_


#include "poly.h"

/* struct Command {
 *   enum {ADD, AT, DEG_BY} command;
 *   union {
 *     poly_coeff_t at_arg;
 *     unsigned long deg_arg; 
 *   };
 * };
 * 
 * struct input {
 *   enum {COMMAND, ERROR} kind;
 *   union {
 *     Poly poly;
 *     struct Command cmnd;    
 *   };
 *   size_t linum;  
 * }; */

  

/*
 * <poly> ::= <mono> { + <poly> } | <coeff>
 * <mono> ::= ( <poly>, <exp> )
 * 
 * <coeff> ::= -9223372036854775808..9223372036854775807
 * <exp> ::= 0..2147483647
 */


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

/**
 * Analogicznie do ParsePoly -- wczytuje z napisu @p src __jednomian__, który
 * zapisuje w @p m. Pod @p err niewczytana część stringa.
 * @param[in] src : źródło do wczytania jednomianu zeń
 * @param[in,out] err : err, gdzie skończył wczytywanie
 * @param[out] p : miejsce na wynikowy jednomian
 * @return czy udało się wczytać jednomian
 */
bool ParseMono(char* src, char** err, Mono* m);

/* idk. myślałem by to zrobić jakoś przy użyciu PolyAddMonos tj, że dostaje
 * tablicę i na nią ładuje kolejne i następnie je dodaje, ale chyba prościej
 * robić kolejne MonoListInserty */
void ParsePolyMonos(char* src, char* err, void* thunk, size_t size);

#endif

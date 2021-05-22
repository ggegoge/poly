/** @file
  Operacje na stosie i złożonych na nim wielomianach.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/

#ifndef _STACK_OP_H_
#define _STACK_OP_H_

#include <stdbool.h>

#include "poly.h"

/**
 * Stos z wielomianami. */
struct Stack {
  Poly* polys;                  /**< tablica wielomianów złożonych na stosie */
  size_t height;                /**< obecna wysokość stosu */
  size_t size;                  /**< fizyczna wielkość tablicy w pamięci */
};

/**
 * Utworzenie nowego pustego stosu.
 * @return pusty stos
 */
struct Stack EmptyStack(void);

/**
 * Usunięcie stosu @p stack z pamięci.
 * @param[in,out] stack : stos do usunięcia
 */
void StackDestroy(struct Stack* stack);

/**
 * Odłożenie wielomianu na czubek stosu.
 * @param[in,out] stack : stos
 * @param[in] p : wielomian
 */
void PushPoly(struct Stack* stack, Poly* p);

/**
 * Zsumowanie dwóch wielomianów ze szczytu stosu @p stack i odłożenie na ich
 * miejsce tejże sumy.
 * @param[in,out] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Add(struct Stack* stack);

/**
 * Odjęcie od siebie dwóch wielomianów ze szczytu stosu @p stack i odłożenie na
 * ich miejsce tejże różnicy. Odejmuje od wielomianu na wierzchołku ten pod
 * wierzchołkiem.
 * @param[in,out] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Sub(struct Stack* stack);

/**
 * Pomnożenie dwóch wielomianów ze szczytu stosu @p stack i odłożenie na ich
 * miejsce tegoż iloczynu.
 * @param[in,out] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Mul(struct Stack* stack);

/**
 * Skopiowanie wielomianu z czubka stosu i wrzucenie go na ten właśnie czubek.
 * @param[in,out] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Clone(struct Stack* stack);

/**
 * Zanegowanie wielomianu wierzchniego.
 * @param[in,out] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Neg(struct Stack* stack);

/**
 * Wstawienie wielomianu zerowego na czubek stosu.
 * @param[in,out] stack : stos
 */
static inline void Zero(struct Stack* stack)
{
  Poly p = PolyZero();
  PushPoly(stack, &p);
}

/**
 * Sprawdzian czy wielomian z czubka stosu to wielomian stały. W razie
 * zaistnienia takiej sytuacji na wyjście wypisywana jest jedynka; wpp. zero.
 * @param[in] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool IsCoeff(const struct Stack* stack);

/**
 * Sprawdzian czy wielomian z czubka stosu to wielomian zerowy. W razie
 * zaistnienia takiej sytuacji na wyjście wypisywana jest jedynka; wpp. zero.
 * @param[in] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool IsZero(const struct Stack* stack);

/**
 * Sprawdzian czy dwa wielomiany z czubka stosu są równe. W razie
 * zaistnienia takiej sytuacji na wyjście wypisywana jest jedynka; wpp. zero.
 * @param[in] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool IsEq(const struct Stack* stack);

/**
 * Podanie stopnia wielomianu z czubka stosu (wypisuje się go na standardowe
 * wyjście).
 * @param[in] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Deg(const struct Stack* stack);

/**
 * Wypisanie wielomianu z czubka stosy
 * @param[in] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Print(const struct Stack* stack);

/**
 * Usunięcie wielomianu z czubka stosu.
 * @param[in,out] stack : stos kalkulacyjny
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool Pop(struct Stack* stack);

/* Komendy dwuargumentowe: */

/**
 * Wypisanie stopnia wielomianu z czubka @p stack względem zmiennej spod indeksu
 * @p idx.
 * @param[in] stack : stos kalkulacyjny
 * @param[in] idx : indeks zmiennej
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool DegBy(const struct Stack* stack, unsigned long long idx);

/**
 * Podstawienie do wielomianu z czubka @p stack pod główną zmienną wartości
 * @p x i podmianka tego pierwotnego wielomianu na ten po podstawieniu.
 * @param[in,out] stack : stos kalkulacyjny
 * @param[in] x : wartość do podstawienia pod zmienną główną
 * @return czy nie nastąpiło niedopełnienie stosu @p stack
 */
bool At(struct Stack* stack, poly_coeff_t x);

#endif

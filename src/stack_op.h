/** @file
  Operacje na stosie i złożonych na nim wielomianach.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/

#ifndef _STACK_OP_H_
#define _STACK_OP_H_

#include "poly.h"

/**
 * Stos z wielomianami. */
struct Stack {
  Poly* polys;                  /**< tablica wielomianów złożonych na stosie */
  size_t height;                /**< obecna wysokość stosu */
  size_t size;                  /**< fizyczna wielkość tablicy w pamięci */
};

struct Stack EmptyStack(void);

/**
 * Usunięcie stosu @p stack z pamięci.
 * @param[in] stack : stos do usunięcia
 */
void StackDestroy(struct Stack* stack);

/**
 * Odłożenie wielomianu na czubek stosu.
 * @param[in] stack : stos
 * @param[in] p : wielomian
 */
void PushPoly(struct Stack* stack, Poly* p);

/**
 * Zsumowanie dwóch wielomianów ze szczytu stosu @p stack i odłożenie na ich
 * miejsce tejże sumy.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Add(struct Stack* stack, size_t linum);

/**
 * Odjęcie od siebie dwóch wielomianów ze szczytu stosu @p stack i odłożenie na
 * ich miejsce tejże różnicy.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Sub(struct Stack* stack, size_t linum);

/**
 * Pomnożenie dwóch wielomianów ze szczytu stosu @p stack i odłożenie na ich
 * miejsce tegoż iloczynu.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Mul(struct Stack* stack, size_t linum);

/**
 * Skopiowanie wielomianu z czubka stosu i wrzucenie go na ten właśnie czubek.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Clone(struct Stack* stack, size_t linum);

/**
 * Zanegowanie wielomianu wierzchniego.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Neg(struct Stack* stack, size_t linum);

/**
 * Wstawienie wielomianu zerowego na czubek stosu.
 * @param[in] stack : stos
 */
static inline void Zero(struct Stack* stack)
{
  Poly p = PolyZero();
  PushPoly(stack, &p);
}

/**
 * Sprawdzian czy wielomian z czubka stosu to wielomian stały. W razie
 * zaistnienia takiej sytuacji na wyjście wypisywana jest jedynka; wpp. zero.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void IsCoeff(struct Stack* stack, size_t linum);

/**
 * Sprawdzian czy wielomian z czubka stosu to wielomian zerowy. W razie
 * zaistnienia takiej sytuacji na wyjście wypisywana jest jedynka; wpp. zero.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void IsZero(struct Stack* stack, size_t linum);

/**
 * Sprawdzian czy dwa wielomiany z czubka stosu są równe. W razie
 * zaistnienia takiej sytuacji na wyjście wypisywana jest jedynka; wpp. zero.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void IsEq(struct Stack* stack, size_t linum);

/**
 * Podanie stopnia wielomianu z czubka stosu (wypisuje się go na standardowe
 * wyjście).
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Deg(struct Stack* stack, size_t linum);

/**
 * Wypisanie wielomianu z czubka stosy
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Print(struct Stack* stack, size_t linum);

/**
 * Usunięcie wielomianu z czubka stosu.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void Pop(struct Stack* stack, size_t linum);

/* Komendy dwuargumentowe: */

/**
 * Wypisanie stopnia wielomianu z czubka @p stack względem zmiennej spod indeksu
 * @p idx.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] idx : indeks zmiennej
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void DegBy(struct Stack* stack, unsigned long long idx, size_t linum);

/**
 * Podstawienie do wielomianu z czubka @p stack pod główną zmienną wartości
 * @p x i podmianka tego pierwotnego wielomianu na ten po podstawieniu.
 * @param[in] stack: stos kalkulacyjny
 * @param[in] x : wartość do podstawienia pod zmienną główną
 * @param[in] linum : numer wiersza, z którego została wywołana ta komenda
 */
void At(struct Stack* stack, poly_coeff_t x, size_t linum);

#endif

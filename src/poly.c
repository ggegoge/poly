/** @file
  Implmenetacja interfejsu z pliku poly.h dot `klasy' reprezentującej wielomiany
  rzadkie wielu zmiennych.
*/

#include <stdlib.h>
#include <assert.h>

#include "poly.h"



/**
 * Usunięcie z pamięci listy jednomianów.
 * @param[in] head : głowa listy do usunięcia.
 */
void MonoListDestroy(MonoList* head)
{
  /* robię schemat typu `fold_right' -- idę najpierw rekurencyjnie na sam koniec
   * listy, a następnie cofając się najpierw usuwam trzymany w liście jednomian
   * i później zwalniam tę komórkę listy. */
  if (!head)
    return;

  MonoListDestroy(head->tail);
  MonoDestroy(&head->m);
  free(head);
}

void PolyDestroy(Poly* p)
{
  if (!PolyIsCoeff(p))
    MonoListDestroy(p->list);

  free(p);
}

/**
 * Utworzenie pełnej kopii listy jednomianów.
 * @param[in] head : głowa listy jednomianów
 * @return pełna kopia listy */
MonoList* MonoListClone(MonoList* head)
{
  MonoList* elem;

  if (!head)
    return NULL;

  elem = malloc(sizeof(MonoList));
  elem->m = head->m;
  elem->tail = MonoListClone(head->tail);

  return elem;
}

Poly PolyClone(const Poly* p)
{
  MonoList* list;

  if (PolyIsCoeff(p))
    return (Poly) {
    .coeff = p->coeff, .list = NULL
  };

  list = MonoListClone(p->list);

  return (Poly) {
    .size = p->size, .list = list
  };
}

/** Dodanie do siebie dwóch jednomianów pod założeniem, że stopnie @p m i @p t
 * są sobie równe.
 * @param[in] m : jednomian @f$ p x_i^n @f$
 * @param[in] t : jednomian @f$ q x_i^n @f$
 * @return jednomian @f$ (p + q) x_i^n @f$ */
Mono MonosAdd(const Mono* m, const Mono* t)
{
  /* assert(("Only monomials of equal exponent are addable", m->exp == t->exp)); */
  assert(m->exp == t->exp);
  return (Mono) {
    .exp = m->exp, .p = PolyAdd(&m->p, &t->p)
  };
}

/**
 * Porównanie jednomianów dwu jednomianów.
 * @param[in] m : wskaźnik na pierwszy z jednomianów
 * @param[in] t : wskaźnik na drugi z jednomianów
 * @return -1 jeżeli wykładnik @p m jest mniejszy od wykładnika @p t, 0 gdy są
 * równe i 1 w odwrotnej sytuacji. */
int MonoCmp(const void* m, const void* t)
{
  Mono* mm = (Mono*) m;
  Mono* tt = (Mono*) t;
  return (mm->exp > tt->exp) < (mm->exp < tt->exp);
}

/**
 * Złączenie dwu list jednomianów w jedną nową, która odpowiada zsumowaniu
 * tychże.
 * @param[in] h1 : pierwsza z list jednomianów
 * @param[in] h2 : druga z list jednomianów
 * @return lista jednomianów zawierająca zsumowane wszystkie jednomiany
 * z oryginalnych. */
MonoList* MonoListsMerge(MonoList* h1, MonoList* h2)
{
  int cmp;
  Mono sum;
  MonoList* elem;

  /* złączenie list à la merge sort dopóki obydwie nie są puste */
  if (h1 == NULL && h2 == NULL)
    return NULL;

  if (h1 == NULL)
    cmp = 1;
  else if (h2 == NULL)
    cmp = -1;
  else
    cmp = MonoCmp(&h1->m, &h2->m);

  switch (cmp) {
    
  case 0 :
    sum = MonosAdd(&h1->m, &h2->m);

    if (!PolyIsZero(&sum.p)) {
      elem = malloc(sizeof(MonoList));
      elem->m = sum;
      elem->tail = MonoListsMerge(h1->tail, h2->tail);
      return elem;
    } else
      return MonoListsMerge(h1->tail, h2->tail);

  case -1 :
    elem = malloc(sizeof(MonoList));
    elem->m = h1->m;
    elem->tail = MonoListsMerge(h1->tail, h2);
    return elem;

  case 1 :
    elem = malloc(sizeof(MonoList));
    elem->m = h2->m;
    elem->tail = MonoListsMerge(h2->tail, h1);
    return elem;
    
  default :
    return NULL;
    
  }
}

Poly PolyAdd(const Poly* p, const Poly* q)
{
  MonoList* list;
  if (PolyIsCoeff(p))
    return PolyClone(q);

  if (PolyIsCoeff(q))
    return PolyClone(p);

  list = MonoListsMerge(p->list, q->list);
  if (!list)
    return PolyZero();

  return (Poly) { .size = 2137, .list = list };
}

/* Mono MonosCombine(const Mono* m, const Mono* t)
 * {
 *   /\* assert(("Only monomials of equal exponent are addable", m->exp == t->exp)); *\/
 *  
 * } */

/**
 * Wstawienie komórki listowej z jednomianem w odpowiednie miejsce listy.
 * @param[in] head : głowa listy
 * @param[in] new : komórka listy z nowym elementem */
void MonoListInsert(MonoList** head, MonoList* new)
{
  MonoList** tracer = head;
  int cmp = !0;

  while ((*tracer && (cmp = MonoCmp(&(*tracer)->m, &new->m)) < 1))
    tracer = &(*tracer)->tail;
  
  if (cmp != 0) {
    /* nowy element o wykładniku niepojawionym jeszcze */
    new->tail = *tracer;
    *tracer = new;
  } else {
    /* już jest element z takim wykładnikiem. muszę jakoś zsumować je, ale...
     * czy nie chciałbym pamięci marnować. najlepiej byłoby mieć wersję Add
     * która by zmieniała jeden z tych dwóch, a nie tworzyła nowy, ale.. ech */
    new->m = MonosAdd(&new->m, &(*tracer)->m);
    MonoDestroy(&(*tracer)->m);
    (*tracer)->m = new->m;
  }
    
}

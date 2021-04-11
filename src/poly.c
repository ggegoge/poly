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
static void MonoListDestroy(MonoList* head)
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
static MonoList* MonoListClone(MonoList* head)
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
static Mono MonosAdd(const Mono* m, const Mono* t)
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
static int MonoCmp(const void* m, const void* t)
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
static MonoList* MonoListsMerge(MonoList* h1, MonoList* h2)
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

  elem = malloc(sizeof(MonoList));

  switch (cmp) {

  case 0 :
    sum = MonosAdd(&h1->m, &h2->m);

    if (!PolyIsZero(&sum.p)) {
      elem->m = sum;
      elem->tail = MonoListsMerge(h1->tail, h2->tail);
      return elem;
    } else
      return MonoListsMerge(h1->tail, h2->tail);

  case -1 :
    elem->m = h1->m;
    elem->tail = MonoListsMerge(h1->tail, h2);
    return elem;

  case 1 :
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

  return (Poly) {
    .size = 2137, .list = list
  };
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

/* TODO */
Mono MonoMul(const Mono* m, const Mono* t);

/* TODO */
Poly PolyMulCoeff(const Poly* p, const Poly* coeff);

Poly PolyMul(const Poly* p, const Poly* q)
{
  Poly pq;
  Mono pm, qm, pqm;
  MonoList new;

  if (PolyIsCoeff(p))
    return PolyMulCoeff(q, p);

  if (PolyIsCoeff(q))
    return PolyMulCoeff(p, q);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail) {
    for (MonoList* ql = q->list; ql != NULL; ql = ql->tail) {
      pm = pl->m;
      qm = ql->m;
      pqm = MonoMul(&pm, &qm);
      new = (MonoList) {
        .tail = NULL, .m = pqm
      };
      MonoListInsert(&pq.list, &new);
    }
  }

  return pq;
}

/**
 * Uprzeciwnienie wielomianu samego w sobie. Nie zwraca nowego wielomianu tylko
 * neguje ten otrzymany. Dokładniej rzecz biorąc neguje jego współczynniki
 * liczbowe.
 * @param[in] p : wielomian @f$ p @f$
*/
void PolyNegComp(Poly* p)
{
  if (PolyIsCoeff(p)) {
    p->coeff *= -1;
    return;
  }

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail)
    PolyNegComp(&pl->m.p);

}

Poly PolyNeg(const Poly* p)
{
  Poly np = PolyClone(p);
  PolyNegComp(&np);

  return np;
}

static poly_exp_t MonoListDeg(const MonoList* head)
{
  assert(head);
  return head->m.exp;
}

static inline poly_exp_t max(poly_exp_t a, poly_exp_t b)
{
  return (a < b) ? b : a;
}

static poly_exp_t PolyCoeffDeg(const Poly* p)
{
  assert(PolyIsCoeff(p));
  return PolyIsZero(p) ? -1 : 0;
}

/* co zrobić, gdy nie starczy stopni?? */
poly_exp_t PolyDegBy(const Poly* p, size_t var_idx)
{
  poly_exp_t max_deg = -1;

  if (PolyIsCoeff(p))
    PolyCoeffDeg(p);

  if (var_idx == 0)
    return MonoListDeg(p->list);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail)
    max_deg = max(max_deg, PolyDegBy(&pl->m.p, var_idx - 1));

  return max_deg;
}

poly_exp_t PolyDeg(const Poly* p)
{
  poly_exp_t max_deg = -1;

  if (PolyIsCoeff(p))
    return PolyCoeffDeg(p);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail)
    /* uwaga z tym dodawaniem -- jeśli wielomian poniżej jest = 0 to...
     * aha nie powinien w sumie być równy zero, co nie? bo to by znaczyło, że
     * gdzieś jest głęboko zero ajj */
    max_deg = max(max_deg, pl->m.exp + PolyDeg(&pl->m.p));

  return max_deg;
}

/**
 * Sprawdzian równości dwu jednomianów.
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 * @return czy @p m i @p t są równe */
static bool MonoIsEq(const Mono* m, const Mono* t)
{
  return (m->exp == t->exp) ? PolyIsEq(&m->p, &t->p) : false;
}

bool PolyIsEq(const Poly* p, const Poly* q)
{
  MonoList* pl;
  MonoList* ql;
  bool eq = true;

  if (PolyIsCoeff(p)) {
    if (PolyIsCoeff(q))
      return p->coeff == q->coeff;
    else return false;
  } else if (PolyIsCoeff(q))
    return false;

  pl = p->list;
  ql = q->list;

  while (pl && ql && eq) {
    eq = MonoIsEq(&pl->m, &ql->m);
    pl = pl->tail;
    ql = ql->tail;
  }

  /* nie są równe wtw któreś z jednomianów były różne lub są różnej długości
   * tj. jeden skończył się zanim skończył się drugi */
  return eq && !pl && !ql;
}

/* jak to zrb */
/* Poly PolyAt(const Poly* p, poly_coeff_t x)
 * {
 * } */

/** @file
  Implementacja funkcji z pliku poly_lib.h czyli biblioteki dodatkowych operacji
  na wielomianach rzadkich z poly.h i ichnich listach jednomianów.

  @authors Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date kwiecień 2021
*/

#include <stdlib.h>

#include "poly.h"
#include "poly_lib.h"

/**
 * Sprawdzian powodzenia (m)allokacyjnego.
 */
#define CHECK_PTR(p)                            \
  do {                                          \
    if (!p) {                                   \
      exit(1);                                  \
    }                                           \
  } while (0)

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

MonoList* MonoListClone(const MonoList* head)
{
  MonoList* elem;

  if (!head)
    return NULL;

  elem = malloc(sizeof(MonoList));
  CHECK_PTR(elem);
  elem->m = MonoClone(&head->m);
  elem->tail = MonoListClone(head->tail);

  return elem;
}


/* pseudowykładnik ma formę c x^0 */
bool PolyIsPseudoCoeff(const MonoList* ml)
{
  return ml && ml->m.exp == 0 && PolyIsCoeff(&ml->m.p) && ml->tail == NULL;
}

void Decoeffise(Poly* p)
{
  assert(p->list);              /* pseudowykładnik z definicji ma niepustą listę */
  MonoList* head = p->list;
  poly_coeff_t c = head->m.p.coeff;
  Poly np = {.coeff = c, .list = NULL};
  MonoListDestroy(head);
  *p = np;
}


/**
 * Suma dwóch jednomów równych stopni, również modyfikująca pierwszy z nich
 * (analogiczny mechanizm jak opisany w `PolyAddComp` czyli operator `+=`).
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 */
static void MonoAddComp(Mono* m, const Mono* t)
{
  assert(m->exp == t->exp);
  PolyAddComp(&m->p, &t->p);
}

/**
 * Porównanie dwu jednomianów po ich wykładnikach.
 * @param[in] m : wskaźnik na pierwszy z jednomianów
 * @param[in] t : wskaźnik na dru1gi z jednomianów
 * @return -1 gdy wykładnik @p m jest mniejszy od wykładnika @p t, w przeciwnym
 * przypadku 1, 0 oznacza równość.
 */
static int MonoCmp(const Mono* m, const Mono* t)
{
  return (m->exp > t->exp) - (m->exp < t->exp);
}

/**
 * Złączenie dwu list jednomianów w jedną nową, która odpowiada zsumowaniu
 * tychże. Jest to robione w formie `+=` -- zmienia się @p lhead w oparciu
 * o @p rhead, która pozostaje niezmieniona.
 * @param[in] lhead : głowa lewej listy
 * @param[in] rhead : głowa prawej listy
 * @return głowa listy jednomianów zawierająca zsumowane wszystkie jednomiany
 * z oryginalnych @p lhead i @p rhead.
 */
static MonoList* MonoListsMerge(MonoList* lhead, const MonoList* rhead)
{
  int cmp;
  MonoList* cpy;
  MonoList* tmp;

  /* złączenie list à la merge sort dopóki obydwie nie są puste. */
  if (lhead == NULL && rhead == NULL)
    return NULL;

  /* chcę podłączyć lhead jeśli rhead jest puste i na odwrót. jeśli obydwa
   * niepuste, to podłączam w kolejności malejącej expów */
  if (lhead == NULL)
    cmp = -1;
  else if (rhead == NULL)
    cmp = 1;
  else
    cmp = MonoCmp(&lhead->m, &rhead->m);

  /* celem jest zmodyfikowanie listy lhead i pozostawienie bez szwanku listy rhead,
   * zatem elementy z lhead pozostawiam takie jakimi są, elementy z rhead
   * wkopiowuję, a trafiając na równe potęgi dokonuję lhead->m += rhead->m */
  switch (cmp) {

  case 0 :                      /* lh == rh */
    /* lh->m += rh->m */
    MonoAddComp(&lhead->m, &rhead->m);

    if (!PolyIsZero(&lhead->m.p)) {
      lhead->tail = MonoListsMerge(lhead->tail, rhead->tail);
      return lhead;
    } else {
      /* jeśli dostałem zero, to go nie chcę utrzymywać bez sensu w liście */
      MonoDestroy(&lhead->m);
      tmp = lhead->tail;
      free(lhead);
      return MonoListsMerge(tmp, rhead->tail);
    }

  case 1 :                      /* lh > rh */
    lhead->tail = MonoListsMerge(lhead->tail, rhead);
    return lhead;

  case -1 :                     /* lh < rh */
    cpy = malloc(sizeof(MonoList));
    CHECK_PTR(cpy);
    cpy->m = MonoClone(&rhead->m);
    cpy->tail = MonoListsMerge(lhead, rhead->tail);
    return cpy;

  default :
    return NULL;

  }
}

/**
 * Tworzy pseudowykładnik dla danej liczby. Więcej szczegółów w `Decoeffise`.
 * @param[in] c : wykładnik
 * @return komórka listy będąca pseudowykładnikiem
 */
static MonoList* PolyPseduoCoeff(poly_coeff_t c)
{
  MonoList* head = malloc(sizeof(MonoList));
  CHECK_PTR(head);
  head->m.p = (Poly) {
    .coeff = c, .list = NULL
  };
  head->m.exp = 0;
  head->tail = NULL;
  return head;
}

void PolyAddComp(Poly* p, const Poly* q)
{
  MonoList* l;

  if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
    p->coeff += q->coeff;
    return;
  }

  if (PolyIsCoeff(p) && !PolyIsZero(p)) {
    p->list = PolyPseduoCoeff(p->coeff);
    p->list = MonoListsMerge(p->list, q->list);
  } else if (PolyIsCoeff(q) && !PolyIsZero(q)) {
    l = PolyPseduoCoeff(q->coeff);
    MonoListInsert(&p->list, l);
  } else
    p->list = MonoListsMerge(p->list, q->list);

  /* nawet jeśli lista się znullyfikowała, to dzięki ścisłemu reżimowi
   * inicjalizacji wykładników możemy spać spokojnie -- są one z defaultu zerowe
   * zatem zostanie nam po prostu lista z 0 i NULLem */

  if (PolyIsPseudoCoeff(p->list))
    Decoeffise(p);
}

Poly PolyAddCoeff(const Poly* p, poly_coeff_t coeff)
{
  Poly new = PolyZero();
  MonoList* coeff_wrapper;

  if (coeff == 0)
    new = PolyClone(p);
  else if (PolyIsCoeff(p)) {
    new.coeff = coeff + p->coeff;
    new.list = NULL;
  } else {
    coeff_wrapper = PolyPseduoCoeff(coeff);
    new.list = MonoListClone(p->list);
    MonoListInsert(&new.list, coeff_wrapper);
  }

  return new;
}

/* algorytm insercji o nazwie `triple ref pointer technique' poznałem o tutaj:
 * https://www.youtube.com/watch?v=1s0w_p5HEuY&t=0s
 * i jest to sprytny sposób na iteracyjne wstawienie elementu do listy bez
 * jakiejś nadmiernej ifologii */
void MonoListInsert(MonoList** head, MonoList* new)
{
  MonoList** tracer = head;
  MonoList* tmp;
  int cmp = 1;

  assert(new);
  assert(!PolyIsZero(&new->m.p));

  while ((*tracer && (cmp = MonoCmp(&(*tracer)->m, &new->m)) > 0))
    tracer = &(*tracer)->tail;

  if (cmp != 0) {
    /* nowy element o wykładniku niepojawionym jeszcze */
    new->tail = *tracer;
    *tracer = new;
  } else {
    /* nowy element jest merge'owany z już istniejącym o równym stopniu */
    MonoAddComp(&(*tracer)->m, &new->m);

    if (PolyIsZero(&(*tracer)->m.p)) {
      /* wyzerowanie -- podłączam po prostu ogon pod tracera */
      MonoDestroy(&(*tracer)->m);
      tmp = *tracer;
      *tracer = (*tracer)->tail;
      free(tmp);
    }

    MonoDestroy(&new->m);
    /* new powstaje przez moj malloc */
    free(new);
  }
}

Mono MonoMul(const Mono* m, const Mono* t)
{
  Mono mt;
  mt.exp = m->exp + t->exp;
  mt.p = PolyMul(&m->p, &t->p);

  return mt;
}

static MonoList* MonoListMulCoeff(MonoList* head, poly_coeff_t coeff);

/**
 * Pomnożenie wielomianu @p p przez współczynnik @p coeff ''w miejscu''.
 * Odpowiednik operacji `p *= c`.
 * @param[in] p : wielomian @f$ p(x) @f$
 * @param[in] coeff : współczynnik @f$ c @f$
 */
static void PolyMulCoeffComp(Poly* p, poly_coeff_t coeff)
{
  if (PolyIsCoeff(p))
    p->coeff *= coeff;
  else
    p->list = MonoListMulCoeff(p->list, coeff);
}

/**
 * Pomnożenie listy przez skalar. Każdy element listy zaczynającej się
 * w @p head zostaje pomnożony przez współczynnik @p coeff.
 * @param[in] head : głowa listy
 * @param[in] coeff : skalar
 * @return głowa przemnożonej listy
 */
static MonoList* MonoListMulCoeff(MonoList* head, poly_coeff_t coeff)
{
  MonoList* tail;

  if (!head)
    return NULL;

  PolyMulCoeffComp(&head->m.p, coeff);

  if (PolyIsZero(&head->m.p)) {
    MonoDestroy(&head->m);
    tail = head->tail;
    free(head);
    return MonoListMulCoeff(tail, coeff);
  }

  head->tail = MonoListMulCoeff(head->tail, coeff);
  return head;
}

Poly PolyMulCoeff(const Poly* p, poly_coeff_t coeff)
{
  Poly pc = PolyClone(p);
  PolyMulCoeffComp(&pc, coeff);

  if (PolyIsPseudoCoeff(pc.list))
    Decoeffise(&pc);

  return pc;
}

poly_exp_t MonoListDeg(const MonoList* head)
{
  assert(head != NULL);
  return head->m.exp;
}

poly_exp_t PolyCoeffDeg(const Poly* p)
{
  /* wielomian stały ma stopień 0 o ile nie jest tożsamościowo równy 0, wtedy
   * jego stopień wynosi -1 */
  assert(PolyIsCoeff(p));
  return PolyIsZero(p) ? -1 : 0;
}

bool MonoIsEq(const Mono* m, const Mono* t)
{
  return (m->exp == t->exp) && PolyIsEq(&m->p, &t->p);
}

void PolyNegComp(Poly* p)
{
  PolyMulCoeffComp(p, -1);
}

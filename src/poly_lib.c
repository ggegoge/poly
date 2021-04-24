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

/* pseudowspółczynnik ma formę c x^0 */
bool PolyIsPseudoCoeff(const MonoList* ml)
{
  return ml && ml->m.exp == 0 && PolyIsCoeff(&ml->m.p) && !ml->tail;
}

void Decoeffise(Poly* p)
{
  assert(p->list);              /* pseudowspółczynnik ma niepustą listę */
  MonoList* head = p->list;
  poly_coeff_t c = head->m.p.coeff;
  Poly np = PolyFromCoeff(c);
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
 * przypadku 1, 0 oznacza równość -- konwencja zgodna z C.
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
  if (!lhead && !rhead)
    return NULL;

  /* chcę podłączyć lhead jeśli rhead jest puste i na odwrót. jeśli obydwa
   * niepuste, to podłączam w kolejności malejącej expów */
  if (!lhead)
    cmp = -1;
  else if (!rhead)
    cmp = 1;
  else
    cmp = MonoCmp(&lhead->m, &rhead->m);

  /* celem jest zmodyfikowanie listy lhead i pozostawienie bez szwanku listy
   * rhead. Elementy z lhead pozostawiam takie jakimi są, elementy z rhead
   * wkopiowuję, a trafiając na równe potęgi dokonuję lhead->m += rhead->m */

  if (cmp == 0) {                /* lh == rh */
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
  } else if (cmp > 0) {        /* lh > rh */
    lhead->tail = MonoListsMerge(lhead->tail, rhead);
    return lhead;
  } else {                      /* lh < rh */
    cpy = malloc(sizeof(MonoList));
    CHECK_PTR(cpy);
    cpy->m = MonoClone(&rhead->m);
    cpy->tail = MonoListsMerge(lhead, rhead->tail);
    return cpy;
  }
}

/**
 * Tworzy jednomian będący _pseudowspółczynnikiem_ reprezentującym @p c.
 * @param[in] c : współczynnik @f$ c @f$
 * @return @f$ c x_0^0 @f$
 */
static inline Mono MonoPseudoCoeff(poly_coeff_t c)
{
  return (Mono) {
    .exp = 0, .p = PolyFromCoeff(c)
  };
}

/**
 * Tworzy pseudowspółczynnik dla danej liczby opakowany w listę.
 * Więcej szczegółów w `Decoeffise`.
 * @param[in] c : współczynnik
 * @return komórka listy będąca pseudowspółczynnikiem
 */
static MonoList* MonoListPseduoCoeff(poly_coeff_t c)
{
  MonoList* head = malloc(sizeof(MonoList));
  CHECK_PTR(head);
  head->m.p = PolyFromCoeff(c);
  head->m.exp = 0;
  head->tail = NULL;
  return head;
}

void PolyAddComp(Poly* p, const Poly* q)
{
  Mono m;

  if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
    p->coeff += q->coeff;
    return;
  }

  if (PolyIsCoeff(p) && !PolyIsZero(p)) {
    /* zamieniam wielomian wykładnikowy na pseudowspółczynnik by był
     * kompatybilny ze standardowym wielomianem q tj. by dało się użyć
     * MonoListsMerge */
    p->list = MonoListPseduoCoeff(p->coeff);
    /* kluczowe jest ustawienie coeff na 0 gdy jego lista jest != NULL ponieważ
     * to daje łatwą obsługę redukcji. Zamiast po dodawaniu patrzeć czy lista
     * opustoszała i konstatować -- aha, to 0 -- umieszczamy tam 0 zawczasu */
    p->coeff = 0;
    p->list = MonoListsMerge(p->list, q->list);
  } else if (PolyIsCoeff(q) && !PolyIsZero(q)) {
    m = MonoPseudoCoeff(q->coeff);
    MonoListInsert(&p->list, &m);
  } else {
    p->list = MonoListsMerge(p->list, q->list);
  }

  /* nawet jeśli lista się znullyfikowała, to dzięki ścisłemu reżimowi
   * inicjalizacji wykładników możemy spać spokojnie -- są one z defaultu zerowe
   * zatem zostanie nam po prostu lista z 0 i NULLem */

  if (PolyIsPseudoCoeff(p->list))
    Decoeffise(p);
}

Poly PolyAddCoeff(const Poly* p, poly_coeff_t coeff)
{
  Poly new = PolyZero();
  Mono m;

  if (coeff == 0) {
    new = PolyClone(p);
  } else if (PolyIsCoeff(p)) {
    new.coeff = coeff + p->coeff;
    new.list = NULL;
  } else {
    m = MonoPseudoCoeff(coeff);
    new.list = MonoListClone(p->list);
    MonoListInsert(&new.list, &m);
  }

  return new;
}

/* algorytm insercji o nazwie `triple ref pointer technique' poznałem o tutaj:
 * https://www.youtube.com/watch?v=1s0w_p5HEuY&t=0s
 * i jest to sprytny sposób na iteracyjne wstawienie elementu do listy bez
 * jakiejś nadmiernej ifologii */
void MonoListInsert(MonoList** head, Mono* m)
{
  MonoList** tracer = head;
  MonoList* tmp;
  MonoList* new;
  int cmp = 1;

  assert(!PolyIsZero(&m->p));

  while ((*tracer && (cmp = MonoCmp(&(*tracer)->m, m)) > 0))
    tracer = &(*tracer)->tail;

  if (cmp != 0) {
    /* nowy element o wykładniku niepojawionym jeszcze */
    new = malloc(sizeof(MonoList));
    CHECK_PTR(new);
    new->m = *m;
    new->tail = *tracer;
    *tracer = new;
  } else {
    /* nowy element jest merge'owany z już istniejącym o równym stopniu */
    MonoAddComp(&(*tracer)->m, m);

    if (PolyIsZero(&(*tracer)->m.p)) {
      /* wyzerowanie -- podłączam po prostu ogon pod tracera */
      MonoDestroy(&(*tracer)->m);
      tmp = *tracer;
      *tracer = (*tracer)->tail;
      free(tmp);
    }

    MonoDestroy(m);
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

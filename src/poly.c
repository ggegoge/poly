/** @file
  Implementacja interfejsu z pliku poly.h `klasy' reprezentującej wielomiany
  rzadkie wielu zmiennych.

  @authors Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date kwiecień 2021
*/

#include <stdlib.h>
#include <assert.h>

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

void PolyDestroy(Poly* p)
{
  if (!PolyIsCoeff(p))
    MonoListDestroy(p->list);
}

Poly PolyClone(const Poly* p)
{
  Poly np;

  np.coeff = p->coeff;
  np.list = MonoListClone(p->list);

  return np;
}

Poly PolyAdd(const Poly* p, const Poly* q)
{
  Poly new = PolyZero();

  if (PolyIsCoeff(p) && PolyIsCoeff(q))
    return PolyFromCoeff(p->coeff + q->coeff);

  if (PolyIsCoeff(p))
    return PolyAddCoeff(q, p->coeff);

  if (PolyIsCoeff(q))
    return PolyAddCoeff(p, q->coeff);

  /* operację p + q można rozumieć jako p' += q, gdzie p' to kopia p */
  new = PolyClone(p);
  PolyAddComp(&new, q);

  return new;
}

Poly PolyMul(const Poly* p, const Poly* q)
{
  Poly pq = PolyZero();
  /* jednomiany należące do wielomianów p, q i p * q */
  Mono pm, qm, pqm;
  /* MonoList* new; */

  if (PolyIsCoeff(p))
    return PolyMulCoeff(q, p->coeff);

  if (PolyIsCoeff(q))
    return PolyMulCoeff(p, q->coeff);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail) {
    for (MonoList* ql = q->list; ql != NULL; ql = ql->tail) {
      pm = pl->m;
      qm = ql->m;
      pqm = MonoMul(&pm, &qm);

      if (PolyIsZero(&pqm.p))
        MonoDestroy(&pqm);
      else {
        /* new = malloc(sizeof(MonoList));
         * CHECK_PTR(new);
         * new->m = pqm;
         * new->tail = NULL; */
        MonoListInsert(&pq.list, &pqm);
      }
    }
  }

  return pq;
}

Poly PolyNeg(const Poly* p)
{
  return PolyMulCoeff(p, -1);
}

Poly PolySub(const Poly* p, const Poly* q)
{
  /* nq := q; nq *= -1; nq += p <---> nq := (-q) + p */
  Poly nq = PolyClone(q);
  PolyNegComp(&nq);
  PolyAddComp(&nq, p);

  return nq;
}

/**
 * Prosta funkcyjka licząca maksimum dwu wykładników.
 * @param[in] a : @f$ a @f$
 * @param[in] b : @f$ b @f$
 * @return @f$ \max(a, b) @f$
 */
static inline poly_exp_t max(poly_exp_t a, poly_exp_t b)
{
  return (a < b) ? b : a;
}

/* co zrobić, gdy nie starczy stopni?? */
poly_exp_t PolyDegBy(const Poly* p, size_t var_idx)
{
  poly_exp_t max_deg = -1;

  if (PolyIsCoeff(p))
    return PolyCoeffDeg(p);

  /* jesteśmy w wielomianie danej zmiennej */
  if (var_idx == 0)
    return MonoListDeg(p->list);

  /* stopniem względem tej zmiennej będzie największy z tych stopni znalezionych
   * rekurencyjnie */
  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail)
    max_deg = max(max_deg, PolyDegBy(&pl->m.p, var_idx - 1));

  return max_deg;
}

poly_exp_t PolyDeg(const Poly* p)
{
  poly_exp_t max_deg = -1;

  if (PolyIsCoeff(p))
    return PolyCoeffDeg(p);

  /* szukam stopnia rekurencyjnie -- stopień to jest jak gdyby zmiana wszystkich
   * zmiennych na jedną i szukanie największej potęgi, zatem to właśnie robię
   * dodając wykładniki i rekurencyjnie się zagłębiając w czeluści dalekich x */
  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail)
    max_deg = max(max_deg, pl->m.exp + PolyDeg(&pl->m.p));

  return max_deg;
}

bool PolyIsEq(const Poly* p, const Poly* q)
{
  MonoList* pl;
  MonoList* ql;
  bool eq = true;

  /* jeśli wielomiany są współczynnikami, to decyduje równość arytmetyczna.
   * jeśli jeden z nich jest, a drugi już nie, to nierówność jest oczywista */
  if (PolyIsCoeff(p) && PolyIsCoeff(q))
    return p->coeff == q->coeff;
  else if (PolyIsCoeff(p) || PolyIsCoeff(q))
    return false;

  for (pl = p->list, ql = q->list; pl && ql && eq; pl = pl->tail, ql = ql->tail)
    eq = MonoIsEq(&pl->m, &ql->m);

  /* nie są równe wtw któreś z jednomianów były różne lub są różnej długości
   * tj. jeden skończył się zanim skończył się drugi */
  return eq && !pl && !ql;
}

/**
 * Potęgowanie liczb całkowitych. Oparty na algorytmie potęgowania przez
 * podnoszenie do kwadratu, który ma złożoność @f$\log n@f$ w przeciwieństwie do
 * naiwnego rozwiązania liniowego. Nie będę kłamać, rozwiązanie iteracyjne
 * wziąłem stąd: https://en.wikipedia.org/wiki/Exponentiation_by_squaring
 * @param[in] a : @f$ a @f$
 * @param[in] n : @f$ n @f$
 * @return @f$ a^n @f$
 */
static poly_coeff_t QuickPow(poly_coeff_t a, poly_coeff_t n)
{
  poly_coeff_t b;

  assert(n >= 0);

  if (n == 0)
    return 1;

  b = 1;

  while (n > 1) {
    if (n % 2 == 0) {
      a *= a;
      n /= 2;
    } else {
      b *= a;
      a *= a;
      n = (n - 1) / 2;
    }
  }

  return a * b;
}

Poly PolyAt(const Poly* p, poly_coeff_t x)
{
  /* zamieniam wszystkie x_0 na x i potęguję je przez odpowiednie wykładniki.
   * Wynik traktować mogę jako mnożenie wielomianu wokół x_1 przez skalar. Robię
   * więc tak: tworzę wielomian wynikowy res, z każdego jednomianu wybieram jego
   * wielomian p, i dokonuję res += p * x^n -- powstaje mi suma kumulatywna
   * wielomianów wielu, która jest wynikiem */
  Poly res = PolyZero();
  Poly mul = PolyZero();
  poly_coeff_t coeff;

  if (PolyIsCoeff(p))
    return PolyClone(p);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail) {
    coeff = QuickPow(x, pl->m.exp);
    mul = PolyMulCoeff(&pl->m.p, coeff);
    PolyAddComp(&res, &mul);
    PolyDestroy(&mul);
  }

  return res;
}

Poly PolyAddMonos(size_t count, const Mono monos[])
{
  MonoList* head = NULL;
  /* MonoList* elem; */
  Poly sum = PolyZero();
  Mono m;

  for (size_t i = 0; i < count; ++i) {
    if (PolyIsZero(&monos[i].p))
      continue;

    /* elem = malloc(sizeof(MonoList));
     * CHECK_PTR(elem); */

    /* elem->m = monos[i];
     * elem->tail = NULL; */
    m = monos[i];
    MonoListInsert(&head, &m);
  }

  sum.list = head;

  if (PolyIsPseudoCoeff(sum.list))
    Decoeffise(&sum);

  return sum;
}

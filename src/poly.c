/** @file
  Implementacja interfejsu z pliku poly.h `klasy' reprezentującej wielomiany
  rzadkie wielu zmiennych.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date kwiecień 2021
*/

#include <stdlib.h>
#include <assert.h>

#include "poly.h"
#include "poly_lib.h"

void PolyDestroy(Poly* p)
{
  if (!PolyIsCoeff(p))
    MonoListDestroy(p->list);
}

Poly PolyClone(const Poly* p)
{
  return (Poly) {
    .coeff = p->coeff, .list = MonoListClone(p->list)
  };
}

Poly PolyAdd(const Poly* p, const Poly* q)
{
  Poly new;

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

  if (PolyIsCoeff(p))
    return PolyMulCoeff(q, p->coeff);

  if (PolyIsCoeff(q))
    return PolyMulCoeff(p, q->coeff);

  for (MonoList* pl = p->list; pl; pl = pl->tail) {
    for (MonoList* ql = q->list; ql; ql = ql->tail) {
      pm = pl->m;
      qm = ql->m;
      pqm = MonoMul(&pm, &qm);

      if (PolyIsZero(&pqm.p))
        MonoDestroy(&pqm);
      else
        MonoListInsert(&pq.list, &pqm);
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
  /* nq := -q; nq += p <---> nq := (-q) + p */
  Poly nq = PolyNeg(q);
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

poly_exp_t PolyDegBy(const Poly* p, size_t idx)
{
  poly_exp_t deg = -1;

  if (PolyIsCoeff(p))
    return PolyCoeffDeg(p);

  /* jesteśmy w wielomianie danej zmiennej */
  if (idx == 0)
    return MonoListDeg(p->list);

  /* stopniem względem tej zmiennej będzie największy z tych stopni znalezionych
   * rekurencyjnie */
  for (MonoList* pl = p->list; pl; pl = pl->tail)
    deg = max(deg, PolyDegBy(&pl->m.p, idx - 1));

  return deg;
}

poly_exp_t PolyDeg(const Poly* p)
{
  poly_exp_t deg = -1;

  if (PolyIsCoeff(p))
    return PolyCoeffDeg(p);

  /* szukam stopnia rekurencyjnie -- stopień to jest jak gdyby zmiana wszystkich
   * zmiennych na jedną i szukanie największej potęgi, zatem to właśnie robię
   * dodając wykładniki i rekurencyjnie się zagłębiając w czeluści dalekich x */
  for (MonoList* pl = p->list; pl; pl = pl->tail)
    deg = max(deg, pl->m.exp + PolyDeg(&pl->m.p));

  return deg;
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
  poly_coeff_t b = 1;

  assert(n >= 0);

  if (n == 0 || a == 1)
    return 1;

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
  Poly mul;
  poly_coeff_t coeff;

  if (PolyIsCoeff(p))
    return PolyClone(p);

  for (MonoList* pl = p->list; pl; pl = pl->tail) {
    coeff = QuickPow(x, pl->m.exp);
    mul = PolyMulCoeff(&pl->m.p, coeff);
    PolyAddComp(&res, &mul);
    PolyDestroy(&mul);
  }

  return res;
}

Poly PolyCompose(const Poly* p, size_t k, const Poly* q)
{
  Poly tmp;
  Poly res = PolyZero();
  Poly pow;
  Poly mul;

  if (PolyIsCoeff(p))
    return PolyClone(p);

  /* chyba zwracamy 0 w tej gałęzi */
  if (k <= 0) {
    /* for (MonoList* pl = p->list; pl; pl = pl->tail) {
     *   tmp = PolyCompose(&pl->m.p, k - 1, q);
     *   PolyAddComp(&res, &tmp);
     *   PolyDestroy(&tmp);
     * } */
  } else {
    for (MonoList* pl = p->list; pl; pl = pl->tail) {
      tmp = PolyCompose(&pl->m.p, k - 1, q + 1);

      if (PolyIsZero(&tmp))
        continue;

      if (!PolyIsCoeff(q))
        pow = PolyPow(q, pl->m.exp);
      else
        pow = PolyFromCoeff(QuickPow(q->coeff, pl->m.exp));

      mul = PolyMul(&pow, &tmp);
      PolyAddComp(&res, &mul);
      PolyDestroy(&tmp);
      PolyDestroy(&mul);
      PolyDestroy(&pow);
    }
  }

  return res;
}

Poly PolyAddMonos(size_t count, const Mono monos[])
{
  Poly sum = PolyZero();
  Mono m;

  for (size_t i = 0; i < count; ++i) {
    if (PolyIsZero(&monos[i].p))
      continue;

    m = monos[i];
    MonoListInsert(&sum.list, &m);
  }

  if (PolyIsPseudoCoeff(sum.list))
    Decoeffise(&sum);

  return sum;
}

Poly PolyOwnMonos(size_t count, Mono monos[])
{
  Poly p;

  if (!count || !monos)
    return PolyZero();

  p = PolyAddMonos(count, monos);
  free(monos);
  return p;
}

Poly PolyCloneMonos(size_t count, const Mono monos[])
{
  Poly p;
  Mono* cloned;

  if (!count || !monos)
    return PolyZero();

  cloned = malloc(count * sizeof(Mono));

  if (!cloned) exit(1);

  for (size_t i = 0; i < count; ++i)
    cloned[i] = MonoClone(monos + i);

  p = PolyOwnMonos(count, cloned);
  return p;
}

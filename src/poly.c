/** @file
  Implmenetacja interfejsu z pliku poly.h dot `klasy' reprezentującej wielomiany
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
#define CHECK_PTR(p) \
  do {              \
    if (!p) {       \
      exit(1);      \
    }               \
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
    return (Poly) {
    .coeff = p->coeff + q->coeff, .list = NULL
  };

  if (PolyIsCoeff(p))
    return PolyAddCoeff(p->coeff, q);

  if (PolyIsCoeff(q))
    return PolyAddCoeff(q->coeff, p);

  new = PolyClone(p);
  PolyAddComp(&new, q);

  return new;
}

Poly PolyMul(const Poly* p, const Poly* q)
{
  Poly pq = PolyZero();
  Mono pm, qm;
  MonoList* new;

  if (PolyIsCoeff(p))
    return PolyMulCoeff(p->coeff, q);

  if (PolyIsCoeff(q))
    return PolyMulCoeff(q->coeff, p);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail) {
    for (MonoList* ql = q->list; ql != NULL; ql = ql->tail) {
      pm = pl->m;
      qm = ql->m;
      new = malloc(sizeof(MonoList));
      CHECK_PTR(new);
      new->m = MonoMul(&pm, &qm);
      new->tail = NULL;

      if (PolyIsZero(&new->m.p))
        MonoListDestroy(new);
      else
        MonoListInsert(&pq.list, new);
    }
  }

  return pq;
}

Poly PolyNeg(const Poly* p)
{
  return PolyMulCoeff(-1, p);
  /* Poly np = PolyClone(p);
   * PolyNegComp(&np);
   * 
   * return np; */
}

Poly PolySub(const Poly* p, const Poly* q)
{
  /* nq = q; nq *= -1; nq += p <--> nq = (-q) + p */
  Poly nq = PolyClone(q);
  PolyNegComp(&nq);
  PolyAddComp(&nq, p);

  return nq;
}

/**
 * Prosta funkcyjka licząca maksimum dwu współczynników.
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
  /* wszystkie x_0^n zmieniam na x^n, mnożę je przez te koeficje i robię
   * sumę wielomianów wielu. czyli każdemu wielomianowi robię *= koef gdzie koef
   * to x^n i później kumsum (suma akumulatywna) jest robiona tak jakby.
   * wait mam tam listę jednomów... hm. no to tak, z każego biorę ->p i multypl,
   * a następnie je wszystkie robię +=.
   * minus -- konieczność destrukcji mul */
  Poly res = PolyZero();
  Poly mul = PolyZero();
  poly_coeff_t coeff;

  if (PolyIsCoeff(p))
    return PolyClone(p);

  for (MonoList* pl = p->list; pl != NULL; pl = pl->tail) {
    coeff = QuickPow(x, pl->m.exp);
    mul = PolyMulCoeff(coeff, &pl->m.p);
    PolyAddComp(&res, &mul);
    PolyDestroy(&mul);
  }

  return res;
}

Poly PolyAddMonos(size_t count, const Mono monos[])
{
  MonoList* head = NULL;
  MonoList* elem;
  Poly sum = PolyZero();

  for (size_t i = 0; i < count; ++i) {
    if (PolyIsZero(&monos[i].p))
      continue;

    elem = malloc(sizeof(MonoList));
    CHECK_PTR(elem);

    elem->m = monos[i];
    elem->tail = NULL;
    MonoListInsert(&head, elem);
  }

  sum.list = head;

  if (PolyIsPseudoCoeff(sum.list))
    Decoeffise(&sum);

  /* skąd wiedzieć czy to nie koeficja? trzeba uważać jakoś ech */
  return sum;
}

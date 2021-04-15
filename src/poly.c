/** @file
  Implmenetacja interfejsu z pliku poly.h dot `klasy' reprezentującej wielomiany
  rzadkie wielu zmiennych.
*/

#include <stdlib.h>
#include <assert.h>

#include "poly.h"

/**
 * Sprawdzian powodzenia (m)allokacyjnego.
 */
#define CheckPtr(p) \
  do {              \
    if (!p) {       \
      exit(1);      \
    }               \
  } while (0)

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
}

/**
 * Utworzenie pełnej kopii listy jednomianów.
 * @param[in] head : głowa listy jednomianów
 * @return pełna kopia listy */
static MonoList* MonoListClone(const MonoList* head)
{
  MonoList* elem;

  if (!head)
    return NULL;

  elem = malloc(sizeof(MonoList));
  CheckPtr(elem);
  elem->m = MonoClone(&head->m);
  elem->tail = MonoListClone(head->tail);

  return elem;
}

Poly PolyClone(const Poly* p)
{
  Poly np;

  np.coeff = p->coeff;
  np.list = MonoListClone(p->list);

  return np;
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
  return (mm->exp > tt->exp) - (mm->exp < tt->exp);
}

/* TODO */
static void MonoAddComp(Mono* m, const Mono* t);


/**
 * Złączenie dwu list jednomianów w jedną nową, która odpowiada zsumowaniu
 * tychże. Jest to robione w formie `+=` -- zmienia się @p lh w oparciu o @p rh,
 * które pozostaje niezmienne.
 * @param[in] lh : głowa lewej listy
 * @param[in] rh : głowa prawej listy
 * @return lista jednomianów zawierająca zsumowane wszystkie jednomiany
 * z oryginalnych @p lh i @p rh. */
static MonoList* MonoListsMerge(MonoList* lh, const MonoList* rh)
{
  int cmp;
  MonoList* cpy;

  /* złączenie list à la merge sort dopóki obydwie nie są puste. */
  if (lh == NULL && rh == NULL)
    return NULL;

  /* chcę podłączyć lh jeśli rh jest puste i na odwrót. jeśli obydwa niepuste,
   * to podłączam w kolejności malejącej expów */
  if (lh == NULL)
    cmp = -1;
  else if (rh == NULL)
    cmp = 1;
  else
    cmp = MonoCmp(&lh->m, &rh->m);

  /* celem jest zmodyfikowanie listy lh i pozostawienie bez szwanku listy rh,
   * zatem elementy z lh pozostawiam takie jakimi są, elementy z rh wkopiowuję,
   * a trafiając na równe potęgi dokonuję lh ->m += rh->m */
  switch (cmp) {

  case 0 :                      /* lh == rh */
    /* lh->m += rh->m */
    MonoAddComp(&lh->m, &rh->m);

    if (!PolyIsZero(&lh->m.p)) {
      lh->tail = MonoListsMerge(lh->tail, rh->tail);
      return lh;
    } else {
      MonoDestroy(&lh->m);
      return MonoListsMerge(lh->tail, rh->tail);
    }

  case 1 :                      /* lh > rh */
    lh->tail = MonoListsMerge(lh->tail, rh);
    return lh;

  case -1 :                     /* lh < rh */
    cpy = malloc(sizeof(MonoList));
    CheckPtr(cpy);
    cpy->m = MonoClone(&rh->m);
    cpy->tail = MonoListsMerge(lh, rh->tail);
    return cpy;

  default :
    return NULL;

  }
}

/**
 * Wstawienie komórki listowej z jednomianem w odpowiednie miejsce listy.
 * @param[in] head : głowa listy
 * @param[in] new : komórka listy z nowym elementem
 */
void MonoListInsert(MonoList** head, MonoList* new)
{
  MonoList** tracer = head;
  int cmp = 1;

  /* czy to działa???? */
  while ((*tracer && (cmp = MonoCmp(&(*tracer)->m, &new->m)) > 0))
    tracer = &(*tracer)->tail;

  if (cmp != 0) {
    /* nowy element o wykładniku niepojawionym jeszcze */
    new->tail = *tracer;
    *tracer = new;
  } else {
    /* nowy element jest merge'owany z już istniejącym o równym stopniu */
    /* MonoAddComp(&new->m, &(*tracer)->m);
     * /\* jeśli sumacja wyzerowała ten element, to... *\/
     * if (PolyIsZero(&new->m.p)) {
     *   MonoDestroy(&(*tracer)->m);
     *   MonoDestroy(&new->m);
     *
     *   *tracer = (*tracer)->tail;
     * } */
    MonoAddComp(&(*tracer)->m, &new->m);

    if (PolyIsZero(&(*tracer)->m.p)) {
      /* wyzerowanie -- podłączam po prostu ogon pod tracera */
      MonoDestroy(&(*tracer)->m);
      *tracer = (*tracer)->tail;
    }

    MonoDestroy(&new->m);
    /* new powstaje przez moj malloc */
    free(new);
  }
}

static MonoList* PolyPseduoCoeff(poly_coeff_t c)
{
  MonoList* head = malloc(sizeof(MonoList));
  CheckPtr(head);
  head->m.p = (Poly) {
    .coeff = c, .list = NULL
  };
  head->m.exp = 0;
  head->tail = NULL;
  return head;
}

/* TODO -- sprawdzian czy to nie pseudo koeficja czyli coś jak c * x^0 */
static bool PolyIsPseudoCoeff(const Poly* p)
{
  MonoList* head;

  if (p->list) {
    head = p->list;
    return head->m.exp == 0 && PolyIsCoeff(&head->m.p)
           /* && head->tail == NULL */;
  }

  return false;
}
/* TODO -- zamiana pseudo koefu c * x^0 na zwykły --> c */
static void Decoeffise(Poly* p)
{
  assert(p->list);
  MonoList* head = p->list;
  poly_coeff_t c = head->m.p.coeff;
  Poly np = {.coeff = c, .list = NULL};
  MonoListDestroy(head);
  *p = np;
}

/**
 * Suma dwu wielomianów, ale w wersji `compound assignment' tj nie tworzy
 * nowego wielomianu, a jedynie modyfikuje ten ,,po lewej''. Odpowiednik
 * operatora `+=`.
 * @param[in] p : wielomian @f$ p @f$
 * @param[in] q : wielomian @f$ q @f$
 * Wykonuje `p += q`. */
static void PolyAddComp(Poly* p, const Poly* q)
{
  MonoList* l;

  if (PolyIsCoeff(p) && PolyIsCoeff(q))
    p->coeff += q->coeff;
  else if (PolyIsCoeff(p)) {
    l = PolyPseduoCoeff(p->coeff);
    p->list = l;
    p->list = MonoListsMerge(p->list, q->list);
  } else if (PolyIsCoeff(q)) {
    l = PolyPseduoCoeff(q->coeff);
    MonoListInsert(&p->list, l);
  } else
    p->list = MonoListsMerge(p->list, q->list);

  if (PolyIsPseudoCoeff(p))
    Decoeffise(p);
}

/**
 * Suma dwóch jednomów równych stopni, również modyfikująca jeden z nich
 * (analogiczny mechanizm jak opisany w `PolyAddComp`).
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 */
static void MonoAddComp(Mono* m, const Mono* t)
{
  assert(m->exp == t->exp);
  PolyAddComp(&m->p, &t->p);
}

/* /\** Dodanie do siebie dwóch jednomianów pod założeniem, że stopnie @p m i @p t
 *  * są sobie równe.
 *  * @param[in] m : jednomian @f$ p x_i^n @f$
 *  * @param[in] t : jednomian @f$ q x_i^n @f$
 *  * @return jednomian @f$ (p + q) x_i^n @f$ *\/
 * static Mono MonoAdd(const Mono* m, const Mono* t)
 * {
 *   assert(m->exp == t->exp);
 *   return (Mono) {
 *     .exp = m->exp, .p = PolyAdd(&m->p, &t->p)
 *   };
 * } */

/* TODO -- próba rozwiązania dualizmu koeficji */
Poly PolyAddCoeff(poly_coeff_t c, const Poly* p)
{
  Poly new;
  MonoList* coeff_wrapper;

  if (c == 0)
    new = PolyClone(p);
  else if (PolyIsCoeff(p)) {
    new.coeff = c + p->coeff;
    new.list = NULL;
  } else {
    coeff_wrapper = PolyPseduoCoeff(c);
    new.list = MonoListClone(p->list);
    MonoListInsert(&new.list, coeff_wrapper);
  }

  return new;
}

Poly PolyAdd(const Poly* p, const Poly* q)
{
  Poly new;

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

/**
 * Iloczyn jednomianów.
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 * @return iloczyn @f$ m \cdot p @f$
 */
static Mono MonoMul(const Mono* m, const Mono* t)
{
  Mono mt;
  mt.exp = m->exp + t->exp;
  mt.p = PolyMul(&m->p, &t->p);

  return mt;
}

/* TODO */
static void PolyMulCoeffComp(Poly* p, poly_coeff_t coeff)
{
  if (PolyIsCoeff(p))
    p->coeff *= coeff;
  else {
    for (MonoList* pl = p->list; pl != NULL; pl = pl->tail)
      PolyMulCoeffComp(&pl->m.p, coeff);
  }
}

/* TODO */
Poly PolyMulCoeff(poly_coeff_t coeff, const Poly* p)
{
  Poly pc = PolyClone(p);
  PolyMulCoeffComp(&pc, coeff);

  return pc;
}

Poly PolyMul(const Poly* p, const Poly* q)
{
  Poly pq;
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
      CheckPtr(new);
      new->m = MonoMul(&pm, &qm);
      new->tail = NULL;
      MonoListInsert(&pq.list, new);
    }
  }

  return pq;
}

/**
 * Uprzeciwnienie wielomianu samego w sobie. Nie zwraca nowego wielomianu tylko
 * neguje ten otrzymany. Dokładniej rzecz biorąc neguje jego współczynniki
 * liczbowe. Coś a la `p *= -1`.
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

Poly PolySub(const Poly* p, const Poly* q)
{
  /* nq = q; nq *= -1; nq += p <--> nq = (-q) + p */
  Poly nq = PolyClone(q);
  PolyNegComp(&nq);
  PolyAddComp(&nq, p);

  return nq;
}

static poly_exp_t MonoListDeg(const MonoList* head)
{
  assert(head != NULL);
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

/* https://stackoverflow.com/a/18581693 */

/*
Function exp_by_squaring_iterative(x, n)
    if n < 0 then
      x := 1 / x;
      n := -n;
    if n = 0 then return 1
    y := 1;
    while n > 1 do
      if n is even then
        x := x * x;
        n := n / 2;
      else
        y := x * y;
        x := x * x;
        n := (n – 1) / 2;
    return x * y
 */

static int QuickPow(int a, int n)
{
  int b;

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



/* jak to zrb */
Poly PolyAt(const Poly* p, poly_coeff_t x)
{
  /* wszystkie x_0^n zmieniam na x^n, mnożę je przez te koeficje i robię
   * sumę wielomianów wielu. czyli każdemu wielomianowi robię *koef gdzie koef
   * to x^n i później kumsum (suma akumulatywna) jest robiona tak jakby.
   * wait mam tam listę jednomów... hm. no to tak, z każego biorę ->p i multypl,
   * a następnie je wszystkie robię +=.
   * minus -- konieczność destrukcji mul */
  Poly res;
  Poly mul;
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

  for (size_t i = 0; i < count; ++i) {
    elem = malloc(sizeof(MonoList));
    CheckPtr(elem);

    elem->m = monos[i];
    elem->tail = NULL;
    MonoListInsert(&head, elem);
  }

  /* skąd wiedzieć czy to nie koeficja? trzeba uważać jakoś ech */
  return (Poly) {
    .list = head
  };
}



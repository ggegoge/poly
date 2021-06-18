/** @file
  Implementacja funkcji z pliku poly_lib.h czyli biblioteki dodatkowych operacji
  na wielomianach rzadkich z poly.h i ichnich listach jednomianów.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date kwiecień i maj 2021
*/

#include <stdlib.h>

#include "poly.h"
#include "poly_lib.h"

/**
 * Próg stosowania alternatywego mnożenia (@ref PolyMulLong) w potęgowaniu.
 * Wyznaczony quasi-eksperymentalnie.
 */
#define BIG_EXP 3000
/** Współczynnik poszerzania tablicy jednomianów w @ref PolyMulLong */
#define BIG_MUL_ARRAY_RESIZE 2
/** Początkowy rozmiar tablicy jednomianów w @ref PolyMulLong. */
#define BIG_MUL_ARRAY_INIT_SIZE 16

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

Mono* CloneMonoArray(size_t count, const Mono monos[])
{
  Mono* cloned = malloc(count * sizeof(Mono));
  CHECK_PTR(cloned);

  for (size_t i = 0; i < count; ++i)
    cloned[i] = MonoClone(monos + i);

  return cloned;
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
 * @param[in,out] m : jednomian
 * @param[in] t : jednomian
 */
static void MonoAddComp(Mono* m, const Mono* t)
{
  assert(m->exp == t->exp);
  PolyAddComp(&m->p, &t->p);
}

int MonoCmp(const Mono* m, const Mono* t)

{
  return (m->exp > t->exp) - (m->exp < t->exp);
}

/**
 * Funkcja wyznaczająca porządek merge'owania list jednomianów. Podłącza zawsze
 * niepustą przed pustą, a w przypadku niepustości zarazem @p lhead i @p rhead
 * porównuje ich wykładniki za pomocą @ref MonoCmp.
 * @param[in] lhead : głowa lewej listy
 * @param[in] rhead : głowa lewej listy
 * @return -1 lub 0 lub 1 podobnie jak w @ref MonoCmp
 */
static int MonoListsCmp(const MonoList* lhead, const MonoList* rhead)
{
  if (!lhead)
    return -1;
  else if (!rhead)
    return 1;
  else
    return MonoCmp(&lhead->m, &rhead->m);
}

/**
 * Złączenie dwu list jednomianów w jedną nową, która odpowiada zsumowaniu
 * tychże. Jest to robione w formie `+=` -- zmienia się @p lhead w oparciu
 * o @p rhead, która pozostaje niezmieniona.
 * @param[in,out] lhead : głowa lewej listy
 * @param[in] rhead : głowa prawej listy
 * @return głowa listy jednomianów zawierająca zsumowane wszystkie jednomiany
 * z oryginalnych @p lhead i @p rhead.
 */
static MonoList* MonoListsMerge(MonoList* lhead, const MonoList* rhead)
{
  int cmp;
  MonoList* cpy;
  MonoList* tmp;

  /* złączenie list à la merge sort dopóki obydwie nie są puste.
   * Elementy z lhead pozostawiam takie jakimi są, elementy z rhead
   * wkopiowuję, a trafiając na równe potęgi dokonuję lhead->m += rhead->m */
  if (!lhead && !rhead)
    return NULL;

  cmp = MonoListsCmp(lhead, rhead);

  if (cmp == 0) {               /* lh == rh */
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
  } else if (cmp > 0) {         /* lh > rh */
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
     * opustoszała i konstatować -- aha, to 0! -- umieszczamy tam 0 zawczasu */
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
  Poly new;
  Mono m;

  if (coeff == 0) {
    new = PolyClone(p);
  } else if (PolyIsCoeff(p)) {
    new = PolyFromCoeff(coeff + p->coeff);
  } else {
    m = MonoPseudoCoeff(coeff);
    new = PolyClone(p);
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
    MonoDestroy(m);

    if (PolyIsZero(&(*tracer)->m.p)) {
      /* wyzerowanie -- podłączam po prostu ogon pod tracera */
      MonoDestroy(&(*tracer)->m);
      tmp = *tracer;
      *tracer = (*tracer)->tail;
      free(tmp);
    }
  }
}

/**
 * Tworzy tablicę na przechowywanie jednomianów przez @ref PolyMulLong.
 * @param[in] init_size : początkowy rozmiar tablicy
 * @return zaalokowana pamięć na tablicę jednomianów
 */
static Mono* MonosArray(size_t init_size)
{
  Mono* monos = malloc(init_size * sizeof(Mono));
  CHECK_PTR(monos);
  return monos;
}

/**
 * Dodanie jednomianu @p m do tablicy @p monos. Wstawienie go na koniec tejże
 * tablicy i jej powiększenie w razie gdy to koniczne. W takim wypadku nowy
 * rozmiar fizyczny zostaje zapisany w @p len, a @p len trzyma liczbę zajętych
 * miejsc.
 * @param[in,out] len : liczba jednomianów w @p monos
 * @param[in,out] size : wielkość zaalokowanej pamięci pod @p monos
 * @param[in] m : jednomian do dodania
 * @param[in,out] monos : obecna tablica
 * @return tablica @p monos wraz z nowym jednomianem @p m.
 */
static Mono* MonosArrayAppend(size_t* len, size_t* size, Mono* m, Mono* monos)
{
  if (*len >= *size) {
    *size *= BIG_MUL_ARRAY_RESIZE;
    monos = realloc(monos, *size * sizeof(Mono));
    CHECK_PTR(monos);
  }

  monos[(*len)++] = *m;
  return monos;
}

/**
 * Alternatywne mnożenie __dużych__ wielomianów. Działa dokładnie tak samo jak
 * @ref PolyMul, ale stosuje inny mechanizm budowy wieomianu z jednomianów
 * cząstkowych. Dokładniej mówiąc jak @ref PolyMul woła @ref MonoListInsert
 * wiele razy co powoduje asymptotyczny wzrost złożoności do kwadratowej, tak
 * ta funkcja najpierw wrzuca jednomiany do tablicy, a później @ref PolyOwnMonos
 * w efektywny sposób tworzy z nich wielomian. Ta heurystyka spowalnia mnożenie
 * małych (tj. liczących sobie niewiele jednomianów) wielomianów, ale przy
 * długich @p p lub @p q jest bardzo korzystna. Istnieje głównie na potrzeby
 * potęgowania.
 * @param[in] p : @f$p@f$
 * @param[in] q : @f$q@f$
 * @return iloczyn @f$p\cdot q@f$ policzony innym algorytmem.
 */
static Poly PolyMulLong(const Poly* p, const Poly* q)
{
  Poly pq;
  /* jednomiany należące do wielomianów p, q i p * q */
  Mono pm, qm, pqm;
  size_t size = BIG_MUL_ARRAY_INIT_SIZE;
  size_t len = 0;
  Mono* monos;

  if (PolyIsCoeff(p))
    return PolyMulCoeff(q, p->coeff);

  if (PolyIsCoeff(q))
    return PolyMulCoeff(p, q->coeff);

  monos = MonosArray(size);

  for (MonoList* pl = p->list; pl; pl = pl->tail) {
    for (MonoList* ql = q->list; ql; ql = ql->tail) {
      pm = pl->m;
      qm = ql->m;
      pqm = MonoMul(&pm, &qm);

      if (PolyIsZero(&pqm.p))
        MonoDestroy(&pqm);
      else
        monos = MonosArrayAppend(&len, &size, &pqm, monos);
    }
  }

  pq = PolyOwnMonos(len, monos);
  return pq;
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
 * @param[in,out] p : wielomian @f$ p(x) @f$
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
 * w @p head zostaje pomnożony przez współczynnik @p coeff wraz z pozbyciem się
 * wyzerowanych jednomianów w razie, gdy takowe się nadażą.
 * @param[in,out] head : głowa listy
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

void PolyNegComp(Poly* p)
{
  PolyMulCoeffComp(p, -1);
}

poly_exp_t MonoListDeg(const MonoList* head)
{
  assert(head);
  return head->m.exp;
}

poly_exp_t PolyCoeffDeg(const Poly* p)
{
  assert(PolyIsCoeff(p));
  return PolyIsZero(p) ? -1 : 0;
}

bool MonoIsEq(const Mono* m, const Mono* t)
{
  return m->exp == t->exp && PolyIsEq(&m->p, &t->p);
}

/**
 * Sprawdza czy spotęgowanie wielomianu @p p do potęgi @p n to ciężka operacja.
 * Decyduje o użyciu funkcji @ref PolyMulLong przez funkcje potęgujące.
 * @param[in] p : wielomian do spotęgowania
 * @param[in] n : wykładnik
 * @return czy potęgowanie @f$p^n@f$ jest ''ciężką'' operacją.
 */
static inline bool PolyBigPower(const Poly* p, poly_coeff_t n)
{
  return n > BIG_EXP && p->list && p->list->tail;
}

/* ten sam algorytm co w potęgowaniu liczb stosowanym w PolyAt w pliku poly.c */
Poly PolyPow(const Poly* p, poly_coeff_t n)
{
  bool big = PolyBigPower(p, n);
  Poly pow = PolyFromCoeff(1);
  Poly tmppow;
  /* jako, że a to na początku płytka kopia p, to muszę wiedzieć czy się
   * zmieniło zanim wywołam na nim destrukcję */
  bool changed = false;
  Poly a = *p;
  Poly tmpa;

  assert(!PolyIsCoeff(p));
  assert(n >= 0);

  if (n == 0 || PolyIsEq(p, &pow))
    return pow;

  while (n > 1) {
    if (n % 2 == 0) {
      tmpa = big ? PolyMulLong(&a, &a) : PolyMul(&a, &a);
      n /= 2;
    } else {
      tmppow = big ? PolyMulLong(&pow, &a) : PolyMul(&pow, &a);
      PolyDestroy(&pow);
      pow = tmppow;
      tmpa = big ? PolyMulLong(&a, &a) : PolyMul(&a, &a);
      n = (n - 1) / 2;
    }

    if (changed)
      PolyDestroy(&a);
    else
      changed = true;

    a = tmpa;
  }

  tmppow = big ? PolyMulLong(&pow, &a) : PolyMul(&pow, &a);
  PolyDestroy(&pow);
  pow = tmppow;

  if (changed)
    PolyDestroy(&a);

  return pow;
}

Poly* PolyPowTable(const Poly* p, const Poly* q, size_t* count)
{
  size_t n = PolyDegBy(p, 0);
  bool big = PolyBigPower(q, n);
  Poly* powers = NULL;

  for (*count = 0; n > 0; ++*count, n /= 2);

  if (*count != 0) {
    powers = malloc(*count * sizeof(Poly));
    CHECK_PTR(powers);
    powers[0] = PolyClone(q);

    for (size_t i = 1; i < *count; ++i) {
      powers[i] = big ? PolyMulLong(powers + i - 1, powers + i - 1)
                  : PolyMul(powers + i - 1, powers + i - 1);
    }
  }

  return powers;
}

Poly PolyGetPow(const Poly powers[], size_t n)
{
  bool big = PolyBigPower(powers, n);
  Poly res = PolyFromCoeff(1);
  Poly tmp;
  size_t i = 0;

  if (n == 0)
    return PolyFromCoeff(1);

  while (n > 0) {
    if (n % 2 == 1) {
      tmp = big ? PolyMulLong(&res, powers + i) : PolyMul(&res, powers + i);
      PolyDestroy(&res);
      res = tmp;
    }

    ++i;
    n /= 2;
  }

  return res;
}

static void MonoIncorporate(Mono* m, Mono* t);

/**
 * Pełne złączenie list @p lhead i @p rhead.
 * Jak @ref MonoListsMerge to było `+=`, tak to jest ''`+=+`'' -- w odróżnieniu
 * od tamtej funkcji tutaj zachodzi prawdziwe złączenie list bez kopiowań.
 * Brany jest ażdy jednomian z @p lhead i @p rhead à la merge sort. Gdy na trafi
 * na parę o równych wykładnikach włącza zawartość prawej głowy w lewą i zwalnia
 * odpowiednio pamięć. W przypadku gdy dokonanie @p lhead `+=+` @p rhead
 * doprowadzi do wyzerowania się @p lhead to zwalnia pamięć zarazem @p lhead jak
 * i @p rhead, po czym rekurencyjnie wywołuje się na ich ogonach.
 * @param[in,out] lhead : głowa lewej listy
 * @param[in,out] rhead : głowa prawej listy
 * @return głowa listy @p lhead `+=+` @p rhead
 */
static MonoList* MonoListsJoin(MonoList* lhead, MonoList* rhead)
{
  int cmp;
  MonoList* tmp;

  if (!lhead && !rhead)
    return NULL;

  cmp = MonoListsCmp(lhead, rhead);

  if (cmp == 0) {
    MonoIncorporate(&lhead->m, &rhead->m);

    if (!PolyIsZero(&lhead->m.p)) {
      tmp = rhead->tail;
      free(rhead);
      lhead->tail = MonoListsJoin(lhead->tail, tmp);
      return lhead;
    } else {
      MonoDestroy(&lhead->m);
      tmp = lhead->tail;
      free(lhead);
      lhead = tmp;

      tmp = rhead->tail;
      free(rhead);
      rhead = tmp;

      return MonoListsJoin(lhead, rhead);
    }
  } else if (cmp > 0) {
    lhead->tail = MonoListsJoin(lhead->tail, rhead);
    return lhead;
  } else {
    rhead->tail = MonoListsJoin(lhead, rhead->tail);
    return rhead;
  }
}

Poly* PolyIncorporate(Poly* p, Poly* q)
{
  if (PolyIsCoeff(p)) {
    PolyAddComp(q, p);
    *p = *q;
    return p;
  } else if (PolyIsCoeff(q)) {
    PolyAddComp(p, q);
    *q = *p;
    return p;
  }

  p->list = q->list = MonoListsJoin(p->list, q->list);

  if (PolyIsPseudoCoeff(p->list))
    Decoeffise(p);

  return p;
}

/**
 * Funkcja dualna do @ref PolyIncorporate. Łączy jednomiany @p m i @p t. */
static void MonoIncorporate(Mono* m, Mono* t)
{
  assert(m->exp == t->exp);
  PolyIncorporate(&m->p, &t->p);
}

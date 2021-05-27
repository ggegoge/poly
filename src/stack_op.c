/** @file
  Implementacja operacji na stosie z pliku stack_op.h.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "poly.h"
#include "poly_lib.h"
#include "stack_op.h"

/**
 * Początkowa wielkość stosu. */
#define INIT_STACK_SIZE 16

/**
 * Mnożnik względem którego rozszerza się tablica stosowa. */
#define ARR_RESIZE 2

void StackDestroy(struct Stack* stack)
{
  for (size_t i = 0; i < stack->height; ++i)
    PolyDestroy(stack->polys + i);

  free(stack->polys);
}

struct Stack EmptyStack()
{
  struct Stack stack;

  stack.height = 0;
  stack.size = INIT_STACK_SIZE;
  stack.polys = malloc(stack.size * sizeof(Poly));

  if (!stack.polys)
    exit(1);

  return stack;
}

void PushPoly(struct Stack* stack, Poly* p)
{
  if (stack->height >= stack->size) {
    stack->size *= ARR_RESIZE;
    stack->polys = realloc(stack->polys, stack->size * sizeof(Poly));

    if (!stack->polys)
      exit(1);
  }

  stack->polys[stack->height++] = *p;
}

/**
 * Wzięcie wielomianu ze szczytu stosu. Nazewnictwo `car` nawiązuje do tradycji
 * znanej choćby z języków takich jak __lisp__, gdzie kluczowe były funkcje
 * `car` i `cdr` zwracające odpowiednio
 * [głowę i ogon](https://en.wikipedia.org/wiki/CAR_and_CDR)
 * listy. Jak widzą nawet osoby obeznane z tymi językami, jest to nazewnictwo
 * dość [ezoteryczne]
 * (https://www.gnu.org/software/emacs/manual/html_node/eintr/Strange-Names.html)
 * i lekko _przedawnione_. Co więcej: [sami stwierdzają]
 * (https://www.gnu.org/software/emacs/manual/html_node/eintr/car-_0026-cdr.html),
 * że lepszą nazwą byłoby chociażby `first`, jednakże jest to owoc długiej
 * tradycji i w ramach hołdu dla języka __lisp__ zachowam tę konwencję.
 * @param[in] stack : stos
 * @return adres pierwszego wielomianu na stosie
 */
static Poly* Car(const struct Stack* stack)
{
  assert(stack->height >= 1);
  return stack->polys + stack->height - 1;
}

/**
 * Złożenie tradycyjnych operacji `car` i `cdr`
 * (https://en.wikipedia.org/wiki/CAR_and_CDR#Compositions)
 * zwraca więc _głowę ogona listy_ czyli drugi element stosu rzecz jasna.
 * @param[in] stack : stos
 * @return adres drugiego wielomianu ze stosu
 */
static Poly* Cadr(const struct Stack* stack)
{
  assert(stack->height >= 2);
  return stack->polys + stack->height - 2;
}

/**
 * Przycięcie stosu w razie sytuacji gdy za duża jego część leży odłogiem.
 * Robi to tylko wtedy, gdy obecna wysokość @p stack jest odległa od jego
 * fizycznej wielkości ''o dwa powiększenia'' celem uniknięcia _efektu jojo_.
 */
static void Trim(struct Stack* stack)
{
  if (stack->height >= INIT_STACK_SIZE &&
      stack->height * ARR_RESIZE * ARR_RESIZE <= stack->size) {
    stack->size /= ARR_RESIZE;
    stack->polys = realloc(stack->polys, stack->size * sizeof(Poly));

    if (!stack->polys)
      exit(1);
  }
}

bool Pop(struct Stack* stack)
{
  if (stack->height < 1)
    return false;

  PolyDestroy(Car(stack));
  --stack->height;
  Trim(stack);
  return true;
}

bool Add(struct Stack* stack)
{
  if (stack->height < 2)
    return false;

  PolyAddComp(Cadr(stack), Car(stack));
  Pop(stack);

  return true;
}

bool Sub(struct Stack* stack)
{
  if (stack->height < 2)
    return false;

  PolyNegComp(Cadr(stack));
  return Add(stack);
}

bool Mul(struct Stack* stack)
{
  Poly new;

  if (stack->height < 2)
    return false;

  new = PolyMul(Car(stack), Cadr(stack));
  Pop(stack);
  Pop(stack);
  PushPoly(stack, &new);

  return true;
}

bool Clone(struct Stack* stack)
{
  Poly cpy;

  if (stack->height < 1)
    return false;

  cpy = PolyClone(Car(stack));
  PushPoly(stack, &cpy);

  return true;
}

bool Neg(struct Stack* stack)
{
  if (stack->height < 1)
    return false;

  PolyNegComp(Car(stack));

  return true;
}

/**
 * Obsługa komend o charakterze predykatowym. Wypisuje `1` jeśli zachodzi
 * predykat @p p; wpp. wypisuje `0`.
 * @param[in] p : predykat
 */
static void Predicate(bool p)
{
  if (p) printf("1\n");
  else printf("0\n");
}

bool IsCoeff(const struct Stack* stack)
{
  if (stack->height < 1)
    return false;

  Predicate(PolyIsCoeff(Car(stack)));

  return true;
}

bool IsZero(const struct Stack* stack)
{
  if (stack->height < 1)
    return false;

  Predicate(PolyIsZero(Car(stack)));

  return true;
}

bool IsEq(const struct Stack* stack)
{
  if (stack->height < 2)
    return false;

  Predicate(PolyIsEq(Car(stack), Cadr(stack)));

  return true;
}

bool Deg(const struct Stack* stack)
{
  if (stack->height < 1)
    return false;

  printf("%d\n", PolyDeg(Car(stack)));

  return true;
}

bool DegBy(const struct Stack* stack, unsigned long long idx)
{
  if (stack->height < 1)
    return false;

  printf("%d\n", PolyDegBy(Car(stack), idx));
  return true;
}

bool At(struct Stack* stack, poly_coeff_t x)
{
  Poly new;

  if (stack->height < 1)
    return false;

  new = PolyAt(Car(stack), x);
  Pop(stack);
  PushPoly(stack, &new);
  return true;
}

/**
 * Obrócenie @p k wielomianów spod czubka stosu na tablicy stosowej. Funkcja
 * pomocnicza dla @ref Compose. Obraca je w taki sposób, że pierwszy pozostaje
 * na miejscu i dopiero kolejne @p k poniżej są obracane.
 * @param[in,out] stack : stos
 * @param[in] k : ile wielomianów do obrotu
 */
static void RevStackSlice(struct Stack* stack, size_t k)
{
  /* obracam pomiędzy l i r __włącznie__ */
  size_t l, r;
  Poly tmp;
  assert(stack->height >= k + 1);
  
  for (l = stack->height - k - 1, r = stack->height - 2; l < r; ++l, --r) {
    tmp = stack->polys[l];
    stack->polys[l] = stack->polys[r];
    stack->polys[r] = tmp;
  }
}

bool Compose(struct Stack* stack, size_t k)
{
  Poly composee;
  
  if (stack->height < k + 1)
    return false;

  RevStackSlice(stack, k);
  composee = PolyCompose(Car(stack), k, stack->polys + stack->height - k - 1);
  /* usuwam k+1 najwyższych */
  for (size_t i = 0; i <= k; ++i)
    Pop(stack);

  PushPoly(stack, &composee);

  return true;
}

static void PrintPoly(const Poly* p);

/**
 * Wypisanie jednomianu.
 * @param[in] m : jednomian
 */
static void PrintMono(const Mono* m)
{
  printf("(");
  PrintPoly(&(m->p));
  printf(",%d)", m->exp);
}

/**
 * Wypisanie od tyłu (albowiem trzymane są one w kolejności malejącej, a żąda
 * się od nas wypisania ich w wykładnikami rosnąco) listy jednomianów. Ze
 * względu  na tę odtylniość jest to robione schematem rekurencyjnego wybrania
 * się na sam koniec listy, by następnie stopniowo wracać i dopiero wypisywać.
 * Jednomiany są rozdzielone plusami.
 * @param[in] ml : lista jednomianów
 */
static void PrintMonoList(MonoList* ml)
{
  if (!ml)
    return;

  PrintMonoList(ml->tail);

  /* przed pierwszym jednomianem nie ma plusa */
  if (!ml->tail) {
    PrintMono(&ml->m);
  } else {
    printf("+");
    PrintMono(&ml->m);
  }
}

/**
 * Wypisanie w kolejności rosnących wykładników wielomianu @p p.
 * @param[in] p : wielomian
 */
static void PrintPoly(const Poly* p)
{
  if (PolyIsCoeff(p))
    printf("%ld", p->coeff);
  else
    PrintMonoList(p->list);
}

bool Print(const struct Stack* stack)
{
  if (stack->height < 1) {
    return false;
  }

  PrintPoly(Car(stack));
  printf("\n");

  return true;
}

/** @file
  Implementacja operacji na stosie z pliku stack_op.h.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/

#include <stdlib.h>
#include <stdio.h>

#include "poly.h"
#include "poly_lib.h"
#include "stack_op.h"

/**
 * Początkowa wielkość stosu. */
#define INIT_STACK_SIZE 16

/**
 * Mnożnik względem którego rozszerza się tablica stosowa. */
#define ARR_RESIZE 2

/**
 * Wypisanie komunikatu o niedopełnieniu stosu względem operacji, jaką chciałoby
 * się na nim wykonać.
 * @param[in] linum : numer wiersza
 */
static void StackUnderflow(size_t linum)
{
  fprintf(stderr, "ERROR %lu STACK UNDERFLOW\n", linum);
}

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

void Pop(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  PolyDestroy(Car(stack));
  --stack->height;
}

void Add(struct Stack* stack, size_t linum)
{
  if (stack->height < 2) {
    StackUnderflow(linum);
    return;
  }

  PolyAddComp(Cadr(stack), Car(stack));
  Pop(stack, linum);
}

void Sub(struct Stack* stack, size_t linum)
{
  if (stack->height < 2) {
    StackUnderflow(linum);
    return;
  }

  PolyNegComp(Cadr(stack));
  Add(stack, linum);
}

void Mul(struct Stack* stack, size_t linum)
{
  Poly new;

  if (stack->height < 2) {
    StackUnderflow(linum);
    return;
  }

  new = PolyMul(Car(stack), Cadr(stack));
  Pop(stack, linum);
  Pop(stack, linum);
  PushPoly(stack, &new);

}

void Clone(struct Stack* stack, size_t linum)
{
  Poly cpy;

  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  cpy = PolyClone(Car(stack));
  PushPoly(stack, &cpy);
}

void Neg(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  PolyNegComp(Car(stack));
}

void IsCoeff(const struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  if (PolyIsCoeff(Car(stack)))
    printf("1\n");
  else
    printf("0\n");
}

void IsZero(const struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  if (PolyIsZero(Car(stack)))
    printf("1\n");
  else
    printf("0\n");
}

void IsEq(const struct Stack* stack, size_t linum)
{
  if (stack->height < 2) {
    StackUnderflow(linum);
    return;
  }

  if (PolyIsEq(Car(stack), Cadr(stack)))
    printf("1\n");
  else
    printf("0\n");
}

void Deg(const struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  printf("%d\n", PolyDeg(Car(stack)));
}

void DegBy(const struct Stack* stack, unsigned long long idx, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  printf("%d\n", PolyDegBy(Car(stack), idx));
}

void At(struct Stack* stack, poly_coeff_t x, size_t linum)
{
  Poly new;

  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  new = PolyAt(Car(stack), x);
  Pop(stack, linum);
  PushPoly(stack, &new);
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

void Print(const struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  PrintPoly(Car(stack));
  printf("\n");
}

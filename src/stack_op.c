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
 * Wypisanie komunikatu o niedopełnieniu stosu względem operacji, jaką chciałoby
 * się na nim wykonać.
 * @param[in] linum : numer wiersza
 */
static void StackUnderflow(size_t linum)
{
  fprintf(stderr, "ERROR %lu STACK UNDERFLOW\n", linum);
}

/**
 * Usunięcie z pamięci listy wielomianów.
 * @param[in] pl : głowa listy
 */
static void PolyListDestroy(struct PolyList* pl)
{
  if (!pl)
    return;

  PolyListDestroy(pl->rest);
  PolyDestroy(&pl->p);
  free(pl);
}

void StackDestroy(struct Stack* stack)
{
  PolyListDestroy(stack->list);
}

void PushPoly(struct Stack* stack, Poly* p)
{
  struct PolyList* new = malloc(sizeof(struct PolyList));

  if (!new) exit(1);

  new->rest = stack->list;
  stack->list = new;
  ++stack->height;
  new->p = *p;
}

void Pop(struct Stack* stack, size_t linum)
{
  struct PolyList* top;

  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  top = stack->list;
  --stack->height;
  stack->list = stack->list->rest;
  PolyDestroy(&top->p);
  free(top);
}

/**
 * Wzięcie wielomianu ze szczytu stosu. Nazewnictwo `car` nawiązuje do tradycji
 * znanej choćby z języków takich jak __lisp__, gdzie kluczowe były funkcje
 * `car` i `cdr` zwracające odpowiednio
 * [głowę i ogon](https://en.wikipedia.org/wiki/CAR_and_CDR)
 * listy. Jak widzą nawet osoby obeznane z tymi językami, jest to nazewnictwo
 * dość
 * [ezoteryczne](https://www.gnu.org/software/emacs/manual/html_node/eintr/Strange-Names.html)
 * i lekko _przedawnione_ (co więcej:
 * [sami stwierdzają](https://www.gnu.org/software/emacs/manual/html_node/eintr/car-_0026-cdr.html),
 * że lepszą nazwą byłoby chociażby `first`, jednakże jest to owoc długiej
 * tradycji i w ramach hołdu dla języka __lisp__ zachowam tę konwencję.
 * @param[in] stack : stos
 * @return adres pierwszego wielomianu na stosie
 */
Poly* CarStack(struct Stack* stack)
{
  assert(stack->height >= 1);
  return &stack->list->p;
}

/**
 * Złożenie tradycyjnych operacji `car` i `cdr`
 * (https://en.wikipedia.org/wiki/CAR_and_CDR#Compositions)
 * zwraca więc _głowę ogona listy_ czyli jej drugi element rzecz jasna.
 * @param[in] stack : stos
 * @return adres drugiego wielomianu ze stosu
 */
Poly* CadrStack(struct Stack* stack)
{
  assert(stack->height >= 2);
  return &stack->list->rest->p;
}

/**
 * Wykonanie (w dość naiwny sposób) operacji operatora binarnego @p op na
 * dwu wielomianach z czubka stosu @p stack.
 * @param[in] stack : stos
 * @param[in] op : operator binarny na wielomianach
 * @param[in] linum : numer linii, z której wywołano tę komendę
 */
static void BinOp(struct Stack* stack, Poly (*op)(const Poly*, const Poly*),
                  size_t linum)
{
  Poly new;

  if (stack->height < 2) {
    StackUnderflow(linum);
    return;
  }

  new = op(CarStack(stack), CadrStack(stack));
  Pop(stack, linum);
  Pop(stack, linum);
  PushPoly(stack, &new);
}

void Add(struct Stack* stack, size_t linum)
{
  BinOp(stack, PolyAdd, linum);
}

void Sub(struct Stack* stack, size_t linum)
{
  BinOp(stack, PolySub, linum);
}

void Mul(struct Stack* stack, size_t linum)
{
  BinOp(stack, PolyMul, linum);
}

void Clone(struct Stack* stack, size_t linum)
{
  Poly cpy;

  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  cpy = PolyClone(CarStack(stack));
  PushPoly(stack, &cpy);
}

void Neg(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  PolyNegComp(CarStack(stack));
}

void IsCoeff(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  if (PolyIsCoeff(CarStack(stack)))
    printf("1\n");
  else
    printf("0\n");
}

void IsZero(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  if (PolyIsZero(CarStack(stack)))
    printf("1\n");
  else
    printf("0\n");
}

void IsEq(struct Stack* stack, size_t linum)
{
  if (stack->height < 2) {
    StackUnderflow(linum);
    return;
  }

  if (PolyIsEq(CarStack(stack), CadrStack(stack)))
    printf("1\n");
  else
    printf("0\n");
}

void Deg(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  printf("%d\n", PolyDeg(CarStack(stack)));
}

void DegBy(struct Stack* stack, unsigned long long idx, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  printf("%d\n", PolyDegBy(CarStack(stack), idx));
}

void At(struct Stack* stack, poly_coeff_t x, size_t linum)
{
  Poly new;

  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  new = PolyAt(CarStack(stack), x);
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
 * Wypisanie od tyłu (jeszcze nie xd) wielomianu @p p.
 * @param[in] p : wielomian
 */
static void PrintPoly(const Poly* p)
{
  if (PolyIsCoeff(p)) {
    printf("%ld", p->coeff);
  } else {
    PrintMono(&(p->list->m));
    
    for (MonoList* ml = p->list->tail; ml; ml = ml->tail) {
      printf(" + ");
      PrintMono(&(ml->m));
    }
  }
}


void Print(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  PrintPoly(CarStack(stack));
  printf("\n");
}

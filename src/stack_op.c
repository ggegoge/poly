
#include <stdlib.h>
#include <stdio.h>

#include "poly.h"
#include "poly_lib.h"
#include "stack_op.h"

static void StackUnderflow(size_t linum)
{
  fprintf(stderr, "ERROR %lu STACK UNDERFLOW\n", linum);
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

Poly* CarStack(struct Stack* stack)
{
  return &stack->list->p;
}

Poly* CadrStack(struct Stack* stack)
{
  return &stack->list->rest->p;
}

/* TODO */
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

static void PrintPoly(Poly* p, size_t index);

/* molasie drukarki */
static void PrintMono(Mono* m, size_t index)
{
  PrintPoly(&(m->p), index + 1);
  printf("(x_%ld)^%d", index, m->exp);
}

static void PrintPoly(Poly* p, size_t index)
{
  if (PolyIsCoeff(p)) {
    printf("%ld", p->coeff);
  } else {
    printf("(");
    PrintMono(&(p->list->m), index);

    for (MonoList* ml = p->list->tail; ml; ml = ml->tail) {
      printf(" + ");
      PrintMono(&(ml->m), index);
    }

    printf(")");
  }
}


void Print(struct Stack* stack, size_t linum)
{
  if (stack->height < 1) {
    StackUnderflow(linum);
    return;
  }

  PrintPoly(CarStack(stack), 0);
  printf("\n");
}

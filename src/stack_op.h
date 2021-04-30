#ifndef _STACK_OP_H_
#define _STACK_OP_H_


#include "poly.h"

struct PolyList {
  Poly p;
  struct PolyList* rest;
};

struct Stack {
  struct PolyList* list;
  size_t height;
};

static inline struct Stack EmptyStack(void)
{
  return (struct Stack) {
    .height = 0, .list = NULL
  };
}

void StackDestroy(struct Stack* stack);

void PushPoly(struct Stack* stack, Poly* p);

void Add(struct Stack* stack, size_t linum);

void Sub(struct Stack* stack, size_t linum);

void Mul(struct Stack* stack, size_t linum);

void Clone(struct Stack* stack, size_t linum);

void Neg(struct Stack* stack, size_t linum);

static inline void Zero(struct Stack* stack)
{
  Poly p = PolyZero();
  PushPoly(stack, &p);
}

void IsCoeff(struct Stack* stack, size_t linum);

void IsZero(struct Stack* stack, size_t linum);

void IsEq(struct Stack* stack, size_t linum);

void Deg(struct Stack* stack, size_t linum);

/* todo */
void Print(struct Stack* stack, size_t linum);

void Pop(struct Stack* stack, size_t linum);

void DegBy(struct Stack* stack, unsigned long long idx, size_t linum);

void At(struct Stack* stack, poly_coeff_t x, size_t linum);

#endif

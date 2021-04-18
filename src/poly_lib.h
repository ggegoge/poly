/** @file
  Zbiór dodatkowych operacji na wielomianach z poly.h. Poszerzają tamtejszy
  interfejs oraz służą do implementacji różnorakich działań na m. in. listach.

  @authors Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date kwiecień 2021
*/

#ifndef __POLY_LIB_H__
#define __POLY_LIB_H__

#include "poly.h"

/**
 * Usunięcie z pamięci listy jednomianów.
 * @param[in] head : głowa listy do usunięcia.
 */
void MonoListDestroy(MonoList* head);

/**
 * Utworzenie pełnej kopii listy jednomianów.
 * @param[in] head : głowa listy jednomianów
 * @return pełna kopia listy
 */
MonoList* MonoListClone(const MonoList* head);

/**
 * Suma wielomianu i liczby całkowitej.
 * @param[in] coeff : współczynnik @f$ c @f$
 * @param[in] p : wielomian @f$ p(x) @f$
 * @return @f$ p(x) + c @f$
 */
Poly PolyAddCoeff(poly_coeff_t coeff, const Poly* p);

/**
 * Suma dwu wielomianów, ale w wersji `compound assignment' tj nie tworzy
 * nowego wielomianu, a jedynie modyfikuje ten ,,po lewej''. Odpowiednik
 * operatora `+=`.
 * @param[in] p : wielomian @f$ p @f$
 * @param[in] q : wielomian @f$ q @f$
 * Wykonuje `p += q`.
 */
void PolyAddComp(Poly* p, const Poly* q);

/**
 * Wstawienie komórki listowej z jednomianem w odpowiednie miejsce listy.
 * @param[in] head : głowa listy
 * @param[in] new : komórka listy z nowym elementem
 */
void MonoListInsert(MonoList** head, MonoList* new);

/**
 * Sprawdzian czy komórka listy @p ml nie jest przypadkiem ,,pseudo
 * wykładnikiem''. Funkcja mówi czy nie jest to przypadkiem lista, w którą
 * zaledwie zapakowany jest wielomian współczynnikowy -- mowa o sytuacji
 * typu @f$ c * x^0 @f$.
 * @param[in] ml : wskaźnik na komórkę listy
 * @return czy to nie pseudowykładnik?
 */
bool PolyIsPseudoCoeff(const MonoList* ml);

/**
 * Zmiana pseudowykładnika w normalny. Funkcja bierze wielomian @p p będący
 * pseudowykładnikiem (patrz: `PolyIsPseudoCoeff` celem zrozumienia pojęcia)
 * i zmienia go w standardowy wykładnik.
 * @param[in] p : wielomian będący pseudo wykładnikiem
 */
void Decoeffise(Poly* p);

/**
 * Iloczyn wielomianu ze skalarem.
 * @param[in] coeff : współczynnik @f$ c @f$
 * @param[in] p : wielomain @f$ p(x) @f$
 * @return @f$ c p(x) @f$
 */
Poly PolyMulCoeff(poly_coeff_t coeff, const Poly* p);

/**
 * Iloczyn jednomianów.
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 * @return iloczyn @f$ m * p @f$
 */
Mono MonoMul(const Mono* m, const Mono* t);

/**
 * Uprzeciwnienie wielomianu samego w sobie. Nie zwraca nowego wielomianu tylko
 * neguje ten otrzymany. Dokładniej rzecz biorąc neguje jego współczynniki
 * liczbowe. Coś a la `p *= -1`.
 * @param[in] p : wielomian @f$ p @f$
*/
void PolyNegComp(Poly* p);

/**
 * Obliczenie współczynnika wielomianu stałego.
 * @param[in] p : wielomian stały
 * @return stopień wielomianu
 */
poly_exp_t PolyCoeffDeg(const Poly* p);

/**
 * Obliczenie stopnia wielomianu zawartego w niepustej liście. Ze wzglęfu na
 * zachowane posortowanie listy, ten stopień jest to potęga przy pierwszym
 * jednomianie będącym w tejże liście.
 * @param[in] head : niepusta lista
 * @return stopień wielomianu reprezentowanego przez listę
 */
poly_exp_t MonoListDeg(const MonoList* head);

/**
 * Sprawdzian równości dwu jednomianów.
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 * @return czy @p m i @p t są równe
 */
bool MonoIsEq(const Mono* m, const Mono* t);



#endif /* __POLY_LIB_H__ */

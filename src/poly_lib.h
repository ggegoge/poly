/** @file
  Zbiór dodatkowych operacji na wielomianach z poly.h. Poszerzają tamtejszy
  interfejs oraz służą do implementacji różnorakich działań na m. in. listach,
  ale także są tutaj odpowiedniki działania `+=` (tak jak w poly.h mamy `+`).

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
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
 * @return pełna kopia listy.
 */
MonoList* MonoListClone(const MonoList* head);

/**
 * Wstawienie jednomianu w odpowiednie miejsce listy.
 * @param[in,out] head : głowa listy
 * @param[in] m : nowy jednomian
 */
void MonoListInsert(MonoList** head, Mono* m);

/**
 * Suma wielomianu i liczby całkowitej.
 * @param[in] coeff : współczynnik @f$ c @f$
 * @param[in] p : wielomian @f$ p(x) @f$
 * @return @f$ p(x) + c @f$
 */
Poly PolyAddCoeff(const Poly* p, poly_coeff_t coeff);

/**
 * Suma dwu wielomianów, ale w wersji `compound assignment' tj nie tworzy
 * nowego wielomianu, a jedynie modyfikuje ten ''po lewej''. Odpowiednik
 * operatora `+=`. Dla @p p i @p q wykonuje `p += q`.
 * @param[in,out] p : wielomian @f$ p @f$
 * @param[in] q : wielomian @f$ q @f$
 */
void PolyAddComp(Poly* p, const Poly* q);

/**
 * Sprawdzian czy komórka listy @p ml nie jest przypadkiem
 * ''pseudowspółczynnikiem''. Funkcja mówi czy nie jest to przypadkiem lista,
 * w którą zaledwie zapakowany jest wielomian współczynnikowy -- mowa o sytuacji
 * typu @f$ c \cdot x^0 @f$. Istnienie pseudowspółczynnika ma wbrew pozorom głęboki
 * sens -- trzymamy listy __jednomianów__ ergo chcąc tam schować współczynnik
 * (tj wielomian) musimy go przebrać. Stąd wspomniana reprezentacja.
 * @param[in] ml : komórka listy jednomianów
 * @return czy to nie pseudowspółczynnik?
 */
bool PolyIsPseudoCoeff(const MonoList* ml);

/**
 * Zmiana pseudowspółczynnika w normalny. Funkcja bierze wielomian @p p będący
 * pseudowspółczynnikiem (patrz: `PolyIsPseudoCoeff` celem zrozumienia pojęcia)
 * i zmienia go w standardowy wykładnik.
 * @param[in,out] p : wielomian będący pseudo wykładnikiem
 */
void Decoeffise(Poly* p);

/**
 * Iloczyn wielomianu ze skalarem.
 * @param[in] coeff : współczynnik @f$ c @f$
 * @param[in] p : wielomain @f$ p(x) @f$
 * @return @f$ c p(x) @f$
 */
Poly PolyMulCoeff(const Poly* p, poly_coeff_t coeff);

/**
 * Uprzeciwnienie wielomianu @p p.
 * @param[in,out] p : wielomian
 */
void PolyNegComp(Poly* p);

/**
 * Iloczyn jednomianów.
 * @param[in] m : jednomian
 * @param[in] t : jednomian
 * @return iloczyn @f$ m \cdot p @f$
 */
Mono MonoMul(const Mono* m, const Mono* t);

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

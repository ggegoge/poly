/** @file
  Zbiór dodatkowych operacji na wielomianach z poly.h. Poszerzają tamtejszy
  interfejs oraz służą do implementacji różnorakich działań na m. in. listach,
  ale także są tutaj odpowiedniki działania `+=` (tak jak w poly.h mamy `+`).

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date kwiecień -- czerwiec 2021
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
 * Porównanie dwu jednomianów po ich wykładnikach.
 * @param[in] m : wskaźnik na pierwszy z jednomianów
 * @param[in] t : wskaźnik na dru1gi z jednomianów
 * @return -1 gdy wykładnik @p m jest mniejszy od wykładnika @p t, w przeciwnym
 * przypadku 1, 0 oznacza równość -- konwencja zgodna ze zwyczajową C, wystarczy
 * sprawdzić znak aby poznać porządek między dwójką jednomianów.
 */
int MonoCmp(const Mono* m, const Mono* t);

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
 * Włączenie wielomianu @p q w wielomian @p p. Wielomian @p p przejmuje całą
 * pamięć należącą do @p q i wszystkie jego jednomiany na własność.
 * Efektywniejsza wersja @ref PolyAddComp, które było zaledwie `+=` -- prawy
 * wielomian pozostawał bez zmian. Tutaj mamy tak jakby ''`+=+`'', łączymy
 * @p p i @p q bez żadnego kopiowania. Po wykonaniu tej operacji wielomiany
 * @p p i @p q są tymi samymi strukturami.
 * @param[in,out] p : wielomian w którym zostanie wynik
 * @param[in,out] q : wielomian do pożarcia i włączenia
 * @return `p +=+ q`
 */
Poly* PolyIncorporate(Poly* p, Poly* q);

/**
 * Sprawdzian czy komórka listy @p ml nie jest przypadkiem
 * ''pseudowspółczynnikiem''. Funkcja mówi czy nie jest to przypadkiem lista,
 * w którą zaledwie zapakowany jest wielomian współczynnikowy -- mowa o sytuacji
 * typu @f$ c \cdot x^0 @f$. Istnienie pseudowspółczynnika ma wbrew pozorom
 * głęboki sens -- trzymamy listy __jednomianów__ ergo chcąc tam schować
 * współczynnik (tj wielomian) musimy go przebrać. Stąd też wspomniana
 * reprezentacja.
 * @param[in] ml : komórka listy jednomianów
 * @return czy to nie pseudowspółczynnik?
 */
bool PolyIsPseudoCoeff(const MonoList* ml);

/**
 * Zmiana pseudowspółczynnika w normalny. Funkcja bierze wielomian @p p będący
 * pseudowspółczynnikiem (patrz: @ref PolyIsPseudoCoeff celem zrozumienia
 * pojęcia) i zmienia go w standardowy wykładnik.
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
 * Podnoszenie wielomianu @p p do potęgi @p n.
 * @param[in] p : wielomain @f$p@f$
 * @param[in] n : wykładnik @f$n@f$
 * @return spotęgowany wielomian @f$p^n@f$ */
Poly PolyPow(const Poly* p, poly_coeff_t n);

/**
 * Tworzenie tablicy potęg wielomianu @p q celem podstawienia go do @p p.
 * Rozmiar tablicy zapisany zostanie pod @p count. Tablica ma formę
 * @f$\{q,q^2,q^4,...,q^{2^k}\}@f$, gdzie @f$k=\log \deg p@f$ czyli właśnie
 * @p count. Później można obliczać potęgi wymnażając wartości z tej tabeli
 * korzystając z rozkładu binarnego @f$n@f$ w @f$q^n@f$.
 * @param[in] p : wielomian pod który podstawiamy @p q
 * @param[in] q : wielomian do spotęgowania
 * @param[out] count : wielkość tablicy potęg
 * @return tablica potęg @p q celem wymnożenia ich w @p p
 */
Poly* PolyPowTable(const Poly* p, const Poly* q, size_t* count);

/**
 * Wyliczenie konkretnej potęgi za pomocą tablicy @p powers. Korzysta z rozkładu
 * liczb na sumę potęg dwójki stąd mając np tablicę @p powers z na przykład
 * @f$\{q,q^2,q^4\}@f$ można policzyć @f$q^6 = q^4 \cdot q^2@f$.
 * @param[in] powers : tablica potęgowa
 * @param[in] n : wykładnik
 * @return odpowiednia potęga
 */
Poly PolyGetPow(const Poly powers[], size_t n);

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

/**
 * Dokonanie głębokiej kopii tablicy @p monos.
 * @param[in] count : wielkość tablicy
 * @param[in] monos : rzeczona tablica
 * @return głęboka kopia tablicy i jej wielomianów
 */
Mono* CloneMonoArray(size_t count, const Mono monos[]);

#endif /* __POLY_LIB_H__ */

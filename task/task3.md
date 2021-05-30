

# Duże zadanie, część 3 {#t3}


# Spis treści

1.  [Duże zadanie, część 3](#orged607db)
    1.  [Konstruowanie wielomianu z tablicy jednomianów](#org4a0d3fc)
    2.  [Składanie wielomianów](#org778dd99)
    3.  [Dokumentacja](#orgcee7feb)
    4.  [Modyfikacja skryptu budującego](#orgeb3bb14)
    5.  [Wymagania](#org2779cb2)
    6.  [Oddawanie rozwiązania](#org6c787db)
    7.  [Punktacja](#orgcaf64a7)

Celem trzeciej części zadania jest dokończenie programu kalkulatora działającego na
wielomianach rzadkich wielu zmiennych. Oczekujemy poprawienia ewentualnych błędów z
poprzednich części zadania oraz wprowadzenie opisanych poniżej modyfikacji i
rozszerzeń. Obowiązują ustalenia z treści poprzednich części zadania i z forum
dyskusyjnego dla studentów.


<a id="org4a0d3fc"></a>

## Konstruowanie wielomianu z tablicy jednomianów

Do stworzonej w części 1 zadania biblioteki operacji na wielomianach poly (pliki poly.h
i poly.c) należy dodać dwie funkcje działające podobnie jak funkcja PolyAddMonos, ale
ze zmienionym sposobem przekazywania własności tablicy jednomianów:

    /**
     * Sumuje listę jednomianów i tworzy z nich wielomian. Przejmuje na własność
     * pamięć wskazywaną przez @p monos i jej zawartość. Może dowolnie modyfikować
     * zawartość tej pamięci. Zakładamy, że pamięć wskazywana przez @p monos
     * została zaalokowana na stercie. Jeśli @p count lub @p monos jest równe zeru
     * (NULL), tworzy wielomian tożsamościowo równy zeru.
     * @param[in] count : liczba jednomianów
     * @param[in] monos : tablica jednomianów
     * @return wielomian będący sumą jednomianów
     */
    Poly PolyOwnMonos(size_t count, Mono *monos);
    /**
     * Sumuje listę jednomianów i tworzy z nich wielomian. Nie modyfikuje zawartości
     * tablicy @p monos. Jeśli jest to wymagane, to wykonuje pełne kopie jednomianów
     * z tablicy @p monos. Jeśli @p count lub @p monos jest równe zeru (NULL),
     * tworzy wielomian tożsamościowo równy zeru.
     * @param[in] count : liczba jednomianów
     * @param[in] monos : tablica jednomianów
     * @return wielomian będący sumą jednomianów
     */
    Poly PolyCloneMonos(size_t count, const Mono monos[]);

Przy implementowaniu tych funkcji należy unikać powtarzania kodu.

Użyte w poniższych przykładach makra `C` i `P` oraz funkcja `M` są zdefiniowane w testach do części 1 zadania, w pliku `poly_test.c.`


### Przykład 1

Funkcja `PolyOwnMonos` przejmuje na własność zarówno zawartość tablicy `monos`, jak i
pamięć zajmowaną przez tę tablicę. Wywołanie funkcji `PolyDestroy` musi zwolnić
wszystkie zasoby.

    Mono *monos = calloc(2, sizeof (Mono));
    assert(monos);
    monos[0] = M(P(C(-1), 1), 1);
    monos[1] = M(P(C(1), 1), 2);
    Poly p = PolyOwnMonos(2, monos);
    PolyDestroy(&p);


### Przykład 2

Funkcja `PolyCloneMonos` nie przejmuje na własność żadnych zasobów i nie modyfikuje
zawartości tablicy monos. Utworzone wielomiany `p1` i `p2` są takie same. Oprócz wywołań
funkcji `PolyDestroy` konieczne są wywołania funkcji `MonoDestroy` i zwolnienie pamięci za
pomocą funkcji `free`.

    Mono *monos = calloc(2, sizeof (Mono));
    assert(monos);
    monos[0] = M(P(C(-1), 1), 1);
    monos[1] = M(P(C(1), 1), 2);
    Poly p1 = PolyCloneMonos(2, monos);
    Poly p2 = PolyCloneMonos(2, monos);
    PolyDestroy(&p1);
    PolyDestroy(&p2);
    MonoDestroy(monos + 0);
    MonoDestroy(monos + 1);
    free(monos);


<a id="org778dd99"></a>

## Składanie wielomianów

Definiujemy operację składania wielomianów. Dany jest wielomian @f$p@f$ oraz @f$k@f$
wielomianów @f$q_0 ,q_1 ,..q_{k-1}@f$
Niech @f$l@f$ oznacza liczbę zmiennych wielomianu @f$p@f$ i niech te zmienne są oznaczone
odpowiednio @f$x_0,..., x_{l-1}@f$. Wynikiem złożenia jest wielomian @f$p(q_0, q_1, ...)@f$
czyli wielomian powstający przez podstawienie w wielomianie @f$p@f$ pod zmienną @f$x_i@f$
wielomianu @f$q_i@f$ dla @f$i=0,1,..,\min(k,l)-1@f$. Jeśli @f$k < l@f$, to pod zmienne
@f$x_k,...,x_{l-1}@f$ podstawiamy zera. Na przykład, jeśli @f$k=0@f$, to wynikiem złożenia jest
liczba @f$p(0,0,0...)@f$.

W celu realizacji operacji składanie wielomianów należy rozszerzyć bibliotekę `poly` o funkcję

    Poly PolyCompose(const Poly *p, size_t k, const Poly q[]);

Do interfejsu kalkulatora należy dodać polecenie

`COMPOSE k`

Polecenie to zdejmuje z wierzchołka stosu najpierw wielomian `p`, a potem kolejno
wielomiany `q[k - 1], q[k - 2], …  …  , q[0]` i umieszcza na stosie wynik operacji
złożenia.

Jeśli w poleceniu `COMPOSE` nie podano parametru lub jest on niepoprawny, program
powinien wypisać na standardowe wyjście diagnostyczne:

`ERROR w COMPOSE WRONG PARAMETER\n`

Wartość parametru polecenia `COMPOSE` uznajemy za niepoprawną, jeśli jest mniejsza od `0`
lub większa od `18446744073709551615`.

Jeśli na stosie jest za mało wielomianów, aby wykonać polecenie, program powinien
wypisać na standardowe wyjście diagnostyczne:

`ERROR w STACK UNDERFLOW\n`

Jak poprzednio w obu przypadkach `w` oznacza numer wiersza, a `\n` – znak przejścia do
nowego wiersza.


### Przykład 1

Dla danych wejściowych:

    (1,2)
    (2,0)+(1,1)
    COMPOSE 1
    PRINT
    (1,3)
    COMPOSE 1
    PRINT

Jako wynik działania programu powinniśmy zobaczyć:

    (2,0)+(1,2)
    (8,0)+(12,2)+(6,4)+(1,6)

Wyjaśnienie do przykładu:

Pierwsze polecenie `COMPOSE` podstawia wielomian @f$x^2_0@f$ pod @f$x_0@f$ w wielomianie
@f$(2 +x_0)@f$, więc w jego wyniku otrzymujemy wielomian @f$(2+𝑥20)@f$.

 Drugie polecenie `COMPOSE` podstawia wielomian @f$(2+x^2_0)@f$
pod @f$x_0@f$  w wielomianie @f$x_0^3@f$, więc w jego wyniku otrzymujemy wielomian
@f$(8+12x_0^2+6x_0^4+x_0^6)@f$.


### Przykład 2

Dla danych wejściowych:

    (1,4)
    ((1,0)+(1,1),1)
    (((1,6),5),2)+((1,0)+(1,2),3)+(5,7)
    COMPOSE 2
    PRINT

Jako wynik działania programu powinniśmy zobaczyć:

    (1,12)+((1,0)+(2,1)+(1,2),14)+(5,28)

Wyjaśnienie do przykładu:

Polecenie `COMPOSE` podstawia do wielomianu
@f$p = x_2^6 x_1^5 x_0^2 + \left(1 + x_1^2\right) x_0^3 + 5 x_0^7@f$:

-   wielomian @f$x_0^4@f$ pod @f$x_0@f$
-   wielomian @f$(1 + x_1) x_0@f$ pod @f$x_1@f$
-   0 pod @f$x_2@f$

W rezultacie:

-   wyraz @f$x_2^6 x_1^5 x_0^2@f$ przechodzi w @f$0@f$
-   wyraz @f$\left(1 + x_1^2\right)@f$ przechodzi w @f$\left(1 + \left(1 + 2x_1 + x_1^2\right) x_0^2\right)@f$
-   wyraz @f$x_0^3@f$ przechodzi w @f$x_0^12@f$
-   wyraz @f$5x_0^7@f$ przechodzi w @f$5x_0^{28}@f$

Zatem cały wielomian @f$p@f$ przechodzi w wielomian:

@f[0 + \left(1 + \left(1 + 2x_1 + x_1^2\right) x_0^2\right) x_0^{12} + 5 x_0^{28} =
    x_0^{12} + \left(1 + 2x_1 + x_1^2\right) x_0^{14} + 5 x_0^{28}. @f]


### Przykład 3

Dla danych wejściowych:

    ((1,0)+(1,1),1)
    (1,4)
    COMPOSE -1
    COMPOSE 18446744073709551615

Jako wynik działania programu powinniśmy zobaczyć:

    ERROR 3 COMPOSE WRONG PARAMETER
    ERROR 4 STACK UNDERFLOW


<a id="orgcee7feb"></a>

## Dokumentacja

Dodany kod należy udokumentować w formacie `doxygen`.


<a id="orgeb3bb14"></a>

## Modyfikacja skryptu budującego

Należy dodać możliwość utworzenia pliku wykonywalnego z testami biblioteki `poly`. Czyli
na przykład po wykonaniu:

    mkdir release
    cd release
    cmake ..

-   polecenie `make` tworzy plik wykonywalny `poly` całego kalkulatora,
-   polecenie `make test` tworzy plik wykonywalny `poly_test` z testami biblioteki `poly`,
-   polecenie `make doc` tworzy dokumentację w formacie `doxygen`.

Funkcja `main` kalkulatora ma się znajdować w pliku `src/calc.c`. Funkcja `main`
uruchamiająca testy biblioteki `poly` ma się znajdować w pliku `src/poly_test.c` – może
to być plik z udostępnionymi testami do części 1 zadania i rozszerzony o własne
testy. Zawartość tego pliku nie będzie oceniana.

Wskazówka: W pliku CMakeList.txt można dodać polecenia

    # Wskazujemy plik wykonywalny testów biblioteki.
    add_executable(test EXCLUDE_FROM_ALL ${TEST_SOURCE_FILES})
    set_target_properties(test PROPERTIES OUTPUT_NAME poly_test)

definiując uprzednio zmienną `TEST_SOURCE_FILES`.


<a id="org2779cb2"></a>

## Wymagania

Rozwiązanie części 3 zadania powinno korzystać z własnego rozwiązania poprzednich jego
części. Obowiązują wszystkie wymagania z poprzednich części zadania, jeśli nie zostały
zmienione w tym dokumencie.

**Uwaga: niezmiernie istotne jest, aby przestrzegać opisanej specyfikacji nazw plików.**


<a id="org6c787db"></a>

## Oddawanie rozwiązania

Rozwiązanie należy oddawać, podobnie jak części 1 i 2, przez repozytorium git. W
repozytorium mają się znaleźć wszystkie pliki niezbędne do zbudowania plików
wykonywalnych i dokumentacji, i tylko te pliki. W repozytorium nie wolno umieszczać
plików binarnych ani tymczasowych. W Moodle jako rozwiązanie należy umieścić tekst
zawierający identyfikator commitu finalnej wersji rozwiązania, na przykład:

`518507a7e9ea50e099b33cb6ca3d3141bc1d6638`

Rozwiązanie należy zatwierdzić (`git commit`) i wysłać do repozytorium (`git push`) przed
terminem podanym w Moodle.


<a id="orgcaf64a7"></a>

## Punktacja

Za w pełni poprawną implementację programu można zdobyć maksymalnie 20 punktów. Od tej
oceny będą odejmowane punkty za poniższe uchybienia:

-   Za problemy ze skompilowaniem rozwiązania można stracić wszystkie punkty.
-   Za każdy test, którego program nie przejdzie, traci się do 1 punktu.
-   Za problemy z zarządzaniem pamięcią można stracić do 6 punktów.
-   Za niezgodną ze specyfikacją strukturę plików w rozwiązaniu, niezgodne ze
    specyfikacją nazwy plików w rozwiązaniu lub umieszczenie w repozytorium
    niepotrzebnych albo tymczasowych plików można stracić do 4 punktów.
-   Za złą jakość kodu, brzydki styl kodowania można stracić do 4 punktów.
-   Za ostrzeżenia wypisywane przez kompilator można stracić do 2 punktów.
-   Za braki w dokumentacji można stracić do 2 punktów.
-   Za niezaimplementowanie funkcji `PolyOwnMonos` lub `PolyCloneMonos` można stracić do 2
    punktów.
-   Za niezaimplementowanie funkcji `PolyCompose` lub polecenia `COMPOSE` można stracić do 2
    punktów.


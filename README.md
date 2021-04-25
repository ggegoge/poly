### Treść zadania

Aktualna treść zadania znajduje się
w [Moodle](https://moodle.mimuw.edu.pl), ale zarazem w pliku task.md,
który można ujrzeć w sekcji [_dodatkowe strony_](pages.html).

### Opis programu

Tegoroczne duże zadanie polega na zaimplementowaniu operacji na wielomianach
rzadkich wielu zmiennych.

Program został podzielony na dwa moduły.
1. `poly`
2. `poly_lib`

#### Pliki nagłówkowe

Interfejs jest w pliku `poly.h`, który poddałem jedynie nieznacznym
zmianom (z czego najważniejszą jest zmiana pola tablicowego w
wielomianie na pole _listowe_).

Do tego dodałem `poly_lib.h`, który jednocześnie w pewnym stopniu
_poszerza_ zawartość `poly.h` (najznamienitszym tego przejawem są
funkcje typu `+=`) jak i zawiera deklaracje istotnych dla
programu funkcji operujących na listach wskaźnikowych.

#### Pliki .c

Plik `poly.c` zawiera rzecz jasna implementację funkcji z
`poly.h`. Staram się tam trzymać __jedynie tamtejsze funkcje__
tj. jeśli coś wykracza poza `poly.h`, to jest częścią modułu
`poly_lib`. Ta prawidłowość zachodzi w większości
przypadków. Wyjątkiem są głównie jakiekolwiek operacje, których
używają funkcje z `poly`, a które wykonują jakieś proste łatwe
rzeczy pozbawione związku z samymi listami czy wielomianami. Myślę tu
o funkcjach czysto liczbowych takich jak maksimum czy potęga. Nie są w
żaden sposób częścią _szerokiej biblioteki wielomianowej_ jak można by
nazwać `poly_lib`.

Plik `poly_lib.c` składa się z funkcji zadeklarowanych w
odpowiadającym sobie pliku nagłówkowym jak i niektórych dodatkowych,
które deklaruję jako `static` albowiem nie korzystam z nich poza tym
modułem. Funkcje z tego pliku można zasadniczo podzielić na dwie
kategorie -- operujące na listach oraz operatory __compound
assignment__. Jak np. funkcja `PolyAdd` z `poly.h` odpowiada
operatorowi `+` na wielomianach, tak `PolyAddComp` z `poly_lib.h` jest
niczym operator `+=` w tym sensie, że zmienia lewy operand, a nie
zwraca nowy obiekt. 

##### Nazewnictwo

Wszelakie nazwy funkcji zachowuję w konwencji `PascalCase` zgodnie z
danym uprzednio `poly.h`.

Konwencja nazewnicza z sufiksem `Comp` (od `compound assignment') jest
w większości wypadków zachowana, wyjątkiem być może są funkcje na
listach, które mają nazwy oparte na bezpośredniej operacji na liście,
którą mają wykonać.

Do tego dodanie `Coeff` do nazwy często oznacza operacje na
wielomianie stałym.

#### Złożoność i działanie

Złożoność dodawania jest rzędu _O(n)_. Dokonuję to scalaniem à la
merge sort listy dwu wielomianów, powstaje pewnego rodzaju splot tych
list, po każdej przechodzę raz, ergo liniowość.

Mnożenie jest zwykłym naiwnym algorytmem kwadratowym.

### Wygląd dokumentacji

W pliku `Doxyfile.in` mam linijkę
```
HTML_EXTRA_STYLESHEET  = ../css/doxygen_theme_flat.css
```
która odpowiada za wygląd tej dokumentacji -- korzystam z cssa
[_z tego repozytorium gitowego_](https://github.com/kcwongjoe/doxygen_theme_flat_design),
które jest olicensjonowane licencją MIT (jest zatem open
source). Licencę zresztą załączam w katalogu `css` zgodnie z jej warunkami.

Dokonałem jedynie nieznacznych poprawek nadając linkom elegancki
różowy kolor.

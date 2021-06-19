
## Wielomiany

### Treść zadania

Aktualna treść zadania znajduje się
w [Moodle](https://moodle.mimuw.edu.pl), ale zarazem w plikach z
katalogu `task/`, które można ujrzeć w sekcji _dodatkowe strony_.

### Opis programu

Tegoroczne duże zadanie polega na zaimplementowaniu operacji na wielomianach
rzadkich wielu zmiennych oraz kalkulatora wielomianowego
korzystającego z tych operacji (opartego na drugiej najlepszej notacji
(_ONP_ czyli _odwrotna notacja polska_) i mechanizmie stosu kalkulatora).

Program został podzielony na moduły.
1. `poly` -- wielomianowa biblioteka standardowa
2. `poly_lib` -- rozszerzenie `poly` i funkcje pomocnicze dlań
3. `calc` -- interaktywny tekstowy kalkulator
4. `parse` -- moduł odpowiedzialny za wczytywanie z tekstu komend
   wielomianowych i wywoływanie odpowiednich operacji na 
   __stosie kalkulatora__
5. `stack_op` -- właściwa obsługa rzeczonego stosu

### Użycie kalkulatora

Jest opisane w poleceniu do drugiej części. Dodatkowo dostarczam
dodanie opcji `-p` lub `---pretty` która uruchamia przyjemniejszy
interfejs użytkowniczy. Działają też wtedy komendy pisane małymi
literami co ułatwia użytek.


    λ ./poly --pretty
    ---< Poly Calc >-----------< v. 1.0 >----
    |1|> (1,1)+(1,0)
    |2|> clone
    |3|> clone
    |4|> mul
    |5|> mul
    |6|> print
    (1,0)+(3,1)+(3,2)+(1,3)
    |7|> (1,1)+(1,0)
    |8|> (1,3)
    |9|> compose 1
    |10|> print
    (1,0)+(3,1)+(3,2)+(1,3)
    |11|> is_eq
    1
    |12|> 
    |13|> pop
    |14|> pop


Jest to jednak jedynie __dodatek__, oficjalna wersja zakłada używanie
samego `./poly`.

#### Pliki nagłówkowe

Interfejs biblioteki działań na wielomianach jest w pliku `poly.h`,
który poddałem jedynie nieznacznym zmianom (z czego najważniejszą jest
zmiana pola tablicowego w wielomianie na pole _listowe_).

Do tego dodałem `poly_lib.h`, który jednocześnie w pewnym stopniu
_poszerza_ zawartość `poly.h` (najznamienitszym tego przejawem są
funkcje typu `+=`) jak i zawiera deklaracje istotnych dla
programu funkcji operujących na listach wskaźnikowych.

Plik `parse.h` zawiera wylistowane funkcje przetwarzające linijki z
wejścia, do użytku przez moduł `calc`.

W `stack_op.h` zawarty został interfejs operacji na stosie. Każda z
nich otrzymuje stos i numer linii z której została wywołana (aby móc
raportować błędy)

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

`calc.c` zawiera główną funkcję programu i odpowiada za utworzenia
stosu i przekazywanie pojedynczych linijek i stosu do `parse`.

W `parse.c` są cztery najważniejsze funkcje: `ParseLine`,
`Parse{Poly,Mono}`, `ParseCommand` -- nazwy mówią same za siebie.

`stack_op.c` zawiera implementację funkcji z `stack_op.h`

##### Nazewnictwo

Wszelakie nazwy funkcji zachowuję w konwencji `PascalCase` zgodnie z
danym uprzednio `poly.h`.

W `poly_lib` konwencja nazewnicza z sufiksem `Comp` (od `compound assignment') jest
w większości wypadków zachowana, wyjątkiem być może są funkcje na
listach, które mają nazwy oparte na bezpośredniej operacji na liście,
którą mają wykonać.

Do tego dodanie `Coeff` do nazwy często oznacza operacje na
wielomianie stałym.

Również po części z chęci uniknięcia jakichś nadmiernie irytujących
dwusłownych funkcji (dla których brak jasnej konwencji. `camelCase`?
ohydka) zgodnie z filozofią języka C używałem chociażby słowa `linum`
na `line number`. Warto zacytować stosowny ustęp 
[Kernel Coding Style](https://www.kernel.org/doc/html/v4.10/process/coding-style.html#naming):

> C is a Spartan language, and so should your naming be. Unlike Modula-2
> and Pascal programmers, C programmers do not use cute names like
> `ThisVariableIsATemporaryCounter`. A C programmer would call that
> variable `tmp`, which is much easier to write, and not the least more
> difficult to understand.

#### Testy

Bibliotekę z poly.h testuje plik poly_test.c. Komendą `make test` można
utworzyć plik wykonywalny `./poly_test`, który testuje różne operacje.

Należy wywołać `./poly_test all` lub `./poly_test` celem puszczenia
wszystkich testów bądź `./poly_test <nazwa testu>`, gdzie nazwy testów
są do znalezienia we wspomnianym `poly_test.c`.

O powodzeniu testu świadczy kod wyjścia równy 0. W przypadku błędu kod
wyniesie 2.

#### Złożoność i działanie

Złożoność dodawania jest rzędu _O(n)_. Dokonuję to scalaniem à la
merge sort listy dwu wielomianów, powstaje pewnego rodzaju splot tych
list, po każdej przechodzę raz, ergo liniowość.

Mnożenie jest zwykłym naiwnym algorytmem kwadratowym.

Składanie wielomianów wykonywane jest reukurencyjnie 
(\ref PolyCompose) i w dużej części opiera się na potęgowaniu
wielomianu podstawianego pod zmienną. To potęgowanie natomiast robione
jest na dwa sposoby:
- gdy wielomian ma _wiele_ (\ref PolyHasManyMonos) jednomianów (co pociąga za sobą konieczność
  wykonania licznych potęgowań na wielomianie podstawianym) obliczamy
  za wczasu tabelę potęg dwójkowych (\ref PolyPowTable) z których później
  obliczamy porządne konkretne potęgi (\ref PolyGetPow). Pozwala to na
  pokaźne zaoszczędzenie na pamięci w przypadku licznego potęgowania
  tego samego wielomianu
- gdy natomiast wielomian ma jednomianów _niewiele_ to używamy
  zwykłego potęgowania szybkiego (\ref PolyPow)
  
W każdym razie potęgowanie jest __logarytmiczne__.

Operacje na stosie dziedziczą złożoność ze zwykłej
biblioteki. Dodatkowo została zastosowana optymalizacja możliwa dzięki
`poly_lib.h` -- operatory `+=`. Dzięki temu dodając wielomiany z góry
stosu nie muszę tworzyć trzeciego, zrzucać dwu i go wstawiać, a po
prostu zrobić `+=` dla drugiego i zrzucić jedynie ten najwyższy.

### Wygląd dokumentacji

W pliku `Doxyfile.in` mam linijkę

    HTML_EXTRA_STYLESHEET  = ../css/doxygen_theme_flat.css

która odpowiada za wygląd tej dokumentacji -- korzystam z cssa
[_z tego repozytorium gitowego_](https://github.com/kcwongjoe/doxygen_theme_flat_design),
które jest olicensjonowane licencją MIT (jest zatem open
source). Licencę zresztą załączam w katalogu `css` zgodnie z jej warunkami.

Dokonałem jedynie nieznacznych poprawek nadając linkom elegancki
różowy kolor.

------------

\author Grzegorz Cichosz
\date kwiecień, maj i czerwiec 2021

/** @file
  Interaktywny kalkulator działający na wielomianach wielu zmiennych.

  @author Grzegorz Cichosz <g.cichosz@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date maj 2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "stack_op.h"
#include "poly.h"
#include "parse.h"

/**
 * Znacznik komentarza. */
#define COMMENT_MARKER '#'

void Interpret(struct Stack*, bool prettification);

/**
 * Główna procedura programu, włącza właściwy interpreter i inicjalizuje stos
 * kalkulatora. Przyjmuje argumenty z linii poleceń: jeśli jest nim `-p` bądź
 * `--pretty` to będzie wypisywać dodatkowo lekkie uładnienia dla użytkownika.
 */
int main(int argc, char* argv[])
{
  struct Stack stack = EmptyStack();

  if (argc >= 2 && (strcmp(argv[1], "-p") == 0 ||
                    strcmp(argv[1], "--pretty") == 0)) {
    printf("---< Poly Calc >-----------< v. 0.4 >----\n");
    Interpret(&stack, true);
  } else {
    Interpret(&stack, false);
  }

  StackDestroy(&stack);
  return 0;
}

static bool UpperLine(char* s, size_t line_len);

static ssize_t
ReadLn(char** ptr, size_t* size, bool* is_eof, bool* is_comment);

/**
 * Sprawdzian pustości linii (all white == pusta (_na razie_)).
 * @param[in] line : linia
 * @param[in] len : długość linii
 * @return czy linia jest pusta
 */
static bool empty(char* line, size_t len)
{
  bool is_empty = true;

  for (size_t i = 0; i < len && is_empty; ++i)
    is_empty = isspace(line[i]);

  return is_empty;
}

/**
 * Czytanie komend ze standardowego wejścia.
 * @param[in] stack : stos kalkulatora
 * @param[in] prettification : jeśli jest `true` to dodaje pseudo prompt
 */
void Interpret(struct Stack* stack, bool prettification)
{
  ssize_t len;
  size_t linum = 1;
  size_t size = 0;
  char* line = NULL;
  bool is_comment = false;
  bool is_eof = false;


  while (!feof(stdin) && !is_eof) {
    if (prettification)
      printf("|%lu|> ", linum);

    len = ReadLn(&line, &size, &is_eof, &is_comment);

    if (!is_comment && !is_eof && !empty(line, len)) {
      if (prettification)
        UpperLine(line, len);

      ParseLn(line, len, linum, stack);
    }

    ++linum;
  }

  free(line);
}

/**
 * Wczytywanie pojedynczych linii ze standardowego wejścia.
 * @param[in] ptr : bufor do zapisywania linii
 * @param[in] size : wielkość bufora
 * @param[out] is_eof : czy ta linia zawiera koniec wejścia
 * @param[out] is_comment : czy to nie linijka komentarzowa
 * @return długość wczytanej linii
 */
static ssize_t ReadLn(char** ptr, size_t* size, bool* is_eof,
                         bool* is_comment)
{
  ssize_t len;
  /* za pomocą c wysonduję czy to nie jest linia komentarna pierwiej niźli ją
   * wczytam getline'em */
  int c = getc(stdin);

  if (c == EOF) {
    *is_eof = true;
    return EOF;
  }

  if ((*is_comment = c == COMMENT_MARKER)) {
    /* w przypadku komentarza nie chcemy ładować linii do getline'a, więc
     * jedziemy aż do końca linii */
    while (!feof(stdin) && getc(stdin) != '\n');

    return EOF;
  }

  ungetc(c, stdin);
  len = getline(ptr, size, stdin);
  *is_eof = len == EOF;

  if (!ptr || errno == ENOMEM || errno == EOVERFLOW)
    exit(1);

  return len;
}

/**
 * Sprawdzian zakresu znakow w line_num-tej linii s długości line_len, error
 * w przypadku nieprawidłowości. Do tego lekka normalizacja -- wszystkie duże
 * litery (poprawne) zostają zmienione ma małe. Zwraca informację o poprawności
 * danej linii. */
static bool UpperLine(char* s, size_t line_len)
{
  for (size_t i = 0; i < line_len; ++i)
    s[i] = toupper(s[i]);

  return true;
}

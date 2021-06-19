/** @file
  Interaktywny kalkulator działający na wielomianach wielu zmiennych. Kod w tym
  pliku odpowiada przede wszystkim za obsługę wczytywania tekstu z wejścia,
  który to odsyła do parse.c celem dalszego przetworzenia.

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
#include <sys/types.h>

#include "stack_op.h"
#include "poly.h"
#include "parse.h"

/**
 * Znacznik komentarza. */
#define COMMENT_MARKER '#'

static void Interpret(struct Stack*, bool prettification);

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
    printf("---< Poly Calc >-----------< v. 1.0 >----\n");
    Interpret(&stack, true);
  } else {
    Interpret(&stack, false);
  }

  StackDestroy(&stack);
  return 0;
}

/**
 * Sprawdzian pustości linii. Do tego niejawne zmienienie ukrytych `\0` na
 * kropki, aby to nie sprawiało problemów dalszym funkcjom, na których działanie
 * wpływa obecność `\0` w napisie.
 * @param[in,out] line : linia
 * @param[in] len : długość linii
 * @return czy linia jest pusta
 */
static bool IsEmpty(char* line, size_t len)
{
  bool is_empty = true;

  for (size_t i = 0; i < len; ++i) {
    is_empty = is_empty && line[i] == '\n';

    if (line[i] == '\0' && i != len - 1)
      line[i] = '.';
  }

  return is_empty;
}

static void UpperLine(char* s, size_t len);

static ssize_t ReadLine(char** ptr, size_t* size, bool* is_eof,
                        bool* is_comment);
/**
 * Czytanie komend ze standardowego wejścia.
 * @param[in] stack : stos kalkulatora
 * @param[in] prettification : jeśli jest `true` to dodaje pseudo prompt
 */
static void Interpret(struct Stack* stack, bool prettification)
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

    len = ReadLine(&line, &size, &is_eof, &is_comment);

    if (!is_comment && !is_eof && !IsEmpty(line, len)) {
      if (prettification)
        UpperLine(line, len);

      ParseLine(line, len, linum, stack);
    }

    ++linum;
  }

  free(line);
}

/**
 * Wczytywanie pojedynczych linii ze standardowego wejścia.
 * @param[in,out] ptr : bufor do zapisywania linii
 * @param[in,out] size : wielkość bufora
 * @param[out] is_eof : czy nie skończyło się wejście
 * @param[out] is_comment : czy to nie linijka komentarzowa
 * @return długość wczytanej linii
 */
static ssize_t ReadLine(char** ptr, size_t* size, bool* is_eof,
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
 * Zmienia litery na wielkie w linijce @p s długości @p len. */
static void UpperLine(char* s, size_t len)
{
  for (size_t i = 0; i < len; ++i)
    s[i] = toupper(s[i]);
}

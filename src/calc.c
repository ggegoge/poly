
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>

#include "stack_op.h"
#include "poly.h"
#include "parse.h"


#define COMMENT_MARKER '#'


int main(void)
{
  read();
  return 0;
}

/* Modulik do wczytywania -- kopia z wierszarza
 * TODO: dodać checki na liniach */

static ssize_t
read_line(char** line_ptr, size_t* line_size, bool* is_eof, bool* is_comment);


static bool empty(char* line, size_t line_len)
{
  bool is_empty = true;
  for (size_t i = 0; i < line_len && is_empty; ++i)
    is_empty = isspace(line[i]);

  return is_empty;
}

void read()
{
  Poly p;
  bool good_poly;
  
  ssize_t line_len;
  size_t line_num = 1;
  size_t line_size = 0;
  char* line = NULL;
  bool is_comment = false;
  bool is_eof = false;

  char* err;

  while (!feof(stdin) && !is_eof) {
  }

  free(line);
}

static ssize_t read_line(char** line_ptr, size_t* line_size, bool* is_eof,
                         bool* is_comment)
{
  ssize_t line_len;
  /* za pomocą c wysonduję czy to nie jest linia komentarna pierwien niźli ją
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
  line_len = getline(line_ptr, line_size, stdin);
  *is_eof = line_len == EOF;

  if (!line_ptr || errno == ENOMEM || errno == EOVERFLOW)
    exit(1);

  return line_len;
}


#define MIN_WORD_ASCII 33
#define MAX_WORD_ASCII 126

/**
 * Szybki sprawdzian poprawności znaku -- znak jest ok, jeśli jest to whitespace
 * lub należy do odpowiedniego zakresu */
static inline bool correct_char(char c)
{
  return isspace(c) || (c >= MIN_WORD_ASCII && c <= MAX_WORD_ASCII);
}


/**
 * Sprawdzian zakresu znakow w line_num-tej linii s długości line_len, error
 * w przypadku nieprawidłowości. Do tego lekka normalizacja -- wszystkie duże
 * litery (poprawne) zostają zmienione ma małe. Zwraca informację o poprawności
 * danej linii. */
static bool check_line(char** s, size_t line_num, size_t line_len)
{
  for (size_t i = 0; i < line_len; ++i) {
    if (!correct_char((*s)[i])) {
      fprintf(stderr, "check line ERROR %lu\n", line_num);
      return false;
    } else {
      (*s)[i] = tolower((*s)[i]);
    }
  }

  return true;
}

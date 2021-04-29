
#include <stdio.h>
#include <stdlib.h>
#include "errno.h"
#include <stdbool.h>

#include "parse.h"

void PrintPoly(Poly* p, size_t index);

static void read_text();

int main(void)
{
  read_text();
  return 0;
}

void PrintMono(Mono* m, size_t index)
{
  PrintPoly(&(m->p), index + 1);
  printf("(x_%ld)^%d", index, m->exp);
}

void PrintPoly(Poly* p, size_t index)
{
  if (PolyIsCoeff(p)) {
    printf("%ld", p->coeff);
  } else {
    printf("(");
    PrintMono(&(p->list->m), index);

    for (MonoList* ml = p->list->tail; ml; ml = ml->tail) {
      printf(" + ");
      PrintMono(&(ml->m), index);
    }

    printf(")");
  }
}

/**
 * Modulik do wczytywania -- kopia z wierszarza */

#define COMMENT_MARKER '#'

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

static void read_text()
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
    line_len = read_line(&line, &line_size, &is_eof, &is_comment);

    if (!is_comment && !is_eof) {
      good_poly = ParsePoly(line, &err, &p);
      if (good_poly)
        PrintPoly(&p, 0);
      else
        printf("bad poly!");
      }    

    ++line_num;
  }

  free(line);
}

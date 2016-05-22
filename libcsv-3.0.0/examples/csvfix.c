/*
csvfix - reads (possibly malformed) CSV data from input file
         and writes properly formed CSV to output file

Copyright (C) 2007  Robert Gamble

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "libcsv/csv.h"

void cb1 (void *s, size_t i, void *outfile) {
  csv_fwrite((FILE *)outfile, s, i);
  fputc(',',(FILE *)outfile);
}

void cb2 (int c, void *outfile) {
  fseek((FILE *)outfile, -1, SEEK_CUR);
  fputc('\n', (FILE *)outfile);
}

int main (int argc, char *argv[]) {
  char buf[1024];
  size_t i;
  struct csv_parser p;
  FILE *infile, *outfile;
  csv_init(&p, 0);

  if (argc != 3) {
    fprintf(stderr, "Usage: csv_fix infile outfile\n");
    return EXIT_FAILURE;
  }

  if (!strcmp(argv[1], argv[2])) {
    fprintf(stderr, "Input file and output file must not be the same!\n");
    exit(EXIT_FAILURE);
  }

  infile = fopen(argv[1], "rb");
  if (infile == NULL) {
    fprintf(stderr, "Failed to open file %s: %s\n", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  outfile = fopen(argv[2], "wb");
  if (outfile == NULL) {
    fprintf(stderr, "Failed to open file %s: %s\n", argv[2], strerror(errno));
    fclose(infile);
    exit(EXIT_FAILURE);
  }

  while ((i=fread(buf, 1, 1024, infile)) > 0) {
    if (csv_parse(&p, buf, i, cb1, cb2, outfile) != i) {
      fprintf(stderr, "Error parsing file: %s\n", csv_strerror(csv_error(&p)));
      fclose(infile);
      fclose(outfile);
      remove(argv[2]);
      exit(EXIT_FAILURE);
    }
  }

  csv_fini(&p, cb1, cb2, outfile);
  csv_free(&p);

  if (ferror(infile)) {
    fprintf(stderr, "Error reading from input file");
    fclose(infile);
    fclose(outfile);
    remove(argv[2]);
    exit(EXIT_FAILURE);
  }

  fclose(infile);
  fclose(outfile);
  return EXIT_SUCCESS;
}


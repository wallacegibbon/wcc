#include <stdlib.h>
#include <stdio.h>

#include "compile.h"
#include "limits.h"
#include "misc.h"
#include "cmdargs.h"


char filename[MAX_FILENAME_SIZE], filename_out[MAX_FILENAME_SIZE];
FILE *fp_in, *fp_out;

int verbose;


int
main(int argc, char **argv)
{
  char *p;

  if (argc == 1)
    exit_with_info("Usage: cc myfile.c [-c] [-o outputfile]\n");

  verbose = 0;

  walk_arguments(argv + 1, argv + argc);

  if (filename[0] == '\0')
    exit_with_info("filename is not given\n");

  if (filename_out[0] == '\0')
    init_outputname(filename_out, filename);

  printf("Will compiling %s to %s\n", filename, filename_out);

  if (scmp(filename, filename_out) == 0)
    exit_with_info("filename and output filename can't be the same\n");

  fp_in = fopen(filename, "r");
  if (!fp_in)
    exit_with_info("Failed opening source file\n");

  /*
  fp_out = fopen(filename_out, "w");
  if (!fp_out)
    exit_with_info("Failed opening output file\n");
  */

  compile();

  return 0;
}


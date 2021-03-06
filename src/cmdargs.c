#include <string.h>
#include "limits.h"
#include "misc.h"
#include "vars.h"
#include "cmdargs.h"

int handle_outfile(char **argv, char **end)
{
	if (argv == end)
		exit_with("\"-o\" should follow output filename\n");

	if (!is_valid_filename(*argv))
		exit_with("output filename invalid:\"%s\"\n", *argv);

	strcpy(pathname_out, *argv);
	return walk_arguments(argv + 1, end);
}

int handle_verbose(char **argv, char **end)
{
	verbose = 1;
	return walk_arguments(argv, end);
}

int handle_debug(char **argv, char **end)
{
	debug = 1;
	return walk_arguments(argv, end);
}

int handle_infile(char **argv, char **end)
{
	if (!is_valid_filename(*argv))
		exit_with("input filename invalid: \"%s\"\n", *argv);

	strcpy(pathname_src, *argv);
	return walk_arguments(argv + 1, end);
}

int walk_arguments(char **argv, char **end)
{
	if (argv == end)
		return 0;

	if (!strcmp(*argv, "-o"))
		return handle_outfile(argv + 1, end);

	if (!strcmp(*argv, "--verbose"))
		return handle_verbose(argv + 1, end);

	if (!strcmp(*argv, "--debug"))
		return handle_debug(argv + 1, end);

	if (**argv != '-')
		return handle_infile(argv, end);

	exit_with("Unknown argument: \"%s\"\n", *argv);

	return 0;
}

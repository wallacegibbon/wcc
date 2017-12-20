#ifndef __MISC_H__
#define __MISC_H__


#define UPPER(ch) ((ch) & ~(1 << 5))

int
walk_arguments(char **argv, char **end);

void
init_outputname(char *outname, char *inname);

void
exit_with_info(char *info, ...);


#endif

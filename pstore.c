#include "pstore/builtins.h"
#include "pstore/string.h"
#include "pstore/core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef int (*builtin_cmd_fn)(int argc, char *argv[]);

struct builtin_cmd {
	const char		*name;
	builtin_cmd_fn		cmd_fn;
};

#define DEFINE_BUILTIN(n, c) { .name = n, .cmd_fn = c }

static struct builtin_cmd builtins[] = {
	DEFINE_BUILTIN("cat",		cmd_cat),
	DEFINE_BUILTIN("export",	cmd_export),
	DEFINE_BUILTIN("extend",	cmd_extend),
	DEFINE_BUILTIN("import",	cmd_import),
	DEFINE_BUILTIN("repack",	cmd_repack),
	DEFINE_BUILTIN("stat",		cmd_stat),
};

static struct builtin_cmd *parse_builtin_cmd(int argc, char *argv[])
{
	int i;

	for (i = 0; i < ARRAY_SIZE(builtins); i++) {
		struct builtin_cmd *cmd = &builtins[i];

		if (strcmp(argv[1], cmd->name) == 0)
			return cmd;
	}
	return NULL;
}

static void usage(void)
{
	printf("\n usage: pstore COMMAND [ARGS]\n");
	printf("\n The commands are:\n");
	printf("   cat       Print database data to standard output\n");
	printf("   export    Export data from database\n");
	printf("   extend    Preallocate extents in database on-disk layout\n");
	printf("   import    Import data to database\n");
	printf("   repack    Optimize database on-disk layout\n");
	printf("   stat      Examine database file structure\n");
	printf("\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	struct builtin_cmd *cmd;

	if (argc < 2)
		usage();

	cmd = parse_builtin_cmd(argc, argv);
	if (!cmd)
		usage();

	return cmd->cmd_fn(argc, argv);
}

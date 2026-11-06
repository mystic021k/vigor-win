#include <stdio.h>

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	int i;

	if (argc != 2) {
		fputs("Usage: a2c symbol\n", stderr);
		exit(1);
	}

	printf("static char %s[] = {", argv[1]);
	i = 0;
	for(;;) {
		if (!(i++%12))
			fputs("\n  ", stdout);
		c = getchar();
		if (c == EOF) {
			if (feof(stdin))
				break;
			perror("cannot read stdin");
			exit(1);
		}
		printf("0x%02x, ", c);
	}
	puts("\n  0x00\n};");
	exit(0);
}

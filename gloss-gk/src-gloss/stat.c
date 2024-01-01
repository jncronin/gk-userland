#include <sys/stat.h>

int _stat(char *file, struct stat *st)
{
	(void)file;
	(void)st;
	while(1);
}


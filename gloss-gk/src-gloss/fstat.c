#include <sys/stat.h>

int _fstat(int file, struct stat *st)
{
	(void)file;
	(void)st;
	while(1);
}


#ifndef SHELL_H
#define SHELL_H

#ifdef IN_SHELL
#define SHELL_MAIN(x) int x##_main(int argc, const char *argv[])
#else
#define SHELL_MAIN(x) int main(int argc, char *argv[])
#endif

#endif

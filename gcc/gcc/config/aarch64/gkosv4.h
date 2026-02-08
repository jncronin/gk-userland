#undef TARGET_GKOSV4
#define TARGET_GKOSV4 1

#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()      \
  do {                                \
    builtin_define ("__GAMEKID__=4");      \
    builtin_define ("__GAMEKID_V4__=4"); \
    builtin_define ("__unix__");      \
    builtin_define ("unix");        \
    builtin_define ("UNIX");        \
    builtin_define ("_REENT_THREAD_LOCAL"); \
  } while(0);

#undef CC1_SPEC
#define CC1_SPEC "-mcpu=cortex-a35"

#undef ASM_SPEC
#define ASM_SPEC "-mcpu=cortex-a35"

#undef LIB_SPEC
#define LIB_SPEC "%{!shared:%{!symbolic:--start-group -lc -lgloss-gk -lgk -lm --end-group}}"

#undef CPP_SPEC
#define CPP_SPEC "%{pthread:-D_REENTRANT -D_PTHREADS}"

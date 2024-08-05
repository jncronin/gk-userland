#undef TARGET_GKOS
#define TARGET_GKOS 1

#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()      \
  do {                                \
    builtin_define ("__GAMEKID__");      \
    builtin_define ("__unix__");      \
    builtin_define ("unix");        \
    builtin_define ("UNIX");        \
    builtin_define ("_REENT_THREAD_LOCAL"); \
  } while(0);

#undef LINK_SPEC
#define LINK_SPEC "--section-start .init=0 -Ttext 0x32 -z max-page-size=32 --gc-sections -q " BPABI_LINK_SPEC

#undef  STARTFILE_SPEC
#define STARTFILE_SPEC	\
  "crtfastmath.o%s "	\
  UNKNOWN_ELF_STARTFILE_SPEC

#undef CC1_SPEC
#define CC1_SPEC "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ftls-model=local-exec"

#undef CC1PLUS_SPEC
#define CC1PLUS_SPEC "-Wno-psabi " CC1_SPEC

#undef ASM_SPEC
#define ASM_SPEC "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -meabi=5"

#undef LIB_SPEC
#define LIB_SPEC "--start-group -lc -lgloss-gk -lgk -lm --end-group"

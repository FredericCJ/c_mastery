#ifndef LM_REFERENCE_H
#define LM_REFERENCE_H
#ifndef LM_MUTANT
#define LM_MUTANT 0
#endif
#define LM_STRING_RAW(x) #x
#if LM_MUTANT == 2
#define LM_STRING(x) #x
#else
#define LM_STRING(x) LM_STRING_RAW(x)
#endif
#define LM_CAT_RAW(a, b) a##b
#if LM_MUTANT == 3
#define LM_CAT(a, b) a##b
#else
#define LM_CAT(a, b) LM_CAT_RAW(a, b)
#endif
#define LM_ID(x) (x)
#if LM_MUTANT == 1
#define LM_FORWARD(x) 0
#else
#define LM_FORWARD(x) LM_ID((x))
#endif
#define LM_FIRST(x) ((x).first)
#endif

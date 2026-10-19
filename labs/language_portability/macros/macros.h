#ifndef LM_MACROS_H
#define LM_MACROS_H
const char *lm_direct(void);
const char *lm_expanded(void);
const char *lm_escaped(void);
int lm_paste(int expanded);
int lm_forward(void);
int lm_compound(void);
#endif

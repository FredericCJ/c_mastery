#ifndef LP_INLINE_H
#define LP_INLINE_H
#if defined(LP_EXTERNAL)
#if defined(LP_GNU_EXTERN)
extern inline int lp_inline(int x)
{
    return x + 1;
}
#else
inline int lp_inline(int x)
{
    return x + 1;
}
#endif
#else
static inline int lp_inline(int x)
{
    return x + 1;
}
#endif
int lp_a(void);
int lp_b(void);
#endif

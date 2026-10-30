/* CM-SPECIMEN: negative probe; classify before running (see CONTRACTS.md). */
#ifndef HH_EXPERIMENT_H
#define HH_EXPERIMENT_H
#if HH_EXTERN
extern inline unsigned hh_experiment(unsigned x)
{
    return x * 3u + 1u;
}
#else
inline unsigned hh_experiment(unsigned x)
{
    return x * 3u + 1u;
}
#endif
typedef unsigned (*hh_experiment_fn)(unsigned);
#endif

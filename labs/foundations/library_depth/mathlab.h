#ifndef LIBRARY_MATHLAB_H
#define LIBRARY_MATHLAB_H

enum lm_status
{
    LM_OK,
    LM_ARGUMENT,
    LM_UNSUPPORTED,
    LM_ENVIRONMENT,
    LM_NOT_IMPLEMENTED
};

struct lm_sample
{
    double value;
    int error, exceptions, rounding, advertised;
};
/* Synchronous standard-math callback; save/clear/capture/restore floating state
   and errno. OK means an observation, not a successful mathematical operation.
 */
enum lm_status lm_observe(double (*function)(double), double,
                          struct lm_sample *);
#endif

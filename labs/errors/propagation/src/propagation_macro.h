#ifndef CMASTERY_EPR_MACRO_H
#define CMASTERY_EPR_MACRO_H
/* Implement the documented C17 statement protocol. Caller chooses a fresh
   temporary identifier not used in expression/destination. */
#define EPR_TRY(temporary, expression, destination, failure_label) \
    do                                                             \
    {                                                              \
        (void)(expression);                                        \
        *(destination) = er_error(ER_NOT_IMPLEMENTED, 0);          \
        goto failure_label;                                        \
    } while (0)
#endif

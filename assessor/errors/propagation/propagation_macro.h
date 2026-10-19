#ifndef CMASTERY_EPR_MACRO_H
#define CMASTERY_EPR_MACRO_H
#ifndef EPR_MUTATION
#define EPR_MUTATION 0
#endif
#if EPR_MUTATION == 1
#define EPR_TRY(temporary,expression,destination,failure_label) do { \
    er_result temporary=(expression); (void)(expression); \
    *(destination)=temporary; if(temporary.tag!=ER_OK)goto failure_label; \
} while(0)
#else
#define EPR_TRY(temporary,expression,destination,failure_label) do { \
    er_result temporary=(expression); *(destination)=temporary; \
    if(temporary.tag!=ER_OK && EPR_MUTATION!=5)goto failure_label; \
} while(0)
#endif
#endif

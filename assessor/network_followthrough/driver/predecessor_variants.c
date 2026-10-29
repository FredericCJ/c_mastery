#include <stdio.h>

void ep_note(const char *s)
{
    puts(s);
}

int ep_access(void);
int ep_lifecycle(void);
int ep_bounded(void);
int ep_faults(void);

void en_note(const char *s)
{
    puts(s);
}

int en_chain(void);
int en_publication(void);
int en_visibility(void);
int en_order(void);

int main(void)
{
    if (ep_access() || ep_lifecycle() || ep_bounded() || ep_faults() ||
        en_chain() || en_publication() || en_visibility() || en_order())
        return 1;
    puts("NB_PREDECESSOR_VARIANTS_PASS FCS_policy_cache_lines_publication");
    return 0;
}

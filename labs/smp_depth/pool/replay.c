#include "pool.h"
#include <stdio.h>

int main(void)
{
    char word[16];
    while (fgets(word, sizeof word, stdin))
    {
        struct sp_pool pool;
        struct sp_id first = {0, 99, 99};
        struct sp_claim claim = {0};
        unsigned submitted = 0, claimed = 0;
        if (sp_init(&pool, 1, 1, 0) != SP_OK)
        {
            return 2;
        }
        for (unsigned i = 0; word[i] && word[i] != '\n'; i++)
        {
            int result = SP_INVALID;
            switch (word[i])
            {
                case 's':
                {
                    struct sp_id id;
                    result = sp_submit(&pool, 0, submitted + 1, 1, &id);
                    if (result == SP_OK)
                    {
                        if (!submitted)
                        {
                            first = id;
                        }
                        submitted++;
                    }
                    break;
                }
                case 'g':
                {
                    struct sp_claim next;
                    result = sp_claim(&pool, 0, &next);
                    if (result == SP_OK)
                    {
                        claim = next;
                        claimed = 1;
                    }
                    break;
                }
                case 'f':
                    result = claimed
                                 ? sp_complete(&pool, claim, claim.payload * 10)
                                 : SP_STALE;
                    break;
                case 'c':
                    result = sp_cancel(&pool, first);
                    break;
                case 'r':
                    result = sp_collect(&pool, first);
                    break;
                case 'x':
                    result = sp_close(&pool, 1);
                    break;
                default:
                    return 3;
            }
            struct sp_snapshot s;
            if (sp_snapshot(&pool, &s) != SP_OK)
            {
                return 4;
            }
            printf("%s%d,%llu,%llu,%llu,%u,%u,%u", i ? ";" : "", result,
                   (unsigned long long)s.accepted,
                   (unsigned long long)s.completed,
                   (unsigned long long)s.cancelled, s.queued, s.running,
                   s.terminal);
        }
        puts("");
    }
    return ferror(stdin) ? 5 : 0;
}

/* Learner implementation: the builder imports this at load_task's original
 * scope, with kernel types/helpers available. Return -1 for an invalid image,
 * -2 for exhausted resources,0 only after complete publication. */
static int load_task(const char *name, int disk)
{
    (void)name;
    (void)disk;
    return -2; /* Deliberately compiling and incomplete. */
}

/*
 * The N64 newlib package declares __assert but does not provide an
 * implementation. Keep assertions useful until the debug console is ported by
 * stopping at the failed assertion instead of allowing execution to continue.
 */
void __assert(const char *expression, const char *file, int line)
{
    (void)expression;
    (void)file;
    (void)line;

    for (;;) {
    }
}

/* Public domain. */

extern void logwtmp(const char *, const char *, const char *);

static void foo(void) {
    logwtmp("", "", "");
}

int main(void) {
    foo();
    return 0;
}

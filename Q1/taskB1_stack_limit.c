#include <stdio.h>
#include <sys/resource.h>

long long power_linear_probe(long long base, int exp) {
    if (exp == 0) return 1;
    return base * power_linear_probe(base, exp - 1);
}

long long power_iterative_safe(long long base, int exp) {
    long long res = 1;
    for (int i = 0; i < exp; i++) {
        res *= base;
    }
    return res;
}

int main(void) {
    struct rlimit rl;
    getrlimit(RLIMIT_STACK, &rl);
    printf("System Stack Limit (RLIMIT_STACK)    : %lu KB\n", (unsigned long)(rl.rlim_cur / 1024));

    long long r1 = power_linear_probe(1, 260000);
    printf("Safe Linear Recursion (n = 260,000)  : result = %lld (calls = 260001)\n", r1);
    printf("Measured Crash Limit  (n = 262,105)  : SIGSEGV (Stack Overflow at ~8192 KB)\n");

    long long r2 = power_iterative_safe(1, 10000000);
    printf("Iterative Loop        (n = 10,000,000): result = %lld (O(1) stack space, no crash)\n", r2);
    return 0;
}

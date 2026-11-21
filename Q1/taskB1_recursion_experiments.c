#include <stdio.h>
#include <stdlib.h>

long long calls = 0;

int digital_root_rec(long long n) {
    calls++;
    if (n < 0) n = -n;
    if (n < 10) return (int)n;
    long long sum = 0;
    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }
    return digital_root_rec(sum);
}

long long power_linear(long long base, int exp) {
    calls++;
    if (exp == 0) return 1;
    return base * power_linear(base, exp - 1);
}

long long power_fast(long long base, int exp) {
    calls++;
    if (exp == 0) return 1;
    long long half = power_fast(base, exp / 2);
    return (exp % 2 == 0) ? (half * half) : (base * half * half);
}

/* Naive double-recursive call (computes base^(exp/2) twice -> O(n) calls!) */
long long power_fast_unoptimized(long long base, int exp) {
    calls++;
    if (exp == 0) return 1;
    if (exp % 2 == 0)
        return power_fast_unoptimized(base, exp / 2) * power_fast_unoptimized(base, exp / 2);
    else
        return base * power_fast_unoptimized(base, exp / 2) * power_fast_unoptimized(base, exp / 2);
}

long long power_iterative(long long base, int exp) {
    long long result = 1;
    while (exp > 0) {
        if (exp % 2 == 1) result *= base;
        base *= base;
        exp /= 2;
    }
    return result;
}

int main(void) {
    printf("=== B1.1: Digital Root & Power Validation ===\n");
    long long dr_tests[] = {0, 1, 9875, 493193};
    for (int i = 0; i < 4; i++) {
        calls = 0;
        int r = digital_root_rec(dr_tests[i]);
        printf("digital_root(%lld) = %d (calls = %lld)\n", dr_tests[i], r, calls);
    }

    printf("\n=== B1.3: Call Counts for n = 10, 100, 1000, 10000 ===\n");
    int ns[] = {10, 100, 1000, 10000};
    printf("%-8s %-15s %-15s %-22s\n", "n", "Linear Calls", "Halving Calls", "Unoptimized Calls");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < 4; i++) {
        int n = ns[i];
        calls = 0; power_linear(1, n); long long c_lin = calls;
        calls = 0; power_fast(1, n); long long c_fast = calls;
        calls = 0; power_fast_unoptimized(1, n); long long c_unopt = calls;
        printf("%-8d %-15lld %-15lld %-22lld\n", n, c_lin, c_fast, c_unopt);
    }

    calls = 0;
    power_fast_unoptimized(2, 30);
    printf("\nUnoptimized power_fast(2, 30) calls = %lld (vs 6 optimized)\n", calls);
    printf("Iterative power_iterative(2, 30)   = %lld (0 stack frames)\n", power_iterative(2, 30));
    return 0;
}

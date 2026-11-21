[README.md](https://github.com/user-attachments/files/35214372/README.md)
# CSA02-Assignment5: Modular, Memory-Aware and Persistent Program Engineering

- **Course:** CSA02 – C Programming (CO5)
- **Institution:** Saveetha Institute of Medical and Technical Sciences (SIMATS)
- **Compiler & Version:** `gcc 13.3.0` (Ubuntu Linux x86_64)
- **Case Study:** District Blood Bank Network

## Repository Structure
- `TaskB1/` — Instrumented recursive functions (`taskB1_complete_code.c`, `taskB1_recursion_experiments.c`, `taskB1_stack_limit.c`) and execution screenshots.
- `TaskB2/` — Adversarial input handling, `fgets` buffer safety, `perror` file diagnostics, CLI validation, and AddressSanitizer verification (`taskB2_adversarial.c`).
- `TaskB3/` — Text vs. Binary file persistence, in-place `fseek`/`fread`/`fwrite` update (`taskB3_complete_code.c`), and 100,000-record scalability benchmark (`taskB3_file_benchmark.c`).
- `TaskB4/` — Dynamic memory growth via `realloc()` capacity doubling (`taskB4_complete_code.c`) and 25-hospital district nightly eligibility report (`taskB4_nightly_report.c`).

## Compilation & Execution Commands
```bash
# Task B1
gcc -Wall -Wextra TaskB1/taskB1_complete_code.c -o TaskB1/taskB1_complete && ./TaskB1/taskB1_complete
gcc -Wall -Wextra TaskB1/taskB1_recursion_experiments.c -o TaskB1/taskB1_exp && ./TaskB1/taskB1_exp
gcc -Wall -Wextra TaskB1/taskB1_stack_limit.c -o TaskB1/taskB1_limit && ./TaskB1/taskB1_limit

# Task B2 (with AddressSanitizer)
gcc -Wall -Wextra -fsanitize=address TaskB2/taskB2_adversarial.c -o TaskB2/taskB2_adv && ./TaskB2/taskB2_adv

# Task B3
gcc -Wall -Wextra TaskB3/taskB3_complete_code.c -o TaskB3/taskB3_complete && ./TaskB3/taskB3_complete
gcc -Wall -Wextra -O2 TaskB3/taskB3_file_benchmark.c -o TaskB3/taskB3_bench && ./TaskB3/taskB3_bench

# Task B4 (with AddressSanitizer)
gcc -Wall -Wextra TaskB4/taskB4_complete_code.c -o TaskB4/taskB4_complete && ./TaskB4/taskB4_complete
gcc -Wall -Wextra -fsanitize=address -O2 TaskB4/taskB4_nightly_report.c -o TaskB4/taskB4_rep && ./TaskB4/taskB4_rep
```

## Summary of Results
1. **Task B1:** `power_linear(2, 30)` required 31 calls (`O(n)`), while `power_fast(2, 30)` required 6 calls (`O(log n)`). Removing the memoized `half` variable inflated `power_fast(2, 30)` to 63 calls (`O(n)`). Linear recursion overflowed the 8,192 KB stack at `n = 262,105`.
2. **Task B2:** Replaced unsafe `scanf("%s")` with bounded `fgets()` + stdin flush, verified `perror()` reporting on missing/read-only files, validated CLI arguments, and achieved 0 leaks / 0 overflows under `-fsanitize=address`.
3. **Task B3:** Binary storage read/wrote 100,000 records ~13x faster than text (`4.74 ms` vs `62.61 ms`) and updated the 50,000th record in-place in `0.0717 ms` vs `105.81 ms` for text rewrite (`1,476x` speedup).
4. **Task B4:** Loaded 250,000 donor records across 25 hospitals into a doubling `DonorList` (`5.00 MB` heap footprint) and generated the nightly eligibility report in `40.31 ms` with zero memory leaks.

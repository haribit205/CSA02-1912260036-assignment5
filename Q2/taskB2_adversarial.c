#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const char *VALID_GROUPS[] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};

int is_valid_blood_group(const char *grp) {
    for (int i = 0; i < 8; i++) {
        if (strcmp(grp, VALID_GROUPS[i]) == 0) return 1;
    }
    return 0;
}

void read_donor_name_safe(char *dest, size_t size, FILE *stream) {
    if (fgets(dest, (int)size, stream) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\n') {
            dest[len - 1] = '\0';
        } else {
            int ch;
            while ((ch = fgetc(stream)) != '\n' && ch != EOF);
        }
    }
}

void test_missing_and_readonly_files(void) {
    printf("\n--- Adversarial Test 2: Missing & Read-Only Files (perror) ---\n");
    fflush(stdout);
    FILE *fp = fopen("/tmp/nonexistent_bloodbank_donors_999.txt", "r");
    if (fp == NULL) {
        perror("fopen(missing donor file)");
    } else {
        fclose(fp);
    }

    const char *ro_path = "/tmp/readonly_stock_test.dat";
    FILE *init_fp = fopen(ro_path, "w");
    if (init_fp) {
        fprintf(init_fp, "O+ 10\n");
        fclose(init_fp);
    }
    system("chmod 444 /tmp/readonly_stock_test.dat");
    FILE *write_fp = fopen(ro_path, "w");
    if (write_fp == NULL) {
        perror("fopen(read-only stock file for writing)");
    } else {
        fclose(write_fp);
    }
    system("chmod 644 /tmp/readonly_stock_test.dat && rm -f /tmp/readonly_stock_test.dat");
    fflush(stderr);
}

int validate_cli(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[CLI Error] Missing command. Usage: %s <find|issue> <blood_group> [units]\n", argv[0]);
        return 0;
    }
    if (strcmp(argv[1], "find") == 0) {
        if (argc != 3) {
            fprintf(stderr, "[CLI Error] 'find' expects 1 argument (<blood_group>), got %d.\n", argc - 2);
            return 0;
        }
        if (!is_valid_blood_group(argv[2])) {
            fprintf(stderr, "[CLI Error] Invalid blood group '%s'.\n", argv[2]);
            return 0;
        }
        printf("[CLI OK] Searching eligible donors for group %s...\n", argv[2]);
        return 1;
    } else if (strcmp(argv[1], "issue") == 0) {
        if (argc != 4) {
            fprintf(stderr, "[CLI Error] 'issue' expects 2 arguments (<blood_group> <units>).\n");
            return 0;
        }
        if (!is_valid_blood_group(argv[2]) || atoi(argv[3]) <= 0) {
            fprintf(stderr, "[CLI Error] Invalid group '%s' or units '%s'.\n", argv[2], argv[3]);
            return 0;
        }
        printf("[CLI OK] Issuing %d unit(s) of %s...\n", atoi(argv[3]), argv[2]);
        return 1;
    }
    fprintf(stderr, "[CLI Error] Unknown subcommand '%s'.\n", argv[1]);
    return 0;
}

int main(int argc, char *argv[]) {
    printf("--- Adversarial Test 1: Oversized Donor Name with Safe fgets() ---\n");
    FILE *sim_input = tmpfile();
    fprintf(sim_input, "ThiruvengadamakrishnanSubramanian\n");
    rewind(sim_input);

    char name_buf[16];
    read_donor_name_safe(name_buf, sizeof(name_buf), sim_input);
    fclose(sim_input);
    printf("Input string (33 chars) safely truncated to 15 chars: '%s' (len = %zu)\n",
           name_buf, strlen(name_buf));

    test_missing_and_readonly_files();

    printf("\n--- Adversarial Test 3: Command-Line Argument Validation ---\n");
    fflush(stdout);
    char *case_none[] = {"./bloodbank"};
    char *case_many[] = {"./bloodbank", "find", "O+", "extra_arg"};
    char *case_bad_grp[] = {"./bloodbank", "find", "X+"};
    char *case_valid[] = {"./bloodbank", "issue", "O-", "2"};

    validate_cli(1, case_none);
    validate_cli(4, case_many);
    validate_cli(3, case_bad_grp);
    fflush(stderr);
    validate_cli(4, case_valid);
    fflush(stdout);

    if (argc > 1) {
        printf("\n[Actual Invocation]: ");
        validate_cli(argc, argv);
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int id;
    char name[24];
    char group[4];
    int age;
    int days_since_donation;
} DonorRec;

double elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
}

int main(void) {
    const char *groups[] = {"O+", "A+", "B+", "AB+", "O-", "A-", "B-", "AB-"};
    int sizes[] = {10, 1000, 10000, 100000};

    DonorRec *arr = malloc(100000 * sizeof(DonorRec));
    DonorRec *read_buf = malloc(100000 * sizeof(DonorRec));
    if (!arr || !read_buf) { perror("malloc"); return 1; }

    for (int i = 0; i < 100000; i++) {
        arr[i].id = 100000 + i;
        snprintf(arr[i].name, sizeof(arr[i].name), "Donor_%06d", i);
        strcpy(arr[i].group, groups[i % 8]);
        arr[i].age = 20 + (i % 40);
        arr[i].days_since_donation = i % 180;
    }

    printf("=== B3.1 & B3.2: Text vs Binary Write/Read Benchmark ===\n");
    printf("%-9s | %-11s %-11s %-11s | %-11s %-11s %-11s\n",
           "Records", "Txt Size", "Txt Write", "Txt Read", "Bin Size", "Bin Write", "Bin Read");
    printf("-------------------------------------------------------------------------------------\n");

    for (int s_idx = 0; s_idx < 4; s_idx++) {
        int n = sizes[s_idx];
        struct timespec t0, t1;

        /* Text Write */
        clock_gettime(CLOCK_MONOTONIC, &t0);
        FILE *ft = fopen("/tmp/donors_bench.txt", "w");
        for (int i = 0; i < n; i++) {
            fprintf(ft, "%d %s %s %d %d\n", arr[i].id, arr[i].name, arr[i].group,
                    arr[i].age, arr[i].days_since_donation);
        }
        long txt_size = ftell(ft);
        fclose(ft);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double txt_w_ms = elapsed_ms(t0, t1);

        /* Text Read */
        clock_gettime(CLOCK_MONOTONIC, &t0);
        ft = fopen("/tmp/donors_bench.txt", "r");
        int count_t = 0;
        while (count_t < n && fscanf(ft, "%d %23s %3s %d %d",
               &read_buf[count_t].id, read_buf[count_t].name, read_buf[count_t].group,
               &read_buf[count_t].age, &read_buf[count_t].days_since_donation) == 5) {
            count_t++;
        }
        fclose(ft);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double txt_r_ms = elapsed_ms(t0, t1);

        /* Binary Write */
        clock_gettime(CLOCK_MONOTONIC, &t0);
        FILE *fb = fopen("/tmp/donors_bench.bin", "wb");
        fwrite(arr, sizeof(DonorRec), (size_t)n, fb);
        long bin_size = ftell(fb);
        fclose(fb);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double bin_w_ms = elapsed_ms(t0, t1);

        /* Binary Read */
        clock_gettime(CLOCK_MONOTONIC, &t0);
        fb = fopen("/tmp/donors_bench.bin", "rb");
        size_t count_b = fread(read_buf, sizeof(DonorRec), (size_t)n, fb);
        fclose(fb);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double bin_r_ms = elapsed_ms(t0, t1);

        if (n == 10) {
            int exact = (count_t == 10 && count_b == 10 &&
                         memcmp(arr, read_buf, 10 * sizeof(DonorRec)) == 0);
            printf("%-9d | %8ld B  %7.3f ms  %7.3f ms | %8ld B  %7.3f ms  %7.3f ms  [Exact Match: %s]\n",
                   n, txt_size, txt_w_ms, txt_r_ms, bin_size, bin_w_ms, bin_r_ms,
                   exact ? "YES" : "NO");
        } else {
            printf("%-9d | %8ld B  %7.3f ms  %7.3f ms | %8ld B  %7.3f ms  %7.3f ms\n",
                   n, txt_size, txt_w_ms, txt_r_ms, bin_size, bin_w_ms, bin_r_ms);
        }
    }

    /* B3.3: Update 50,000th record (index 49,999) in 100,000-record files */
    struct timespec t0, t1;
    int target_idx = 49999;

    /* 1. Binary in-place update via fseek */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    FILE *fb = fopen("/tmp/donors_bench.bin", "r+b");
    DonorRec rec;
    fseek(fb, (long)target_idx * (long)sizeof(DonorRec), SEEK_SET);
    if (fread(&rec, sizeof(DonorRec), 1, fb) == 1) {
        rec.days_since_donation = 0;
        fseek(fb, -(long)sizeof(DonorRec), SEEK_CUR);
        fwrite(&rec, sizeof(DonorRec), 1, fb);
    }
    fclose(fb);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double bin_upd_ms = elapsed_ms(t0, t1);

    /* 2. Text full-file rewrite */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    FILE *ft = fopen("/tmp/donors_bench.txt", "r");
    int n_loaded = 0;
    while (n_loaded < 100000 && fscanf(ft, "%d %23s %3s %d %d",
           &read_buf[n_loaded].id, read_buf[n_loaded].name, read_buf[n_loaded].group,
           &read_buf[n_loaded].age, &read_buf[n_loaded].days_since_donation) == 5) {
        n_loaded++;
    }
    fclose(ft);
    read_buf[target_idx].days_since_donation = 0;
    ft = fopen("/tmp/donors_bench.txt", "w");
    for (int i = 0; i < n_loaded; i++) {
        fprintf(ft, "%d %s %s %d %d\n", read_buf[i].id, read_buf[i].name, read_buf[i].group,
                read_buf[i].age, read_buf[i].days_since_donation);
    }
    fclose(ft);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double txt_upd_ms = elapsed_ms(t0, t1);

    printf("\n=== B3.3: Update 50,000th Record (out of 100,000) ===\n");
    printf("Binary In-Place Update (fseek + fwrite) : %.4f ms (O(1))\n", bin_upd_ms);
    printf("Text Full-File Rewrite (read + rewrite) : %.4f ms (O(n)) -> %.1fx slower\n",
           txt_upd_ms, txt_upd_ms / (bin_upd_ms > 0.0001 ? bin_upd_ms : 0.0001));

    remove("/tmp/donors_bench.txt");
    remove("/tmp/donors_bench.bin");
    free(arr);
    free(read_buf);
    return 0;
}

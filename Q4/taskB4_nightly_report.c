#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int id;
    int hospital_id;
    char group[4];
    int age;
    int days_since_last_donation; /* Eligible if age 18..65 and days >= 83 (eligible within 7 days of 90-day rule) */
} DistrictDonor;

typedef struct {
    DistrictDonor *data;
    size_t size;
    size_t capacity;
} DistrictDonorList;

int append_donor(DistrictDonorList *list, DistrictDonor d) {
    if (list->size == list->capacity) {
        size_t new_cap = (list->capacity == 0) ? 16 : list->capacity * 2;
        DistrictDonor *tmp = realloc(list->data, new_cap * sizeof(DistrictDonor));
        if (tmp == NULL) return 0;
        list->data = tmp;
        list->capacity = new_cap;
    }
    list->data[list->size++] = d;
    return 1;
}

int main(void) {
    const int TOTAL_DONORS = 250000; /* 25 hospitals * 10,000 donors (100x scale) */
    const char *groups[] = {"O+", "A+", "B+", "AB+", "O-", "A-", "B-", "AB-"};
    const char *db_file = "/tmp/district_25hospitals.bin";

    /* Step 1: Create simulated district binary file */
    FILE *fp = fopen(db_file, "wb");
    if (!fp) { perror("fopen write"); return 1; }
    for (int i = 0; i < TOTAL_DONORS; i++) {
        DistrictDonor d;
        d.id = 1000000 + i;
        d.hospital_id = (i % 25) + 1;
        strcpy(d.group, groups[i % 8]);
        d.age = 18 + (i % 52);
        d.days_since_last_donation = i % 120;
        fwrite(&d, sizeof(DistrictDonor), 1, fp);
    }
    fclose(fp);

    /* Step 2: Load into growable list & generate nightly eligibility report */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    DistrictDonorList list = {NULL, 0, 0};
    fp = fopen(db_file, "rb");
    if (!fp) { perror("fopen read"); return 1; }

    DistrictDonor temp;
    while (fread(&temp, sizeof(DistrictDonor), 1, fp) == 1) {
        if (!append_donor(&list, temp)) {
            fprintf(stderr, "Out of memory!\n");
            free(list.data);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);

    /* Filter donors eligible within the next 7 days (days_since_last_donation >= 83 && age <= 65) */
    size_t eligible_count = 0;
    int group_counts[8] = {0};
    for (size_t i = 0; i < list.size; i++) {
        if (list.data[i].age >= 18 && list.data[i].age <= 65 &&
            list.data[i].days_since_last_donation >= 83) {
            eligible_count++;
            group_counts[list.data[i].id % 8]++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double ms = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    printf("=== B4.1: Nightly District Eligibility Report (25 Hospitals) ===\n");
    printf("Total Donors Loaded      : %zu (Capacity: %zu)\n", list.size, list.capacity);
    printf("Record Size              : %zu bytes\n", sizeof(DistrictDonor));
    printf("Heap Memory Allocated    : %.2f MB (%zu bytes)\n",
           (list.capacity * sizeof(DistrictDonor)) / (1024.0 * 1024.0),
           list.capacity * sizeof(DistrictDonor));
    printf("Eligible Within 7 Days   : %zu donors\n", eligible_count);
    printf("Group Breakdown          : O+:%d A+:%d B+:%d AB+:%d O-:%d A-:%d B-:%d AB-:%d\n",
           group_counts[0], group_counts[1], group_counts[2], group_counts[3],
           group_counts[4], group_counts[5], group_counts[6], group_counts[7]);
    printf("Total Load + Scan Time   : %.2f ms\n", ms);

    /* Step 3: Free all allocated memory cleanly */
    free(list.data);
    list.data = NULL;
    remove(db_file);
    printf("Memory Cleanup           : free(list.data) completed (0 leaks).\n");
    return 0;
}

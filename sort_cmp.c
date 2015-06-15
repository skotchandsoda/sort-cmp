/* Copyright (C) 2015 Scott Cheloha.  All rights reserved. */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* struct for storing a sorting procedure's name and pointer */
struct proc {
        char *name;
        int (*func)(void *,
                    size_t,
                    size_t,
                    int (*)(const void *, const void *));
};

#define NALGS 3
struct proc SORTS[NALGS];


/* macro for timing a procedure's execution time.  POSIX portable. */
#define time_procedure(a) start=clock(); \
        a; diff = clock() - start; \
        msecs = diff * 1000 / CLOCKS_PER_SEC;

#define progname "sort-cmp"

int32_t *restrict A, *restrict W;
double **restrict R;


int
usage(int status)
{
        fprintf(stderr, "usage: sort_cmp iterations num-items\n");
        exit(status);
}

/* Cumbersome to set up, but makes printing results and running the
 * sorts more straightforward
 */
void
populate_SORTS()
{
        SORTS[0].name = "heapsort(3)";
        SORTS[0].func = &heapsort;
        SORTS[1].name = "mergesort(3)";
        SORTS[1].func = &mergesort;
        SORTS[2].name = "qsort(3)";

        /* Ugly, but if we don't cast we get a dumb warning that I'd
         * rather not suppress. */
        SORTS[2].func = (int (*)(void*,
                                 size_t,
                                 size_t,
                                 int(*)(const void*, const void*)))&qsort;
}

void
allocate_sort_memory(uint32_t num_items)
{
        A = (int32_t *)malloc(num_items * sizeof(int32_t));
        if (A == NULL) {
                perror(progname);
                exit(2);
        }

        W = (int32_t *)malloc(num_items * sizeof(int32_t));
        if (W == NULL) {
                perror(progname);
                exit(2);
        }
}

void
allocate_results_memory(uint32_t nalgs, uint32_t niters)
{
        R = (double **)malloc(nalgs * sizeof(double *));
        if (R == NULL) {
                perror(progname);
                exit(2);
        }
        for (uint32_t i = 0; i < nalgs; i++) {
                R[i] = (double *)malloc(niters * sizeof(double));
                if (R[i] == NULL) {
                        perror(progname);
                        exit(2);
                }
        }
}

void
fill_array_with_random_uint32(int32_t *restrict A, uint32_t num_items)
{
        for (uint32_t i = 0; i < num_items; i++) {
                A[i] = (int32_t)arc4random_uniform(UINT32_MAX);
        }
}

int
compar(const void *x, const void *y)
{
        return *((int32_t*)x) < *((int32_t*)y) ? -1 : 1;
}

void
perform_sorts_and_record_results(uint32_t niters,
                                 uint32_t nel,
                                 int32_t *restrict A,
                                 int32_t *restrict W,
                                 double **restrict R)
{
        clock_t start, diff;
        double msecs;

        const uint32_t A_SIZE = nel * sizeof(uint32_t);
        const uint32_t width = sizeof(int32_t);

        for (uint32_t i = 0; i < niters; i++) {
                // Initialize A with new random data
                fill_array_with_random_uint32(A, nel);

                for (uint32_t j = 0; j < NALGS; j++) {
                        memcpy(W, A, A_SIZE);
                        fprintf(stderr,
                            "iteration: %6u, alg: %12s\r", i, SORTS[j].name);
                        time_procedure((*SORTS[j].func)(W, nel, width, &compar));
                        R[j][i] = msecs;
                }
        }
}

double
mean(double *A, uint32_t nel)
{
        double mean = 0;
        for (uint32_t i = 0; i < nel; i++) {
                mean = (A[i] + (mean * i)) / (double)(i + 1);
        }

        return mean;
}

/* Destructive (!) computation of the standard deviation of A */
double
stdev(double *A, double m, uint32_t nel)
{
        double res;
        for (uint32_t i = 0; i < nel; i++) {
                res = A[i] - m;
                A[i] = res * res;
        }

        return sqrt(mean(A, nel));
}

void
print_results(double **R, uint32_t nalgs, uint32_t niters)
{
        double mn;
        printf("%12s%16s%16s\n", "SORT", "MEAN (ms)", "STDEV (ms)");
        for (uint32_t i = 0; i < nalgs; i++) {
                mn = mean(R[i], niters);
                printf("%12s%14fms%14fms\n",
                    SORTS[i].name, mn, stdev(R[i], mn, niters));
        }
}

int
main(int argc, char **argv)
{
        if (argc != 3) {
                usage(1);
        }

        uint32_t niters = (uint32_t)atoll(argv[1]);
        uint32_t nel = (uint32_t)atoll(argv[2]);

        allocate_sort_memory(nel);
        allocate_results_memory(NALGS, niters);

        populate_SORTS();
        perform_sorts_and_record_results(niters, nel, A, W, R);
        print_results(R, NALGS, niters);

        exit(0);
}

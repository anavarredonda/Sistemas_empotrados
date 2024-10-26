#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#define THREADS_NUM 4
#define NUMBER_SUM 1000000000
#define PERIOD_TIME 900000000

void *
thread_function(void *arg) {

    clockid_t clockid = CLOCK_MONOTONIC;
    struct timespec start, finish, result, print_time;
    int id = *(int *)arg;

    for (int i = 0; i < 5; i++) {

        if (clock_gettime(clockid, &start) == -1) {
            fprintf(stderr, "Error getting the first time");
            pthread_exit(NULL);
        }

        volatile unsigned long long j;
        for (j=0; j < 250000000ULL; j++);

        if (clock_gettime(clockid, &finish) == -1) {
            fprintf(stderr, "Error getting the second time");
            pthread_exit(NULL);
        }

        if (start.tv_sec == finish.tv_sec) {
            result.tv_sec = finish.tv_sec - start.tv_sec;
            result.tv_nsec = finish.tv_nsec - start.tv_nsec;

        } else if (finish.tv_sec > start.tv_sec) {
            // Subtract one because it's the carry in subtraction
            result.tv_sec = finish.tv_sec - start.tv_sec - 1;

            result.tv_nsec = finish.tv_nsec - start.tv_nsec;

            //In order to know the real diference we add 1 follow with as many 0
            // as the number of digits result->tv_nsec has
            result.tv_nsec = NUMBER_SUM + result.tv_nsec;
        }

        if (clock_gettime(CLOCK_REALTIME, &print_time) == -1) {
            fprintf(stderr, "Error getting the print time");
            pthread_exit(NULL);
        }

        if (result.tv_nsec < PERIOD_TIME && result.tv_sec == 0) {
            fprintf(stdout, "[%ld.%ld] Thread %d - Iteracion %d: Coste=%ld.%ld s.\n",
                print_time.tv_sec, print_time.tv_nsec, id, i, result.tv_sec,
                result.tv_nsec);

            // sleep remaining time to next activation
            result.tv_nsec = PERIOD_TIME - result.tv_nsec;
            if (result.tv_nsec < 0) {
                result.tv_nsec = 0;
            }
            nanosleep(&result, NULL);

        } else {
            fprintf(stdout, "[%ld.%ld] Thread %d - Iteracion %d: Coste=%ld.%ld s.",
                print_time.tv_sec, print_time.tv_nsec, id, i, result.tv_sec,
                result.tv_nsec);
            fprintf(stdout, "(fallo temporal)\n");
        }
    }

    pthread_exit(NULL);
}

int
main(int argc, char const *argv[]) {
    int i, ids[THREADS_NUM];
    pthread_t thread_id[THREADS_NUM];

    for (i = 0; i < THREADS_NUM; i++) {
        ids[i] = i + 1;
        if (pthread_create(thread_id, NULL, thread_function, &ids[i]) != 0) {
            err(EXIT_FAILURE, NULL);
        }
    }

    for (i = 0; i < THREADS_NUM; i++) {
        if (pthread_join(thread_id[i], NULL) != 0) {
            err(EXIT_FAILURE, NULL);
        }
    }

    exit(EXIT_SUCCESS);
}

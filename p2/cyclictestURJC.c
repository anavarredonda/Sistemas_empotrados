#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>

// TODO Seguir con los tiempos, hacer los de 10ms
#define TIME_PROGRAM_NS 1e9

struct thread_info {
    int id_core;
    int **dataset;
    int iterations;
};

struct thread_info*
create_thread_info(int n_nucleos){
    struct thread_info *thread_info = malloc(sizeof(struct thread_info));
    if (thread_info == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    thread_info->dataset = malloc(sizeof(int *) * n_nucleos);
    for (int i = 0; i < n_nucleos; i++) {
        // 70 ->segundos programa    100->iteraciones por segundo
        thread_info->dataset[i] = malloc(sizeof(int) * 70 * 100);
    }

    return thread_info;
}

void
free_thread_info(struct thread_info *thread_info, int N) {
    for (int i = 0; i < N; i++) {
        free(thread_info->dataset[i]);
    }
    free(thread_info->dataset);
    free(thread_info);
}

long
seconds2nseconds(struct timespec timespec) {
    long nseconds;
    nseconds = timespec.tv_nsec + (timespec.tv_sec * 1e9);

    return nseconds;
}

void *
core_cal_thread(void *arg) {

    struct sched_param param;
    cpu_set_t cputset;
    struct thread_info *thread_info = (struct thread_info *)arg;
    struct timespec start_min, finish_min;
    clockid_t clockid = CLOCK_MONOTONIC;
    long a_nsecond, b_nsecond;

    param.sched_priority = 99;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam");
        exit(EXIT_FAILURE);
    }

    CPU_ZERO(&cputset);
    CPU_SET(thread_info->id_core ,&cputset);

    // Coger un minuto
    if (clock_gettime(clockid, &start_min) == -1) {
        fprintf(stderr, "Error getting the first time");
        pthread_exit(NULL);
    }

    if (clock_gettime(clockid, &finish_min) == -1) {
        fprintf(stderr, "Error getting the last time");
        pthread_exit(NULL);
    }

    b_nsecond = seconds2nseconds(start_min);
    a_nsecond = seconds2nseconds(finish_min);

    // Espera 1 min
    if (a_nsecond - b_nsecond > TIME_PROGRAM_NS) {
        pthread_exit(EXIT_SUCCESS);
    }
}

int
main(int argc, char *argv[]) {

    static int32_t latency_target_value = 0;
    int latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    write(latency_target_fd, &latency_target_value, 4);

    int N = (int) sysconf(_SC_NPROCESSORS_ONLN);
    if (N == -1) {
        perror("sysconf");
        exit(EXIT_FAILURE);
    }

    pthread_t *thread_ids = malloc(sizeof(pthread_t) * N);
    if (thread_ids == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    struct thread_info *thread_info = create_thread_info(N);
    

    for (int i = 0; i < N; i++) {
        thread_info->id_core = i;
        if (pthread_create(&thread_ids[i], NULL, core_cal_thread, thread_info) != 0) {
            err(EXIT_FAILURE, NULL);
        }
    }

    for (int i = 0; i < N; i++) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }  
    }
    
    free_thread_info(thread_info, N);
    close(latency_target_fd);
    free(thread_ids);
    exit(EXIT_SUCCESS);
}

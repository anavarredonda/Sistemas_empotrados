#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>

// TODO Arreglar free (solo funciona con 44 segundos), arreglar print
#define TIME_PROGRAM_NS 44e9//6e10
#define SLEEP_WAIT 1e7

struct thread_info {
    int id_core;
    int **dataset;
    int *iterations;
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
        // Espacio sobreestimado
        thread_info->dataset[i] = malloc(sizeof(int) * 70 * 1000);
    }
    thread_info->iterations = malloc(sizeof(int *) * n_nucleos);

    return thread_info;
}

void
free_thread_info(struct thread_info *thread_info, int n_nucleos) {
    for (int i = 0; i <= n_nucleos; i++) {
        fprintf(stderr, "Liberando: %d\n", i);
        free(thread_info->dataset[i]);
    }
    fprintf(stderr, "Liberando dataset\n");
    free(thread_info->dataset);
    fprintf(stderr, "Liberando iterations\n");
    free(thread_info->iterations);
    fprintf(stderr, "Liberando thread_info\n");
    free(thread_info);
    fprintf(stderr, "Todo liberado\n");
}

long
seconds2nseconds(struct timespec timespec) {
    long nseconds;
    nseconds = timespec.tv_nsec + (timespec.tv_sec * 1e9);

    return nseconds;
}

void *
core_calcu_thread(void *arg) {

    struct sched_param param;
    cpu_set_t cputset;
    struct thread_info *thread_info = (struct thread_info *)arg;
    struct timespec start_min, finish, start_measure;
    struct timespec duration;
    clockid_t clockid = CLOCK_MONOTONIC;
    long a_nsecond = 0, b_nsecond = 0, m_nsecond, latency_v;

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

    while (a_nsecond - b_nsecond < TIME_PROGRAM_NS) {

        //Coger tiempo, esperar 10ms, coger tiempo
        if (clock_gettime(clockid, &start_measure) == -1) {
            fprintf(stderr, "Error getting the last time");
            pthread_exit(NULL);
        }
        duration.tv_nsec = SLEEP_WAIT;
        nanosleep(&duration, NULL);

        if (clock_gettime(clockid, &finish) == -1) {
            fprintf(stderr, "Error getting the last time");
            pthread_exit(NULL);
        }

        b_nsecond = seconds2nseconds(start_min);
        m_nsecond = seconds2nseconds(start_measure);
        a_nsecond = seconds2nseconds(finish);

        latency_v = a_nsecond - m_nsecond - SLEEP_WAIT;
        thread_info->dataset[thread_info->id_core][thread_info->iterations[thread_info->id_core]] = latency_v;
        fprintf(stderr, "latency: %d\n", thread_info->dataset[thread_info->id_core][thread_info->iterations[thread_info->id_core]]);
        thread_info->iterations[thread_info->id_core]++;
    }
    pthread_exit(EXIT_SUCCESS);
}

void
print_results(struct thread_info* thread_info, int n_nucleos) {
    long avg = 0, max = 0;

    fprintf(stderr, "dentro de print\n");
    for (int i = 0; i <= n_nucleos; i++) {
        for (int j = 0; j < thread_info->iterations[n_nucleos]; j++) {
            //fprintf(stderr, "%d\n", thread_info->dataset[i][j]);
            avg = avg + thread_info->dataset[i][j];
            if (max < thread_info->dataset[i][j]) {
                max = thread_info->dataset[i][j];
            }  
        }
        avg = avg / thread_info->iterations[n_nucleos];
        printf("[%d]    latencia media = %09ld ns. | max = %09ld ns\n", i, avg, max); 
    }
}

void 
save_results(struct thread_info* thread_info) {

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

    struct thread_info* thread_info = create_thread_info(N);

    for (int i = 0; i < N; i++) {
        thread_info->id_core = i;
        if (pthread_create(&thread_ids[i], NULL, core_calcu_thread, thread_info) != 0) {
            err(EXIT_FAILURE, NULL);
        }
    }

    for (int i = 0; i < N; i++) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }  
    }

    print_results(thread_info, N);
    //save_results(thread_info);
    
    free_thread_info(thread_info, N);
    close(latency_target_fd);
    free(thread_ids);
    exit(EXIT_SUCCESS);
}

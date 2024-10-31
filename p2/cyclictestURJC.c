#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <string.h>

#define TIME_PROGRAM_NS 60000000000 
#define SLEEP_WAIT 10000000

struct thread_info {
    int id_core;
    int *dataset;
    int iterations;
};

struct thread_info*
create_thread_info(int n_nucleos){
    struct thread_info *thread_info = malloc(sizeof(struct thread_info));
    if (thread_info == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    thread_info->dataset = malloc(sizeof(int) * 70 * 100);
    if (thread_info->dataset == NULL) {
        perror("malloc");
        free(thread_info);
        exit(EXIT_FAILURE);
    }

    thread_info->iterations = 0;
    return thread_info;
}

void
free_thread_info(struct thread_info *thread_info, int n_nucleos) {

    free(thread_info->dataset);
    free(thread_info);
}

void *
core_calcu_thread(void *arg) {
    struct sched_param param;
    cpu_set_t cputset;
    struct thread_info *thread_info = (struct thread_info *)arg;
    struct timespec start_min, finish, start_measure, duration;
    clockid_t clockid = CLOCK_MONOTONIC;
    long long a_nsecond = 0, b_nsecond = 0, m_nsecond, latency_v, time_exe = 0;

    param.sched_priority = 99;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam");
        exit(EXIT_FAILURE);
    }

    CPU_ZERO(&cputset);
    CPU_SET(thread_info->id_core, &cputset);

    if (clock_gettime(clockid, &start_min) == -1) {
        fprintf(stderr, "Error getting the first time");
        pthread_exit(NULL);
    }
    fprintf(stderr, "Valores reloj 1: %ld:%ld\n", start_min.tv_sec, start_min.tv_nsec);

    duration.tv_nsec = SLEEP_WAIT;
    duration.tv_sec = 0;
    
    b_nsecond = (long long)start_min.tv_nsec + ((long long)start_min.tv_sec * 1000000000);

    while (time_exe < TIME_PROGRAM_NS) {

        if (clock_gettime(clockid, &start_measure) == -1) {
            fprintf(stderr, "Error getting the last time");
            pthread_exit(NULL);
        }
        fprintf(stderr, "Valores reloj 2: %ld:%ld\n", start_measure.tv_sec, start_measure.tv_nsec);

        nanosleep(&duration, NULL);

        if (clock_gettime(clockid, &finish) == -1) {
            fprintf(stderr, "Error getting the last time");
            pthread_exit(NULL);
        }
        fprintf(stderr, "Valores reloj 3: %ld:%ld\n", finish.tv_sec, finish.tv_nsec);

	    m_nsecond = start_measure.tv_nsec + (start_measure.tv_sec * 1000000000);
	    a_nsecond = (long long)(finish.tv_nsec) + ((long long)(finish.tv_sec) * 1000000000);

        latency_v = a_nsecond - m_nsecond - SLEEP_WAIT;
        thread_info->dataset[thread_info->iterations] = latency_v;
        thread_info->iterations++;

	time_exe = a_nsecond - b_nsecond;
	fprintf(stderr, "Tiempo que se lleva: %lld", time_exe);
    }
    //fprintf(stderr, "Saliendo de hilo %d\n", thread_info->id_core);
    pthread_exit((void *)thread_info);
}

void
print_results(struct thread_info** thread_infos, int n_nucleos) {
    long long avg, max;
    //fprintf(stderr, "Dentro de print_results\n");
    for (int i = 0; i < n_nucleos; i++) {
        avg = 0;
        max = 0;
        
        for (int j = 0; j < thread_infos[i]->iterations; j++) {
            avg += thread_infos[i]->dataset[j];
            if (max < thread_infos[i]->dataset[j]) {
                max = thread_infos[i]->dataset[j];
            }
        }
        if (thread_infos[i]->iterations > 0) {
            avg /= thread_infos[i]->iterations;
        }
        printf("[%d]    latencia media = %09lld ns. | max = %09lld ns\n", i, avg, max);
    }
}

void 
save_results(struct thread_info** thread_infos, int n_nucleos) {
    char buf[(sizeof(int) * 3) + 10]; //size of 3 int + size of \n\0
    ssize_t bytes_writen, result;
    size_t len;

    int fd = open("cyclictestURJC_data", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_nucleos; i++) {
        for (int j = 0; j < thread_infos[i]->iterations; j++) {
            memset(buf, 0, sizeof(buf));
            if (snprintf(buf, sizeof(buf), "%d,%d,%d\n", 
                        thread_infos[i]->id_core, j, 
                        thread_infos[i]->dataset[j]) < 0) {
                perror("snprintf");
                continue;
            }

            len = strlen(buf);
            bytes_writen = 0;

            while (bytes_writen < len) {
                result = write(fd, buf + bytes_writen, len - bytes_writen);
                if (result == -1) {
                    perror("write");
                    break;
                }
                bytes_writen += result;
            }
        }
    }
    close(fd);
}

int
main(int argc, char *argv[]) {
    struct thread_info* thread_info;
    struct thread_info** thread_info_array;
    void* ret_val;

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

    thread_info_array = malloc(sizeof(struct thread_info*) * N);
    if (thread_info_array == NULL) {
        perror("malloc");
        free(thread_ids);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        thread_info = create_thread_info(N);
        thread_info->id_core = i;

        if (pthread_create(&thread_ids[i], NULL, core_calcu_thread, thread_info) != 0) {
            err(EXIT_FAILURE, NULL);
        }
    }

    for (int i = 0; i < N; i++) {
        if (pthread_join(thread_ids[i], &ret_val) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
        thread_info = (struct thread_info*)ret_val;
        thread_info_array[thread_info->id_core] = thread_info;
    }

    print_results(thread_info_array, N);
    save_results(thread_info_array, N);

    for (int i = 0; i < N; i++) {
        free_thread_info(thread_info_array[i], N);
    }
    free(thread_info_array);
    free(thread_ids);
    close(latency_target_fd);
    exit(EXIT_SUCCESS);
}

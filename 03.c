#include <stdio.h>     // fprint, sprintf
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork
#include <time.h>
#include <stdlib.h>    // atoi

/*
 * Создать дерево процессов по индивидуальному заданию. Каждый процесс постоянно,
 * через время t, выводит на экран следующую информацию: номер процесса/потока, pid, ppid текущее время (мсек).
 * Время t=(номер процесса/потока по дереву) * 200 (мсек).
 *
 * на вход 2 аргумента: количество поколений и количество потомков у каждого из процессов.
 * пример дерева с аргументами 2 2:
    родитель
    - потомок 1 поколения
    - - потомок 2 поколения
    - - потомок 2 поколения
    - потомок 1 поколения
    - - потомок 2 поколения
    - - потомок 2 поколения
Время должно выводиться по указанной в задании формуле.
 */

const int TIME_STR_LEN = 2 * 3 + 3;

void assign_time_to_str(struct timespec timer, char full_time[TIME_STR_LEN]) {
    struct tm* my_local_time = localtime(&timer.tv_sec);

    char time_buff[70];
    strftime(time_buff, sizeof time_buff, "%H : %M : %S", my_local_time);

    long ms_expired = timer.tv_nsec / 1000 / 1000;
    sprintf(full_time, "%s : %ld", time_buff, ms_expired);
}

void build_tree(
        int total_levels,
        int num_siblings,
        int tree_level,
        pid_t parent_pid,
        pid_t root_pid,
        int *node_id_ptr) {

    if (tree_level == total_levels) {
        return;
    }

    if (getpid() == parent_pid) {
        for (int i = 0; i < num_siblings; i++) {
            pid_t cur_pid = fork();
            sleep(4);
            if (cur_pid == 0) {
                *node_id_ptr = getpid() - root_pid + 1;
                build_tree(
                        total_levels,
                        num_siblings,
                        tree_level + 1,
                        getpid(),
                        root_pid,
                        node_id_ptr);
                return;
            }
        }
    }
}

struct timespec calc_timespecs_dif(struct timespec past_time, struct timespec cur_time) {
    struct timespec diff = {
            .tv_sec = cur_time.tv_sec - past_time.tv_sec,
            .tv_nsec = cur_time.tv_nsec - past_time.tv_nsec
    };
    if (diff.tv_nsec < 0) {
        diff.tv_nsec += 1000000000;
        diff.tv_sec--;
    }
    return diff;
}

long timespec_to_ms(struct timespec elapsed_time) {
    long time = elapsed_time.tv_sec * 1000 + elapsed_time.tv_nsec / 1000 / 1000;
    return time;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Number of levels was not supplied!\n");
        return 1;
    } else if (argc < 3) {
        printf("Number of siblings was not supplied!\n");
        return 1;
    }

    int total_levels = atoi(argv[1]);
    int num_siblings = atoi(argv[2]);
    int node_id = 1;

    build_tree(
            total_levels,
            num_siblings,
            0,
            getpid(),
            getpid(),
            &node_id);
    printf("initialization: id = %d, pid = %d, ppid = %d\n",
           node_id, getpid(), getppid());

    struct timespec last_time;
    struct timespec now;
    struct timespec time_diff;
    char time_str[TIME_STR_LEN];

    clock_gettime(CLOCK_REALTIME, &last_time);

    long time_interval = node_id * 200; // ms

    while (1) {
        clock_gettime(CLOCK_REALTIME, &now);
        time_diff = calc_timespecs_dif(last_time, now);
        long time_diff_ms = timespec_to_ms(time_diff);
        if (time_diff_ms >= time_interval) {
            assign_time_to_str(now, time_str);
            printf("id = %d, pid = %d, ppid = %d, time = %s\n",
                   node_id, getpid(), getppid(), time_str);
            clock_gettime(CLOCK_REALTIME, &last_time);
        }
    };
    return 0;
}

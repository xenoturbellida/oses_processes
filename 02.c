#include <stdio.h>     // fprint, sprintf
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork
#include <time.h>      // time, timespec
#include <stdlib.h>    // system
#include <stdbool.h>   // bool
#include <string.h>    // strstr

/*
 * Написать программу, создающую два дочерних процесса с использованием двух вызовов fork().
 * Родительский и два дочерних процесса должны выводить на экран свой pid и pid родительского процесса
 * и текущее время в формате: часы : минуты : секунды : миллисекунды.
 * Используя вызов system(), выполнить команду ps -x в родительском процессе.
 * Найти свои процессы в списке запущенных процессов.
*/

const int PID_LEN = 8 * sizeof(int) + 1;
const int TIME_STR_LEN = 2 * 3 + 3;

void assign_current_time(char full_time[TIME_STR_LEN]) {
    struct timespec my_timer;
    clock_gettime(CLOCK_REALTIME, &my_timer);

    struct tm* my_local_time = localtime(&my_timer.tv_sec);

    char time_buff[70];
    strftime(time_buff, sizeof time_buff, "%H : %M : %S", my_local_time);

    long ms_expired = my_timer.tv_nsec / 1000 / 1000;
    sprintf(full_time, "%s : %ld", time_buff, ms_expired);
}

bool check_if_procs_exist(const char filename[], pid_t parent_pid, pid_t child_pid1, pid_t child_pid2) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error while opening file %s!\n", filename);
        return false;
    }

    bool parent_pid_was_found = false;
    bool child_pid1_was_found = false;
    bool child_pid2_was_found = false;

    char parent_pid_str[PID_LEN];
    char child_pid1_str[PID_LEN];
    char child_pid2_str[PID_LEN];

    sprintf(parent_pid_str, "%d", parent_pid);
    sprintf(child_pid1_str, "%d", child_pid1);
    sprintf(child_pid2_str, "%d", child_pid2);

    getline(&line, &len, fp);
    printf("\n%s", line);
    while ((read = getline(&line, &len, fp)) != -1) {
        if (!parent_pid_was_found
                && strstr(line, parent_pid_str) != NULL) {
            printf("%s", line);
            parent_pid_was_found = true;
        }
        else if (!child_pid1_was_found
            && strstr(line, child_pid1_str) != NULL) {
            printf("%s", line);
            child_pid1_was_found = true;
        }
        else if (!child_pid2_was_found
            && strstr(line, child_pid2_str) != NULL) {
            printf("%s", line);
            child_pid2_was_found = true;
        }
    }

    if (fclose(fp) == -1) {
        printf("Error while closing file %s!\n", filename);
        return false;
    };

    return parent_pid_was_found & child_pid1_was_found & child_pid2_was_found;
}

void print_proc(char owner[32], char time_str[TIME_STR_LEN]) {
    printf("%s, %s, pid = %d, ppid = %d\n", owner, time_str, getpid(), getppid());
}

int main(int argc, char** argv) {
    pid_t pid_child_1;
    pid_t pid_child_2;

    char current_time_str[TIME_STR_LEN];
    const char PROC_FILENAME[] = "proc_snapshot.txt";
    bool all_procs_were_found = false;

    pid_child_1 = fork();

    if (pid_child_1 == 0) {
        assign_current_time(current_time_str);
        print_proc("Child 1", current_time_str);
    } else if (pid_child_1 > 0) {
        pid_child_2 = fork();
        if (pid_child_2 == 0) {
            assign_current_time(current_time_str);
            print_proc("Child 2", current_time_str);
        } else if (pid_child_2 > 0) {
            assign_current_time(current_time_str);
            print_proc("Parent", current_time_str);

            char command_buf[64];
            sprintf(command_buf, "ps -x > %s", PROC_FILENAME); // The x option causes ps to list all processes owned by the current user.
            system(command_buf);
            all_procs_were_found = check_if_procs_exist(
                    PROC_FILENAME,
                    getpid(),
                    pid_child_1,
                    pid_child_2);
            if (!all_procs_were_found) {
                printf("\nNot all processes were found in %s!\n", PROC_FILENAME);
                return 1;
            } else {
                printf("\nAll processes were found\n");
            }
        }
    }

    return 0;
}

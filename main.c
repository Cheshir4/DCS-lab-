#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "all.h"
#include "pa2345.h"
#include "ipc.h"
#include "banking.h"
#include "lamport.h"

int main(int argc, char** argv) {

    int X;
    int flag = 0;

    pid_t fork_pid[10];
    pid_t parent_pid;
    int local_id = 0;
    int status_wait;
    int pid;

    FILE *file_log;

    if (argc == 0) {
        printf("No arguments, error\n");
        exit(0);
    }

    if (strcmp(argv[1], "-p") == 0) {
        X = atoi(argv[2]);
    };

    if (argc == 4) {
        if (strcmp(argv[2], "-p") == 0) {
            X = atoi(argv[3]);
        };

        if ((strcmp(argv[1], "--mutexl") == 0) || (strcmp(argv[3], "--mutexl") == 0)) {
            flag = 1;
        };

    }

    if (X <= 0) {
        printf("Argument incorrect, error\n");
        exit(0);
    }

    //printf("flag = %i\n", flag);

    parent_pid = getpid();

    for (int i = 0; i <= X; i++) {
        for (int j = 0; j <= X; j++) {
            if ( i != j ) {
                pipe2(pipes[i][j], O_NONBLOCK);
            }
        }
    }

    file_log = fopen("events.log", "w");


    fork_pid[0] = parent_pid;

    // ФОРК ЗДЕСЬ!!!!!!!!!!!!!!!!!!
    for (int i = 1; i <= X; i++) {

            pid = fork();

            switch (pid) {
                case -1:
                    break;
                case 0: {
                    int balance = 0;


                    //printf("process %i: %i\n", i, balance);

                    char msg[256];
                    local_id = i;
                    close_pipes(X, local_id);

                    inc_lamport();

                    //формирование строки для вывода и отправки в сообщ (STARTED)
                    int l = sprintf(msg, log_started_fmt, get_lamport_time(), local_id, getpid(), parent_pid, balance);
                    msg[l] = '\0';

                    fprintf(file_log, "%s", msg);
                    fflush(file_log);

                    //формирование msg
                    MessageType type = STARTED;
                    self_structure slf;
                    slf.X = X;
                    slf.id_sourse = local_id;
                    slf.type = type;

                    send_msg(msg, slf);

                    //получили от всех старт
                    Message msg_get;
                    self_structure slf_get;
                    slf_get.X = X;
                    slf_get.id_sourse = local_id;
                    resive_all(&slf_get, &msg_get, STARTED);

                    //вывод в лог о готовности всех детей
                    fprintf(file_log, log_received_all_started_fmt, get_lamport_time(),local_id);
                    fflush(file_log);

                    //int count = 0;


                    if (flag == 1) {
                        //printf("Кря\n");
                        work(&slf_get, 0, file_log);

                    } else {
                        for (int i = 0; i <= local_id * 5; i++){
                            char msg[256];
                            int l = sprintf(msg, log_loop_operation_fmt, local_id, i, local_id * 5);
                            msg[l] = '\0';
                            print(msg);
                        }

                    }


                    fprintf(file_log, log_done_fmt, get_lamport_time(), local_id, balance);
                    fflush(file_log);

                    if (flag == 0) {
                        inc_lamport();

                        slf.type = DONE;

                        l = sprintf(msg, log_done_fmt, get_lamport_time(), local_id, 0);
                        msg[l] = '\0';

                        send_msg(msg, slf);

                        slf_get.X = X;
                        slf_get.id_sourse = local_id;
                        printf("КРя\n");
                        resive_all(&slf_get, &msg_get, DONE);

                    }

                    fprintf(file_log, log_received_all_done_fmt, get_lamport_time(), local_id);
                    fflush(file_log);


                    exit(0);
                }
                default:
                    fork_pid[i] = pid;
                    break;
            }
    }


    local_id = 0;
    close_pipes(X, local_id);


    Message msg_get;
    self_structure slf_get;
    slf_get.X = X;
    slf_get.id_sourse = local_id;
    resive_all(&slf_get, &msg_get, STARTED);

    sleep(1);
    //bank_robbery(&slf_get, X);


    //Отправить всем стоп
    inc_lamport();


    //resive_all(&slf_get, &msg_get, DONE);

    for (int i = 1; i <= X; i++) {
        waitpid(fork_pid[i], &status_wait, 0);
    }

   //printf("Конец!\n");


    exit(0);

}

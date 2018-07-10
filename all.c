//
// Created by ulyaf on 29.04.2018.
//

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "all.h"
#include "ipc.h"
#include "banking.h"
#include "pa2345.h"
#include "lamport.h"
#include <errno.h>

void close_pipes(int X, int local_id) {
    for (int i = 0; i <= X; i++) {
        for (int j = 0; j <= X; j++) {
            if ( ((local_id != i) && (local_id != j)) ) {
                close(pipes[i][j][0]);
                close(pipes[i][j][1]);
                continue;
            }
            if (i == local_id) {
                close(pipes[i][j][1]);
                continue;
            }
            if (j == local_id) {
                close(pipes[i][j][0]);
            }
        }
    }
}

void work(self_structure * slf, BalanceHistory * balanceHistory, FILE * file_log) {

    int i = 1;
    slf->done = 0;
    slf->count_reply = 0;
    slf->queue_length = 0;

    sleep(1);
    while (slf->done < slf->X - 1) {

        while (i <= slf->id_sourse * 5) {
            //printf("Кря\n");
            request_cs(slf);

            char msg[256];
            int l = sprintf(msg, log_loop_operation_fmt, slf->id_sourse, i, slf->id_sourse * 5);
            msg[l] = '\0';
            print(msg);

            release_cs(slf);

            i++;
            slf->count_reply = 0;

            if ( i == slf->id_sourse * 5 + 1) {

                //printf("Я есть закончиль!\n");
                //Отправить DONE
                inc_lamport();

                slf->type = DONE;

                l = sprintf(msg, log_done_fmt, get_lamport_time(), slf->id_sourse, 0);
                msg[l] = '\0';

                send_msg(msg, *slf);
            }
        }

        if (slf->id_sourse == slf->X && slf->done == slf->X-1) {
            //printf(" UUUEEE %i\n", slf->id_sourse);
            slf->done++;
            //printf( "END DONE %i", slf->done);
        } else {
            get_and_analyze_msg(slf);
        }

        //printf(" UUUEEE %i\n", slf->id_sourse);
    }
    //printf("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
}

//подготовка к отправке ВСЕХ сообщений и их отправка
void send_msg(char* str_to_msg, self_structure slf) {
    MessageHeader msg_header;
    Message msg;

   // printf("t %i\n", slf.id_sourse);


    msg_header.s_magic = MESSAGE_MAGIC;
    msg_header.s_local_time = get_lamport_time();
    msg_header.s_type = slf.type;
    msg_header.s_payload_len = strlen(str_to_msg);

    //printf("vze:>%s<\n", str_to_msg);
    slf.len_str_msg = msg_header.s_payload_len;

    msg.s_header = msg_header;
    strncpy(msg.s_payload, str_to_msg, msg_header.s_payload_len);

    send_multicast(&slf, &msg);

}

void resive_all(self_structure * slf, Message * msg, int type) {

    int count = 0;
    int code = 0;

    if (slf->id_sourse == 0)
        count = -1;

    while (count < (slf->X - 1)) {
        code = receive_any(slf, msg);
        if ((code == 0) && (msg->s_header.s_type == type))
            count++;
        sleep(1);
    }

}


void do_full_balance(timestamp_t s_time, BalanceHistory * balanceHistory) {
    //return;
    balanceHistory->s_history_len = s_time;
    int time = (int) s_time;
    for (int i = 1; i < time; i++) {
        if (balanceHistory->s_history[i].s_time != i) {
            balanceHistory->s_history[i].s_time = i;
            balanceHistory->s_history[i].s_balance = balanceHistory->s_history[i - 1].s_balance;
            balanceHistory->s_history[i].s_balance_pending_in = 0;
        }
        //

    }
}



int request_cs(const void * self) {

    self_structure * slf = (self_structure*)self;

    //printf("request\n");

    inc_lamport();

    add_queue(slf, slf->id_sourse, get_lamport_time());


    slf->type = CS_REQUEST;

    char msg[1] = "\0";
    //slf->id_sourse = 10;
    send_msg(msg, *slf);

    while (slf->count_reply < slf->X - 1) {
        get_and_analyze_msg(slf);
        //printf("id = %i, count_reply = %i\n", slf->id_sourse, slf->count_reply);
    }
    while (slf->queue[0].id != slf->id_sourse) {
        //printf("ПРишел\n");
        get_and_analyze_msg(slf);
        //printf("МЯУ\n");
    }

    return 0;
}



int release_cs(const void * self) {

    self_structure * slf = (self_structure*)self;

    for (int i = 0; i < slf->queue_length - 1; i++) {
        slf->queue[i] = slf->queue[i + 1];
    }
    slf->queue_length--;

    inc_lamport();

    slf->type = CS_RELEASE;

    char msg[1] = "\0";
    send_msg(msg, *slf);

    return 0;
}

int add_queue(self_structure * slf, int id, timestamp_t time) {
    special sp;


    sp.id = id;

    sp.time = time;

    slf->queue[slf->queue_length] = sp;

    slf->queue_length++;
    //printf("add %i\n", slf->queue_length);

    for(int i = 0 ; i < slf->queue_length; i++) {

        for(int j = 0 ; j < slf->queue_length - i - 1 ; j++) {
            if(slf->queue[j].time > slf->queue[j+1].time) {
                special tmp = slf->queue[j];
                slf->queue[j] = slf->queue[j+1];
                slf->queue[j+1] = tmp;
            }

            if ((slf->queue[j].time == slf->queue[j+1].time) && (slf->queue[j].id > slf->queue[j+1].id)) {
                special tmp = slf->queue[j];
                slf->queue[j] = slf->queue[j+1];
                slf->queue[j+1] = tmp;
            }
        }
    }


    for (int i = 0; i < slf->queue_length; i++) {
        //printf("mass %i for %i: id %i time % i\n", i, slf->id_sourse, slf->queue[i].id, slf->queue[i].time);
    }
    //printf("add end\n");

    return 0;

}

void get_and_analyze_msg(self_structure * slf) {

    //printf("КРЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯЯяя");

    Message msg_get;

    int code = -1;

    int i;
    while (code != 0) {
        for (i = 0; i <= slf->X; i++) {
            if (i == slf->id_sourse) {
                continue;
            }
            code = receive(slf, i, &msg_get);
            //sleep(1);
            //printf("meow %i\n", slf->id_sourse);
            if (code == 0) break;
        }

        if (code != EAGAIN) {
            //printf("code = %i\n", code);
        }
    }

    if (msg_get.s_header.s_type == DONE) {

        slf->done++;
        //printf("done %i  id %i\n", slf->done, slf->id_sourse);
    } else if (msg_get.s_header.s_type == CS_REQUEST) {
        //printf("req %i from %i\n", slf->id_sourse, i);
        add_queue(slf, i, msg_get.s_header.s_local_time);

        inc_lamport();

        //slf->type = CS_REPLY;

        char msg_str[1] = "\0";

        Message msg;
        msg.s_header.s_magic = MESSAGE_MAGIC;
        msg.s_header.s_local_time = get_lamport_time();
        msg.s_header.s_type = CS_REPLY;
        msg.s_header.s_payload_len = sizeof(msg_str);
        memcpy(msg.s_payload, &msg_str, sizeof(msg_str));


        send(slf, i, &msg);
        //send_msg(msg, *slf);
    } else if (msg_get.s_header.s_type == CS_REPLY) {
        //printf("repl %i from %i\n", slf->id_sourse, i);
        slf->count_reply++;
    } else if (msg_get.s_header.s_type == CS_RELEASE) {
        //printf("rele\n");
        for (int n = 0; n < slf->queue_length - 1; n++) {
            slf->queue[n] = slf->queue[n + 1];
        }
        slf->queue_length--;
    } else
    {
        //printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa!");
    }
}

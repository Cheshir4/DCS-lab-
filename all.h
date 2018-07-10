//
// Created by ulyaf on 29.04.2018.
//

#ifndef UNTITLED_ALL_H
#define UNTITLED_ALL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipc.h"
#include "banking.h"

int pipes[10][10][2]; // 0 - чтение, 1 - запись

typedef struct {
    int id;
    timestamp_t time;
} special;

typedef struct {
    int X;
    int id_sourse;
    int len_str_msg;
    MessageType type;
    int done;
    int count_reply;
    special queue[10];
    int queue_length;

} self_structure;

void close_pipes(int X, int local_id);

void work(self_structure * slf, BalanceHistory * balanceHistory, FILE * file_log);

void send_msg(char* str_to_msg, self_structure slf);

void resive_all(self_structure * slf, Message * msg, int type);

void do_full_balance(timestamp_t s_time, BalanceHistory * balanceHistory);

int add_queue(self_structure * slf, int id, timestamp_t time);

void get_and_analyze_msg(self_structure * slf);

#endif //UNTITLED_ALL_H

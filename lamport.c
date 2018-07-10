//
// Created by ulyaf on 15.05.2018.
//
#include "lamport.h"
#include "banking.h"

int time_s = 0;

void inc_lamport() {
    time_s++;

}

void add_lamport(int time) {
    if (time > time_s) {
        time_s = time;
    }
}

timestamp_t get_lamport_time() {
    return time_s;
}

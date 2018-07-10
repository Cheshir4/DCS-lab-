//
// Created by ulyaf on 29.04.2018.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "ipc.h"
#include "all.h"
#include <errno.h>
#include "lamport.h"

// собственно отправка ОДНОГО сообщения
int send(void * self, local_id dst, const Message * msg) {

    self_structure * slf = (self_structure*)self;
    //printf("process %i send %i to %i time %i\n", slf->id_sourse, msg->s_header.s_type, dst, msg->s_header.s_local_time);
    write(pipes[dst][slf->id_sourse][1], msg, msg->s_header.s_payload_len + sizeof(MessageHeader));
	return 0;
}


//рассылка ВСЕХ сообщений
int send_multicast(void * self, const Message * msg) {

    self_structure * slf = (self_structure*)self;
    for (int i = 0; i <= slf->X; i++) {
        if (i != slf->id_sourse)
            send(slf, i, msg);
    }
	return 0;
}

int receive(void * self, local_id from, Message * msg) {

    self_structure * slf = (self_structure*)self;
    ssize_t code;
	//printf("wat? %i\n", slf->id_sourse);
    code = read(pipes[slf->id_sourse ][from][0], &msg->s_header, sizeof(MessageHeader));
    //printf("process %i received %i from %i with code %i\n", slf->id_sourse, msg->s_header.s_type, from, code);
    //perror(",eow");
    if (code > 0) {
        code = read(pipes[slf->id_sourse][from][0], &msg->s_payload, msg->s_header.s_payload_len);

        add_lamport(msg->s_header.s_local_time);
        inc_lamport();

        //printf("process %i received %i from %i time %i\n", slf->id_sourse, msg->s_header.s_type, from, msg->s_header.s_local_time);
        if (code >= 0)
            return (0);
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            return  EAGAIN;
        return (-1);
    }

    if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        return  EAGAIN;
    return (-1);

}

int receive_any(void * self, Message * msg) {

    self_structure * slf = (self_structure*)self;

    int code;

    for (int i = 0; i <= slf->X; i++) {

        if (slf->id_sourse != i) {
            code = receive(slf, i, msg);

            if ((code != -1) && (code != EAGAIN))
                return (0);
        }
    }

    return -1;

}

#include <string.h>
#include <stdio.h>


#include "banking.h"
#include "all.h"
#include "lamport.h"

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount) {

    self_structure * slf = (self_structure*)parent_data;
    //slf->id_sourse = 0;

    TransferOrder transferOrder;
    transferOrder.s_src = src;
    transferOrder.s_dst = dst;
    transferOrder.s_amount = amount;

    inc_lamport();

    Message msg;
    msg.s_header.s_magic = MESSAGE_MAGIC;
    msg.s_header.s_local_time = get_lamport_time();
    msg.s_header.s_type = TRANSFER;
    msg.s_header.s_payload_len = sizeof(transferOrder);
    memcpy(msg.s_payload, &transferOrder, sizeof(TransferOrder));

    send(slf, src, &msg);


    MessageType type = TRANSFER;
    while (type != ACK) {
        //printf("мяу\n");
        receive(slf, dst, &msg);
        type = msg.s_header.s_type;
    }

}


/*
int main(int argc, char * argv[])
{
    //bank_robbery(parent_data);
    //print_history(all);

    return 0;
}
*/

#include <stdlib.h>
#include <stdio.h>

#include "headers/msgwrappers.h"

int msgGet(key_t mkey, int flags){
    int ret;
    if((ret = msgget(mkey, flags)) < 0){
        perror("msgget failed");
        exit(2);
    }
    return ret;
}

int msgCtl(int msq, int flags, struct msqid_ds *msgStatus){
    int ret;
    if((ret = msgctl(msq, flags, msgStatus)) < 0){
        perror("msgctl failed");
        exit(3);
    }
}

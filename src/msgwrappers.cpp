#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include "headers/msgwrappers.h"

int msgGet(key_t mkey, int flags){
    int ret;
    if((ret = msgget(mkey, flags)) < 0){
        perror("msgget failed");
        exit(2);
    }
    return ret;
}

void msgCtl(int msq, int flags, struct msqid_ds *msgStatus){
    if(msgctl(msq, flags, msgStatus) < 0){
        perror("msgctl failed");
        exit(3);
    }
}

void msgSnd(int msq, const MsgBuff *msgbuff, int size, int flags){
    if(msgsnd(msq, reinterpret_cast<const void*>(msgbuff), size, flags) == -1){
        perror("msgsnd failed");
        exit(4);
    }
}

int msgRcv(int msq, MsgBuff *msgbuff, int size, long type, int flags, int *read){
    if((*read = msgrcv(msq, reinterpret_cast<void*>(msgbuff), size, type, flags)) == -1){
        //no messages to read or an interupt stopped us
        if(errno == ENOMSG || errno == EINTR) {
            return 0;
        }
        perror("msgrcv failed");
        exit(5);
    }
    return 1;
}

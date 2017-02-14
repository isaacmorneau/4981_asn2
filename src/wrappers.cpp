#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/sem.h>
#include "headers/wrappers.h"

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

int semGet(key_t key, int value){
    int ret;
    if((ret = semget(key, value, 0666 | IPC_CREAT)) == -1){
        perror("semget failed");
        exit(6);
    }
    return ret;
}

void semSet(int sid, int value){
    if(semctl(sid, 0, SETVAL, value) == -1){
        perror("semctl failed");
        exit(7);
    }
}

void semWait(int sid){
    struct sembuf buf[1];
    buf[0].sem_num = 0;
    buf[0].sem_op = -1;
    buf[0].sem_flg = 0;

    //buf[1].sem_num = 0;
    //buf[1].sem_op = 0;
    //buf[1].sem_flg = 0;
    if(semop(sid, buf, 1) == -1){
        if(errno == EINTR)
            return;
        perror("semop wait failed");
        exit(8);
    }
}

void semSignal(int sid, int value){
    struct sembuf buf[1];
    buf->sem_num = 0;
    buf->sem_op = value;
    buf->sem_flg = 0;
    if(semop(sid,buf,1) == -1){
        if(errno == EINTR)
            return;
        perror("semop signal failed");
        exit(9);
    }
}

void semRelease(int sid){
    if(semctl(sid, 0, IPC_RMID, 0) == -1){
        perror("semctl failed");
    }
}

int semGetValue(int sid){
    int ret;
    errno = 0;
    if((ret = semctl(sid, 0, GETVAL)) == -1){
        if(errno){
            perror("semctl get failed");
            exit(7);
        }
    }
    return ret;
}



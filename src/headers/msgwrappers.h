#ifndef MSGWRAPPERS_H
#define MSGWRAPPERS_H

#include <sys/msg.h>

#define BUFFSIZE 1024

int msgGet(key_t mkey, int flags);

void msgCtl(int msq, int flags, struct msqid_ds *msgStatus = 0);

struct MsgBuff {
    long mtype;
    char mtext[BUFFSIZE];
};

void msgSnd(int msq, const MsgBuff *msgbuff, int size, int flags);

int msgRcv(int msq, MsgBuff *msgbuff, int size, long type, int flags);

#endif

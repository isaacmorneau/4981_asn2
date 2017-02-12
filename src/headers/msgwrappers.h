#ifndef MSGWRAPPERS_H
#define MSGWRAPPERS_H

#include <sys/msg.h>

#define BUFFSIZE 1024
//text of buffsize for use in printf formatting
#define BUFFSIZETXT "1024"

#define TO_SERVER 1
//generate a new unlikely id from the client pid
#define QUIT_CLIENT(x) ((x) ^ ((x) << 6)) 

#define FILE_NOT_FOUND 'N'
#define FILE_END 'E'

int msgGet(key_t mkey, int flags);

void msgCtl(int msq, int flags, struct msqid_ds *msgStatus = 0);

struct MsgBuff {
    long mtype;
    char mtext[BUFFSIZE];
};

void msgSnd(int msq, const MsgBuff *msgbuff, int size, int flags = 0);

int msgRcv(int msq, MsgBuff *msgbuff, int size, long type, int flags, int *read);

#endif

#ifndef MSGWRAPPERS_H
#define MSGWRAPPERS_H

#include <sys/msg.h>

int msgGet(key_t mkey, int flags);
int msgCtl(int msq, int flags, struct msqid_ds *msgStatus = 0);

#endif

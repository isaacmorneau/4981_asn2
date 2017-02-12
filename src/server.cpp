#include <cstring>
#include <string>
#include <stdlib.h>
#include "headers/server.h"
#include "headers/msgwrappers.h"

using namespace std;

void server(const int msgQueue){

    MsgBuff msg;
    int read;

    //msg.mtext = (char*)(calloc(BUFFSIZE, sizeof(char)));
    
    //block to wait for connection
    read = msgRcv(msgQueue, &msg, BUFFSIZE, 1, 0);
    
    printf("%d\n",read);
    
    msg.mtext[read] = '\0';
    
    printf(">%s\n", msg.mtext);
    //set it to what the client is looking for
    msg.mtype = 2;
    //send it back
    msgSnd(msgQueue, &msg, sizeof(msg.mtext), IPC_NOWAIT);
}

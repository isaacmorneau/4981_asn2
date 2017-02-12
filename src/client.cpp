#include <string>
#include <cstring>
#include "headers/client.h"
#include "headers/msgwrappers.h"

using namespace std;

void client(const int msgQueue, const string fileRequest){
    MsgBuff msg;
    int read;

    msg.mtype = 1;
    //msg.mtext = (char*)(malloc(BUFFSIZE * sizeof(char)));
    sprintf(msg.mtext, "im looking for %s",fileRequest.c_str());
    
    msgSnd(msgQueue, &msg, strlen(msg.mtext), IPC_NOWAIT);

    read = msgRcv(msgQueue, &msg, BUFFSIZE, 2, 0);

    msg.mtext[read] = '\0';
    printf("<%s\n", msg.mtext);
}


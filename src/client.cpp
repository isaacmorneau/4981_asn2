#include <string>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include "headers/client.h"
#include "headers/msgwrappers.h"

using namespace std;

void client(const int msgQueue, const string fileRequest, const int priority, const int verbose){
    MsgBuff msg;
    int read;
    int pid = getpid();

    msg.mtype = TO_SERVER;
    
    sprintf(msg.mtext, "%d %d %s", pid, priority, fileRequest.c_str());
    
    msgSnd(msgQueue, &msg, strlen(msg.mtext), IPC_NOWAIT);
    while(1){
        //non blocking check if the server wants us out
        if(msgRcv(msgQueue, &msg, BUFFSIZE, QUIT_CLIENT(pid), IPC_NOWAIT, &read)){
            msg.mtext[read] = '\0';
            if(verbose)
                printf("recieved %s from server, exiting...\n", msg.mtext);
            break;
        }
        while(msgRcv(msgQueue, &msg, BUFFSIZE, pid, IPC_NOWAIT, &read)) {
            msg.mtext[read] = '\0';
            if(verbose)
                printf("<%s\n", msg.mtext);
        }
    }
}


#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "headers/server.h"
#include "headers/msgwrappers.h"

#define FILESIZE 256

using namespace std;

void server(const int msgQueue, const int verbose){

    MsgBuff msg;
    int read;
    char file[FILESIZE];
    int clientPriority;
    int clientPid;

    while(1){
        //block to wait for connection request
        msgRcv(msgQueue, &msg, BUFFSIZE, TO_SERVER, 0, &read);
        if(!fork()){
            msg.mtext[read] = '\0';
            //who for, how fast and what
            sscanf(msg.mtext, "%d %d %s", &clientPid, &clientPriority, file);

            if(verbose){
                printf("file '%s' for process %d with priority %d\n", file, clientPid, clientPriority);
                printf(">%s\n", msg.mtext);
            }
            //set it to the client to recv it
            msg.mtype = clientPid;
            while(1){
                for(int i = 0; i < clientPriority; ++i) {
                    //make this where you get the next file data and remove the strcpy
                    strcpy(msg.mtext,"This is some random test data bla bla");

                    msgSnd(msgQueue, &msg, strlen(msg.mtext));
                    if(verbose)
                        printf("%d:[%d/%d]\n", clientPid, i+1, clientPriority);
                }
                //wait for server to give go ahead
                //for now just break until semaphore is added
                break;
            }

            msg.mtype = QUIT_CLIENT(clientPid);
            strcpy(msg.mtext, "File end");
            msgSnd(msgQueue, &msg, strlen(msg.mtext));
            exit(1);
        }
    }
}

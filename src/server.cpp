#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sys/signal.h>
#include "headers/server.h"
#include "headers/msgwrappers.h"

#define FILESIZE 256

using namespace std;

int sharedRunning = 1;

void interuptQuit(int signal){
    sharedRunning = 0;
    printf("Caught interupt, exiting.\n");
}

void server(const int msgQueue, const int verbose){

    MsgBuff msg;
    int read;
    char file[FILESIZE];
    int clientPriority;
    int clientPid;

    //catch ^c cancel of server
    signal(SIGINT,interuptQuit);

    while(sharedRunning){
        //TODO sync slaver servers and make nonblock
        if(msgRcv(msgQueue, &msg, BUFFSIZE, TO_SERVER, 0, &read)){
            //if child, server functions as a slave
            if(!fork()){
                //who for, how fast and what
                sscanf(msg.mtext, "%d %d %s", &clientPid, &clientPriority, file);

                ifstream inFile(file, ios::binary);

                //could we open the file
                if(inFile.fail()){
                    msg.mtype = QUIT_CLIENT(clientPid);
                    *msg.mtext = FILE_NOT_FOUND;
                    msgSnd(msgQueue, &msg, 1);
                    exit(1);
                }

                if(verbose){
                    printf("}file '%s' for process %d with priority %d\n", file, clientPid, clientPriority);
                    printf(">%.*s\n", read, msg.mtext);
                }
                //set it to the client to recv it
                msg.mtype = clientPid;
                int running = 1;
                //main loop for slave servers to handle reading and sending of messages to clients
                while(running) {
                    //higher priority means more loops before sync with other slave servers
                    //this will result in it finishing priority times faster than a lower one
                    for(int i = 0; i < clientPriority; ++i) {
                        inFile.read(msg.mtext, BUFFSIZE);
                        read = inFile.gcount();
                        msgSnd(msgQueue, &msg, read);
                        if(verbose)
                            printf("<%d:[%d/%d]\n", clientPid, i + 1, clientPriority);
                        if(inFile.eof()) {
                            if(verbose)
                                printf("<%d:[complete]\n", clientPid);
                            running = 0;
                            break;
                        }
                    }
                    if(!running)
                        break;
                    //TODO wait for server to give go ahead
                    //for now just continue until the file is fully transmitted
                }

                msg.mtype = QUIT_CLIENT(clientPid);
                *msg.mtext = FILE_END;
                msgSnd(msgQueue, &msg, 1);
                //exit here so it doesnt delete the message queue by returning to main
                exit(0);
            }
        }
    }
    //when the master process exits clean up.
    msgCtl(msgQueue, IPC_RMID, 0);
}

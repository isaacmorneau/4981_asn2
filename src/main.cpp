#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "headers/msgwrappers.h"
#include "headers/server.h"
#include "headers/client.h"

using namespace std;

int main(int argc, char *argv[]){

    int msqId;
    //default to invalid as key must be specified
    key_t mkey = -1;
    //default is silent mode
    int verbose = 0;
    //verify only one is specified
    int isClient = 0;
    int isServer = 0;
    //a file must be entered if running as client
    string fileRequest;
   
    if(argc == 1){
                printf("usage %s <options> <args>\n"
                        "-s - run as server\n"
                        "-c - run as client\n"
                        "-v - run in verbose mode\n"
                        "-h - this message\n"
                        "-f - the file requested in client mode\n"
                        "-k - the key for the message queue\n"
                        , argv[0]);
                exit(0);
    }

    int opt;
    while ((opt = getopt(argc, argv, "hcsvk:f:")) != -1) {
        switch(opt) {
            case 'c'://client
                if(isServer){
                    perror("cannot be both client and server");
                    exit(1);
                }
                isClient = 1;
                break;
            case 's'://server
                if(isClient){
                    perror("cannot be both client and server");
                    exit(1);
                }
                isServer = 1;
                break;
            case 'f'://file
                fileRequest = string(optarg);
                break;
            case 'k'://key
                mkey = static_cast<key_t>(atoi(optarg));
                if(!mkey){
                    perror("invalid key. key must be a non zero integer");
                    exit(1);
                }
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                printf("usage %s <options> <args>\n"
                        "-s - run as server\n"
                        "-c - run as client\n"
                        "-v - run in verbose mode\n"
                        "-h - this message\n"
                        "-f - the file requested in client mode\n"
                        "-k - the key for the message queue\n"
                        , argv[0]);
                exit(0);
                break;
            case '?':
                //ignore random options i dont know
                break;
        }
    }

    //default to client
    if(!(isClient || isServer)){
        isClient = 1;
    }
    if(isClient && mkey == -1){
        perror("invalid key. key must be a non zero integer");
        exit(1);
    }

    msqId = msgGet(mkey, IPC_CREAT | 0666);

    if(isServer) {
        server(msqId);
    } else {
        client(msqId, fileRequest);
    }

    //msgCtl(msqId, IPC_STAT, &msqStatus);

    // Remove he message queue
    msgCtl(msqId, IPC_RMID);
    exit(0);
}

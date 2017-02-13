#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "headers/wrappers.h"
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
    //priority of the client
    int priority = 0;

    //a file must be entered if running as client
    string inFile, outFile;
   
    if(argc == 1){
                printf("usage %s <options> <args>\n"
                        "-s - run as server\n"
                        "-c - run as client\n"
                        "-v - run in verbose mode\n"
                        "-h - this message\n"
                        "-f - the file requested in client mode\n"
                        "-o - the file to save to in client mode (default stdout)\n"
                        "-k - the key for the message queue\n"
                        "-p - the priority of the client. a positive integer or [l]ow, [m]edium, [h]igh\n"
                        , argv[0]);
                exit(0);
    }

    int opt;
    while ((opt = getopt(argc, argv, "hcsvk:f:o:p:")) != -1) {
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
            case 'f'://file to get
                inFile = string(optarg);
                break;
            case 'o'://file to put to
                outFile = string(optarg);
                break;
            case 'k'://key
                mkey = static_cast<key_t>(atoi(optarg));
                if(!mkey){
                    perror("invalid key. key must be a non zero integer");
                    exit(1);
                }
                break;
            case 'p':
                switch(*optarg){
                    case 'l'://low
                        priority = 1;
                        break;
                    case 'm'://medium
                        priority = 10;
                        break;
                    case 'h'://high
                        priority = 100;
                        break;
                    default://custom
                        priority = atoi(optarg);
                        if(priority < 1){
                            perror("priority must be a positive integer");
                            exit(1);
                        }
                        break;
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
                        "-o - the file to save to in client mode (default stdout)\n"
                        "-k - the key for the message queue\n"
                        "-p - the priority of the client. a positive integer or [l]ow, [m]edium, [h]igh\n"
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

    msqId = msgGet(mkey, IPC_CREAT | 0660);

    if(isServer) {
        server(msqId, mkey, verbose);
    } else {
        client(msqId, inFile, outFile, priority, verbose);
    }

    // Remove he message queue
    //msgCtl(msqId, IPC_RMID, 0);
    exit(0);
}

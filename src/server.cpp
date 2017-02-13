#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sys/signal.h>
#include <vector>
#include <atomic>
#include "headers/server.h"
#include "headers/msgwrappers.h"

#define FILESIZE 256

using namespace std;

//filewide so signal handlers as well as server can use them
//is the master still running?
int sharedRunning = 1;
//how many slaves are ready to continue?
atomic<unsigned int> sharedRcv(0);
//list of all slave pids
vector<int> slaves;

int verbose = 0;

void interuptRequest(int signal){
    switch(signal){
        case SIGINT:
            if(verbose && sharedRunning)
                printf("Caught interupt, exiting.\n");
            sharedRunning = 0;
            break;
        case SIGUSR1:
            if(++sharedRcv >= slaves.size()){
                sharedRcv = 0;
                for(auto i = slaves.begin(); i != slaves.end(); ++i)
                    kill(*i, SIGCONT);
            }
            //check if all servers are done
            if(verbose)
                printf("%d current shared, %d slaves\n", (int)sharedRcv, (int)slaves.size());
            break;
        case SIGUSR2:
            //a server exited, find it and remove it from the queue
            
            break;
    }
}

void server(const int msgQueue, const int verb){
    verbose = verb;
    //message queue struct
    MsgBuff msg;
    //how many bytes were read
    int read;
    //whos asking for a file
    int clientPid;
    //how fast do they want it
    int clientPriority;
    //which file
    char file[BUFFSIZE];
    //who is the master server to send messages to
    int masterPid = getpid();


    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = interuptRequest;
    //catch ^c cancel of server
    sigaction(SIGINT,&sa,0);
    //catch slave servers done messages
    sigaction(SIGUSR1,&sa,0);
    //catch slave servers exited messages
    sigaction(SIGUSR2,&sa,0);



    while(sharedRunning){
        //TODO sync slaver servers and make nonblock
        if(msgRcv(msgQueue, &msg, BUFFSIZE, TO_SERVER, 0, &read)){
            //who for, how fast and what
            sscanf(msg.mtext, "%d %d %s", &clientPid, &clientPriority, file);

            int slaveServerPid;
            if((slaveServerPid = fork())){
                //another slave for the slave pit, hahaha!
                slaves.push_back(slaveServerPid);
            } else {
                //if child, server functions as a slave
                slaveServer(msgQueue, masterPid, file, clientPid, clientPriority);
            }
        }
    }
    //when the master process exits clean up.
    msgCtl(msgQueue, IPC_RMID, 0);
}

void slaveServer(const int msgQueue, 
        const int parentPid,
        char const file[],
        const int clientPid,
        const int clientPriority) {
    
    if(verbose)
        printf("}file '%s' for process %d with priority %d\n", file, clientPid, clientPriority);

    //message queue struct
    MsgBuff msg;

    //try and open file
    ifstream inFile(file, ios::binary);

    //could we open the file
    if(inFile.fail()){
        if(verbose)
            printf("}file '%s' not found.\n", file);
        msg.mtype = QUIT_CLIENT(clientPid);
        *msg.mtext = FILE_NOT_FOUND;
        msgSnd(msgQueue, &msg, 1);
        //dont exit with an error but do close this fork
        exit(0);
    }
    
    //how many bytes were read
    int read;
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

        if(verbose)
            printf("<%d finished loop signaling parent\n",clientPid);
        //notify server that we have completed an itteration
        kill(parentPid, SIGUSR1);
        //wait for the server to unblock via SIGCONT
        //cant use pause as it doenst respond unless proc was stopped by SIGSTOP
        kill(getpid(), SIGSTOP);
    }

    msg.mtype = QUIT_CLIENT(clientPid);
    *msg.mtext = FILE_END;
    msgSnd(msgQueue, &msg, 1);
    //exit here so it doesnt delete the message queue by returning to main
    exit(0);
}

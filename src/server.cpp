#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sys/signal.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <atomic>
#include "headers/server.h"
#include "headers/wrappers.h"

using namespace std;

//filewide so signal handlers as well as server can use them
//is the master still running?
int sharedRunning = 1;
//how many slaves are there
int slaves = 0;
//how many are ready to execute
atomic<int> slavesReady(0);

//to mannage locking and unlocking of slave servers
//and to allow interupt to clean up resources
//semaphore id
int semId;
int msgId;
//do we say whats happening or not
int verbose = 0;

void interuptRequest(int signal){
    switch(signal){
        case SIGINT:
            if(!sharedRunning)
                return;
            if(verbose)
                printf("\n}Caught interupt, exiting.\n");
            sharedRunning = 0;
            //when the master process exits clean up.
            semRelease(semId);
            msgRelease(msgId);
            exit(0);
            break;
        case SIGUSR2:
            //decrement total semaphores
            --slaves;
            break;
        case SIGUSR1:
            //a client says they are waiting
            ++slavesReady;
            //make sure someone is actually waiting
            //we should never be off but if its too fast, spinlock until the slave waits
            while(!semGetWaiting(semId));

            //everyone is accounted for, proceed to unlock
            if(slavesReady == slaves){
                slavesReady = 0;
                for(int i = 0; i < slaves; ++i)
                    semSignal(semId);//be free!
                if(verbose)
                    printf("}released %d slaves\n", slaves);
            }
            break;
    }
}

void server(const int msgQueue, const key_t mkey, const int verb){
    verbose = verb;
    //message queue struct
    MsgBuff msgBuff;
    //how many bytes were read
    int read;
    //whos asking for a file
    int clientPid;
    //how fast do they want it
    int clientPriority;
    //who is the master server to send messages to
    int masterPid = getpid();


    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = interuptRequest;
    //catch ^c cancel of server
    sigaction(SIGINT, &sa, 0);
    //catch slave servers done messages
    sigaction(SIGUSR1, &sa, 0);
    //catch slave finished messages
    sigaction(SIGUSR2, &sa, 0);


    semId = semGet(mkey, 1);
    //the queue is created in main so the client can share its creation
    //so instead of refetching just assign the id
    msgId = msgQueue;

    while(sharedRunning){
        //start off blocking but as soon as there are slave processes be sure to check on them
        if(msgRcv(msgId, &msgBuff, BUFFSIZE, TO_SERVER, 0, &read)){

            if(!fork()){
                //which file
                char file[BUFFSIZE];
                memset(file,0,BUFFSIZE);
                //who for, how fast and what
                sscanf(msgBuff.mtext, "%d %d %s", &clientPid, &clientPriority, file);

                //if child, server functions as a slave
                slaveServer(masterPid, file, clientPid, clientPriority);
                //exit here so it doesnt delete the message queue by returning to main

                exit(0);
            } else {
                ++slaves;
            }
        }
    }
}


void slaveServer(const int parentPid,
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
        msgSnd(msgId, &msg, 1);
        kill(parentPid, SIGUSR2);
        //dont exit with an error but do close this fork
        return;
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
            msgSnd(msgId, &msg, read);
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
            printf("<%d:[%d]\n", clientPid, clientPriority);

        //notify master server that we have completed an itteration
        kill(parentPid, SIGUSR1);
        //wait for the server to unblock semaphore
        semWait(semId);
    }

    msg.mtype = QUIT_CLIENT(clientPid);
    *msg.mtext = FILE_END;
    msgSnd(msgId, &msg, 1);

    kill(parentPid, SIGUSR2);
}

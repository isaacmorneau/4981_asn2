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
//semaphore id
int sem;
//do we say whats happening or not
int verbose = 0;

void interuptRequest(int signal){
    switch(signal){
        case SIGINT:
            if(verbose && sharedRunning)
                printf("\n}Caught interupt, exiting.\n");
            printf("}semaphore %d\n}slaves %d\n}ready %d\n", semGetValue(sem),slaves, int(slavesReady));
            sharedRunning = 0;
            break;
        case SIGUSR2:
            //decrement total semaphores
            --slaves;
        case SIGUSR1://check when a client says they are waiting
            if(++slavesReady >= slaves){
                slavesReady = 0;
                for(int i = 0; i < slaves; ++i)
                    semSignal(sem);//be free!
                if(verbose)
                    printf("}resumed %d slaves\n", slaves);
            }
            if(slavesReady == 1 && semGetValue(sem) == 0)
                semSignal(sem);//miss sync from client exit
            break;
    }
}

void server(const int msgQueue,const key_t mkey, const int verb){
    verbose = verb;
    //message queue struct
    MsgBuff msg;
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


    sem = semGet(mkey, 1);

    while(sharedRunning){
        //start off blocking but as soon as there are slave processes be sure to check on them
        if(msgRcv(msgQueue, &msg, BUFFSIZE, TO_SERVER, 0, &read)){

            if(!fork()){
                //which file
                char file[BUFFSIZE];
                memset(file,0,BUFFSIZE);
                //who for, how fast and what
                sscanf(msg.mtext, "%d %d %s", &clientPid, &clientPriority, file);
                
                //if child, server functions as a slave
                slaveServer(msgQueue, masterPid, file, clientPid, clientPriority);
                //exit here so it doesnt delete the message queue by returning to main
                
                exit(0);
            } else {
                ++slaves;
            }
        }
    }
    //when the master process exits clean up.
    semRelease(sem);
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
            printf("<%d wait for parent\n",clientPid);
        
        //notify server that we have completed an itteration
        kill(parentPid, SIGUSR1);
        //wait for the server to unblock via semPost
        semWait(sem);

        if(verbose)
            printf("<%d go ahead\n",clientPid);
    }

    msg.mtype = QUIT_CLIENT(clientPid);
    *msg.mtext = FILE_END;
    msgSnd(msgQueue, &msg, 1);

    kill(parentPid, SIGUSR2);
}

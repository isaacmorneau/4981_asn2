#include <string>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#include "headers/client.h"
#include "headers/wrappers.h"

using namespace std;

void client(const int msgQueue, 
        const std::string& reqFile, 
        const std::string& outFile, 
        const int priority, 
        const int verbose) {
    //the buffer for sending the messages
    MsgBuff msg;
    //where should i write to
    ofstream oFile;
    //who am i
    int pid = getpid();
    //how many was read in
    int read = 0;
    //checking the msgrcv return
    int msgret = 0;

    //was a file specified or should we output to stdout
    int useFile = outFile.size();
    if(useFile)
        oFile.open(outFile.c_str(), ios::binary | ios::app);
    if(oFile.fail()){
        perror("failed to open output file");
        exit(2);
    }

    msg.mtype = TO_SERVER;

    sprintf(msg.mtext, "%d %d %s", pid, priority, reqFile.c_str());

    //send the length of the string + 1 for the null char
    msgSnd(msgQueue, &msg, strlen(msg.mtext) + 1);
    //load up the message loop into a lambda and throw it into a thread
    thread([&]{
            while(msgret != -1){
                //while its sending us messages read them in without checking for control
                while((msgret = msgRcv(msgQueue, &msg, BUFFSIZE, pid, IPC_NOWAIT, &read)) == 1) {
                    //write data to output
                    if(useFile) {
                        oFile.write(msg.mtext, read);
                        if(oFile.fail()){
                            oFile.close();
                            if(verbose)
                                printf("write failed, exiting...");
                            exit(3);
                        }
                    } else {
                        //use fwrite because it may be a binary file
                        fwrite(msg.mtext, sizeof(char), read, stdout);
                    }
                }
                //the queue doesnt have any more messages for us for now
                //check to see if the server is needing to tell is something
                if((msgret = msgRcv(msgQueue, &msg, BUFFSIZE, QUIT_CLIENT(pid), IPC_NOWAIT, &read)) == 1){
                    switch(*msg.mtext){
                        case FILE_NOT_FOUND:
                            if(verbose)
                                printf("recieved 'File Not Found' from server, exiting...\n");
                            break;
                        case FILE_END:
                            if(verbose)
                                printf("recieved 'End Of File' from server, exiting...\n");
                            break;
                        case INTERUPT_QUIT:
                            if(verbose)
                                printf("recieved 'Interupt Quit' from server, exiting...\n");
                            break;
                    }
                    oFile.close();
                    break;
                }
            }
            if(msgret == -1 && verbose)
                printf("resource in use removed, exiting...\n");
        }
    ).join();
    //no nead to clean up the message queue, the server will
}

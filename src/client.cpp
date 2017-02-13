#include <string>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#include "headers/client.h"
#include "headers/msgwrappers.h"

using namespace std;

void client(const int msgQueue, 
        const std::string& reqFile, 
        const std::string& outFile, 
        const int priority, 
        const int verbose) {
    MsgBuff msg;
    int read;
    int pid = getpid();
    ofstream oFile;
    //was a file specified or should we output to terminal?
    int useFile = outFile.size();
    if(useFile)
        oFile.open(outFile.c_str(), ios::binary | ios::app);

    msg.mtype = TO_SERVER;

    sprintf(msg.mtext, "%d %d %s", pid, priority, reqFile.c_str());

    msgSnd(msgQueue, &msg, strlen(msg.mtext), IPC_NOWAIT);
    //load up the message loop into a lambda and throw it into a thread
    thread msgWorker([&]{
            while(1){
                //while its sending us messages read them in
                while(msgRcv(msgQueue, &msg, BUFFSIZE, pid, IPC_NOWAIT, &read)) {
                    //write data to output
                    if(useFile)
                        oFile.write(msg.mtext, read);
                    else
                        //use fwrite because it may be a binary file
                        fwrite(msg.mtext, sizeof(char), read, stdout);
                }
                //thats the last message on the queue
                //non blocking check if the server is done with us
                if(msgRcv(msgQueue, &msg, BUFFSIZE, QUIT_CLIENT(pid), IPC_NOWAIT, &read)){
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
        }
    );
    msgWorker.join();
}

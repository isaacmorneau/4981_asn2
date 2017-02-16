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

/**
 *
 * Function: main
 *
 *
 * Date: 2017/02/07
 *
 *
 * Designer: Isaac Morneau; A00958405
 *
 *
 * Programmer: Isaac Morneau; A00958405
 *
 *
 * Interface:
 *      int main(
 *          int argc,       - the number of parameters passed to the program
 *          char *argv[]    - the parameters themselves
 *          );
 *
 * Return: int the status that the program is exiting with
 *
 *
 * Notes: This function handles the initialization of both the client and server parameters. Its primary purpos is 
 *      to verify the parameters used for both client and server. Using getopts it iterates through the parameters
 *      checking how the user wants to create either the client or the server and passing them to the right function
 *      
 *      if no parameters are specified it will print out the usage page
 *      if -h is specified it will print out the usage page
 *
 *      The server can receive the following parameters
 *          -s      [required] to run in server mode
 *          -k arg  [required] the key to use for the message queue
 *          -v      [optional] the server will print out information on what their doing
 *      example:
 *          $asn2 -s -k 1234 -v
 *      
 *      The client can receive the following parameters
 *          -c      [required] to run as a client
 *          -k arg  [required] the key to use for the message queue
 *          -f arg  [required] the file to be requested
 *          -p arg  [required] the priority to request. predefined constants are l, m, and h for low medium and high
 *                              the priority can also be any positive integer if a greater range is desired.
 *          -o arg  [optional] the file to write the response to. If not specified the response will be echoed to
 *                              standard out so that it can be redirected or piped.
 *          -v      [optional] the client will print out information on what its doing
 *
 *      example:
 *          $asn2 -c -k 1234 -v -f /path/to/file -o /path/to/destination -p m
 *
 */
int main(int argc, char *argv[]){

    int msqId;
    //default to invalid as key must be specified
    key_t mkey = -1;
    //default is silent mode
    int verbose = 0;
    //verify only one is specified
    int isClient = 0;
    int isServer = 0;
    //priority of the client default to invalid but only check if its a client
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
    if(isClient){
        if(mkey == -1){
            perror("invalid key. key must be a non zero integer");
            exit(1);
        }
        if(!priority){
            perror("priority must be specified");
            exit(1);
        }
    }

    msqId = msgGet(mkey, IPC_CREAT | 0660);

    if(isServer) {
        server(msqId, mkey, verbose);
    } else {
        client(msqId, inFile, outFile, priority, verbose);
    }

    // Remove he message queue
    exit(0);
}

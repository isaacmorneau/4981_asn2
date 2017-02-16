#ifndef MSGWRAPPERS_H
#define MSGWRAPPERS_H
#include <sys/msg.h>

/**
 * Program: 4981 Assignment 2
 * 
 *
 * Date: 2017/02/10
 *
 *
 * Source File: wrappers.c
 *
 *
 * Designer: Isaac Morneau; A00958405
 * 
 *
 * Functions:
 *      int msgGet(key_t mkey, int flags);
 *      
 *      void msgCtl(int msq, int flags, struct msqid_ds *msgStatus = 0);
 *      
 *      void msgSnd(int msq, const MsgBuff *msgbuff, int size, int flags = 0);
 *      
 *      int msgRcv(int msq, MsgBuff *msgbuff, int size, long type, int flags, int *read);
 *      
 *      void msgRelease(int msq);
 *      
 *      int semGet(key_t key, int value);
 *      
 *      void semSet(int sid, int value);
 *      
 *      void semWait(int sid);
 *      
 *      void semSignal(int sid, int value = 1);
 *      
 *      void semRelease(int sid);
 *      
 *      int semGetValue(int sid);
 *      
 *      int semGetWaiting(int sid);
 *
 *
 * Notes: This file holds all the wrappers for all my system calls to clean up the code as a whole. Each of these
 *      functions have built in error checking so you don't need to remember to when using them. Specifically the
 *      first half of the messages are only for dealing with message queues while the second is only for dealing
 *      with semaphores. 
 *
 *      For implementation, internally all wrappers implement SysV calls.
 *      
 */

//the buffer size for all the messages
#define BUFFSIZE 1024
//the mtype of messages from client to server
#define TO_SERVER 1
//generate a new unlikely id from the client pid
//while this could have collisions it would only do so after 2^8 simultaneous client connections or
//if there were so many processes running on the machine that a bitshift left would cause the number to trucate
//several of the least significant bits resulting in a small enough collision space ie 2^2 that there could be
//that many simultaneous processes at once.
#define QUIT_CLIENT(x) ((x) << 8) 
//signaling constants for server to client control messages
#define FILE_NOT_FOUND 'N'// we could not aquire the file
#define FILE_END 'E' // the file is finished sending
#define INTERUPT_QUIT 'I' //the server encountered a condition that requires imediate termination



/**
 *
 * Struct: MsgBuff
 * 
 *
 * Members:
 *      long mtype              - the type that will be looked for when reading
 *      char mtext[BUFFSIZE]    - the actual message data to send or receive
 *
 *
 * Notes: For use with msgSnd and msgRcv to hold and receive the data to and from
 *
 */
struct MsgBuff {
    long mtype;
    char mtext[BUFFSIZE];
};


/**
 *
 * Function: msgGet
 *
 *
 * Date: 2017/02/10
 *
 *
 * Designer: Isaac Morneau; A00958405
 *
 *
 * Programmer: Isaac Morneau; A00958405
 *
 *
 * Interface:
 *      int msgGet(
 *          key_t mkey,     - The key from which to create the message queue from
 *          int flags       - The flags for the permissions to create the message queue
 *          );
 * 
 *
 * Return: 
 *      int         - the address of the message queue for use with the rest of the handlers
 *
 *
 * Notes: create a message queue out of the key mkey and return its address.
 *
 */
int msgGet(key_t mkey, int flags);

/**
 *
 * Function: msgCtl
 *
 *
 * Date: 2017/02/10
 *
 *
 * Designer: Isaac Morneau; A00958405
 *
 *
 * Programmer: Isaac Morneau; A00958405
 *
 *
 * Interface:
 *      void msgCtl(
 *          int msg,                    - The msg queue to operate on
 *          int flags,                  - The flags for what operation to perform
 *          struct msgid_ds *msgStatus  - [optional] if the operation requires a return structure it can be passe
 *                                          in, if its not specified, it defaults to 0
 *          );
 * 
 *
 * Return: void
 *
 *
 * Notes: This is a general purpos wrapper for when msgctl is required. It just passes through arguments and
 *      catches errors if any occur.
 *
 */
void msgCtl(int msq, int flags, struct msqid_ds *msgStatus = 0);

/**
 *
 * Function: msgSnd
 *
 *
 * Date: 2017/02/10
 *
 *
 * Designer: Isaac Morneau; A00958405
 *
 *
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void msgSnd(
 *          int msg,                    - The msg queue to operate on
 *          const MsgBuff *msgbuff      - the buffer contain a type and a char array for the text
 *          int size,                   - The size of the char array in the msgbuff
 *          int flags,                  - [optional] The flags for what operation to perform
 *                                          if not provided it defaults to 0
 *          );
 * 
 *
 * Return: void
 *
 * 
 * Notes: This is a wrapper to send messages to the message queue. By default it is a blocking call while if
 *      IPC_NOWAIT is specified and it actually fails it will do so silent.
 *
 */
void msgSnd(int msq, const MsgBuff *msgbuff, int size, int flags = 0);

/**
 *
 * Function: msgRcv
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void msgRcv(
 *          int msg,                    - The msg queue to operate on
 *          struct msgid_ds *msgStatus  - [optional] if the operation requires a return structure it can be passe
 *                                          in, if its not specified, it defaults to 0
 *          int size,                   - The size of the buffer that will be read into
 *          long type,                  - The type of the message to read
 *          int flags,                  - The flags for how to read
 *          int *read,                  - The int pointer of how many bytes are read in if the operation suceeds
 *          );
 * 
 *
 * Return: int      - indicates if the operation succeeds 1 or if the operation fails 0 if the resource was
 *                      removed use -1 for this special error so you can notify the user
 *
 * 
 * Notes: this wapper differs from the original wrapper as it has an extra parameter which is where the value
 *          will be set into read while the return is its success or failure.
 *
 */
int msgRcv(int msq, MsgBuff *msgbuff, int size, long type, int flags, int *read);

/**
 *
 * Function: msgRelease
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void msgRelease(
 *          int msg,        - The msg queue to operate on
 *          );
 * 
 * Return: void
 *
 * 
 * Notes: Clean up an existing message queue that was allocated by msgGet. Internally it wraps msgctl for removal.
 *
 */
void msgRelease(int msq);

/**
 *
 * Function: semGet
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      int semGet(
 *              key_t key,      - The key from which to create the semaphore for.
 *              int value       - The number of semaphores to create in the array
 *          );
 * 
 *
 * Return: int
 *
 * 
 * Notes: Wraps creation of semaphores while only requiring key and value
 *
 */
int semGet(key_t key, int value);

/**
 *
 * Function: semSet
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void semSet(
 *          int sid,        - The semaphore to operates on
 *          int value,      - The value to set the semaphore to (must be positive)
 *          );
 * 
 * Return: void
 *
 * 
 * Notes: wrapper for semctl to set the value of the semaphore withouot the extra parameters
 *
 */
void semSet(int sid, int value);

/**
 *
 * Function: semWait
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void semWait(
 *          int sid,        - The semaphore to operates on
 *          );
 * 
 *
 * Return: void
 *
 * 
 * Notes: decrements the value of the semaphore and waits if the semaphores at zero
 *          intended intitially to only wait as the semaphore will always be zero
 *
 */
void semWait(int sid);

/**
 *
 * Function: semSignal
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void semSignal(
 *          int sid,        - The semaphore to operates on
 *          );
 * 
 * Return: void
 *
 * 
 * Notes: increments the semaphore by the amount of value
 *
 */
void semSignal(int sid, int value = 1);

/**
 *
 * Function: semRelease
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void semRelease(
 *          int sid,        - The semaphore to operates on
 *          );
 * 
 *
 * Return: void
 *
 * 
 * Notes: destroy the semaphore at sid, wraps semctl for removing it
 *
 */
void semRelease(int sid);

/**
 *
 * Function: semGetValue
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      int semGetValue(
 *          int sid,        - The semaphore to operates on
 *          );
 * 
 *
 * Return: int the current value of the semaphore sid
 *
 * 
 * Notes: internally queries the value of the semaphore and returns it. This is its value not how many are waiting
 *
 */
int semGetValue(int sid);

/**
 *
 * Function: semGetWaiting
 *
 * 
 * Date: 2017/02/10
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      int semGetWaiting(
 *          int sid,        - The semaphore to operates on
 *          );
 * 
 *
 * Return: int the current number of waiting processes. This is how many are waiting not the semaphores value.
 *
 * 
 * Notes: internally queries the number of waiting processes and returns it.
 *
 */
int semGetWaiting(int sid);

#endif

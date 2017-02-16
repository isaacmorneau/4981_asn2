#ifndef SERVER_H
#define SERVER_H

/**
 * Program: 4981 Assignment 2
 *
 * 
 * Date: 2017/02/08
 *
 * 
 * Source File: server.cpp
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Functions:
 *      void server(const int msgQueue, const key_t mkey, const int verbose);
 *
 *      void slaveServer(const int parentPid, char const file[],
 *          const int clientPid, const int clientPriority);
 *
 * 
 * Notes: This file is used to encapsulate all of the server side operations.
 *
 */


/**
 *
 * Function: server
 *
 * 
 * Date: 2017/02/08
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void server(
 *          const int msgQueue,         - the int address of the message queue to read from
 *          const key_t mkey            - the key from which to create the message queue and semaphore from
 *          const int verbose           - if verbose is non zero, status messages will be print
 *          );
 *
 * 
 * Return: void
 *
 * 
 * Notes: The master (this) server initializes a semaphore using the mkey and then listens blocking for messages 
 *      with the mtype TO_SERVER [wrapper.h]. When a client makes a request the server forks and the child then
 *      runs slaveServer. After there are slave servers the master server switches to nonblocking so that it can
 *      also manage syncronization. It checks first if there are any new client requests. If so it forks again.
 *      It also checks for how many of the slave servers are waiting and if all slaves are waiting it unlocks the 
 *      semaphore allowing all slaves to continue. This repeats until all slaves complete and terminate in which
 *      case it returns to blocking while waiting for new connections.
 *
 */
void server(const int msgQueue, const key_t mkey, const int verbose);



/**
 *
 * Function: slaveServer
 *
 * 
 * Date: 2017/02/08
 *
 * 
 * Designer: Isaac Morneau; A00958405
 *
 * 
 * Programmer: Isaac Morneau; A00958405
 *
 * 
 * Interface:
 *      void slaveServer(
 *          const int parentPid,        - the id of the parent for use in signals to indicate completion
 *          char const file[],          - the c string of the file that the client just requested
 *          const int clientPid,        - the pid of the client for setting the mtype
 *          const int clientPriority    - the priority that the client wants the messages at
 *          );
 *
 * 
 * Return: void
 *
 * 
 * Notes: The slave server is created when the master server receives a request from a client and forks. It then
 *      opens up the file and starts sending the messages to the client in chunks of BUFFSIZE [wrapper.h]. Each
 *      slave will send the number of file chunks as the client requested in priority and then wait for the master
 *      server to signal the slaves they can continue. This means that per 'step' of the master server each slave
 *      will send its client priority number of file chunks to it.
 *      This send amount and wait design ensures that per 'step' each slave will conform to the relative amount
 *      to send to the client.
 *
 *      The way that the blocking is handled between the master and the slave is that per step the client will 
 *      only be limited to the speed of its relative priority of the highest current client priority.
 *      This means that a single client of priority 1 will run at aproximately the same speed as a single client
 *      of priority 10. This only changes with multiple clients. For example if both the priority 1 and 10 ran at
 *      the same time then the one with priority 10 would run 10 times as often as the priority 1. This scales to
 *      n clients with m priorities.
 *
 */
void slaveServer(const int parentPid, char const file[], const int clientPid, const int clientPriority);

#endif

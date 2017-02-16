#ifndef CLIENT_H
#define CLIENT_H
#include <string>
/**
 * Program: 4981 Assignment 2
 *
 *
 * Date: 2017/02/08
 *
 *
 * Source File: client.cpp
 *
 *
 * Functions:
 *      void client(const int msgQueue, const std::string& reqFile,
 *              const std::string& outFile, const int priority, const int verbose);
 *
 *
 * Notes: This file is used to encapsulate all of the client side operations.
 *
 */


/**
 *
 * Function: client
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
 *      void client(
 *          const int msgQueue,         - the int address of the message queue to read from
 *          const std::string& reqFile, - the path to the file that is being requested
 *          const std::string& outFile, - [optional] if specified, the path to the file to write to
 *                                          if it is empty, default to output to standard output
 *          const int priority,         - the priority (how fast) to request the file from the server is
 *                                          it must be a positive non zero integer
 *          const int verbose           - if verbose is non zero, status messages will be print
 *          );
 *
 * Return: void
 *
 *
 * Notes: The client sends a message to the server by setting the mtype to TO_SERVER [wrapper.h] and putting
 *      int its pid, priority, and file requested. It then listens to the message queue for any file parts
 *      which will have the mtype of its pid as well as for control messages which will have the mtype of
 *      QUIT_CLIENT(pid) [wrapper.h].
 *      The control messages are either FILE_NOT_FOUND, FILE_END, or 
 *      INTERUPT_QUIT which respectively inform the client that the file cannot be found, the file is finished
 *      or the server has shutdown.
 *      The client is a dumb listener. Once it has requested its information it simply reads all messages
 *      tagged to it and outputs them to standard out or a file if one was specified.
 *
 */
void client(const int msgQueue, const std::string& reqFile, const std::string& outFile,
        const int priority, const int verbose);

#endif

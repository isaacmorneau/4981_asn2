#ifndef SERVER_H
#define SERVER_H

void server(const int msgQueue, const key_t mkey, const int verbose);
void slaveServer(const int parentPid, char const file[], const int clientPid, const int clientPriority);

#endif

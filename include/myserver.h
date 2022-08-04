#ifndef MYSERVER_H
#define MYSERVER_H

#include "mycommon.h"

typedef DWORD (*HANDLER_PTR)(LPVOID);

typedef struct ConnectionInfo
{
	LPHANDLE pool;
	int index;
	SOCKET client;
} CONNECTION_INFO;

int server_init(BOOL *runFlag, HANDLER_PTR clientHandler);

#endif

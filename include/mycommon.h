#ifndef MYCOMMON_H
#define MYCOMMON_H

#ifndef UNICODE
#define UNICODE 1
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#endif

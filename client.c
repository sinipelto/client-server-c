#include "myclient.h"

int client_main()
{
	WSADATA wsaData;
	int wsaResult;

	wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (wsaResult != 0)
	{
		fprintf(stderr, "ERROR: Failed to execute WSAStartup: %d", wsaResult);
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	SOCKET SERVER_SOCKET = INVALID_SOCKET;
	SERVER_SOCKET = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	fprintf(stdout, "Got socket: %d\r\n", SERVER_SOCKET);

	if (SERVER_SOCKET == INVALID_SOCKET)
	{
		fprintf(stderr, "ERROR: Failed to create socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Socket create ok.\r\n");

	wsaResult = WSACleanup();

	if (wsaResult != 0)
	{
		fprintf(stderr, "ERROR: Failed to execute WSACleanup: %d", wsaResult);
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

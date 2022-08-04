#include "myserver.h"

BOOL RUNNING = true;

// Handle interrupts in the main thread (here)
// To gracefully stop the client and the server
BOOL WINAPI signal_handler(DWORD signal)
{
	switch (signal)
	{
	case CTRL_C_EVENT:
		fprintf(stdout, "Caught Ctrl+C, stopping server..\r\n");
		RUNNING = FALSE;
		return FALSE;
	}
}

// The custom method how we want to handle incoming clients
// Should be passed as param to the server init
DWORD WINAPI process_client(LPVOID args)
{
	CONNECTION_INFO *info = (CONNECTION_INFO *)args;
	const size_t bufLen = 128;
	char *recvBuf = (char *)calloc(bufLen, sizeof(char));
	char *msg = (char *)calloc(bufLen, sizeof(char));

	int fails = 0;
	while (RUNNING)
	{
		if (fails > 3)
		{
			fprintf(stderr, "ERROR: Client %d: Error threshold reached. Abort connection.\r\n", info->client);
			break;
		}

		int recvResult = recv(info->client, recvBuf, bufLen, 0);
		if (recvResult > 0)
		{
			// TODO parse as protocol package struct
			// PACKAGE package = parse(recvBuf, len);
			strncpy(msg, recvBuf, (size_t)recvResult);
			// msg[recvResult] = '\0'; // Not needed
			fprintf(stdout, "Client %d: Received: %s\r\n", info->client, msg);
		}
		else if (recvResult == 0)
		{
			fprintf(stderr, "WARNING: Client %d: Disconnected.\r\n", info->client);
			break;
		}
		else
		{
			fprintf(stderr, "ERROR: Client %d: Failed to receive message from client.\r\n", info->client);
			fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
			Sleep((DWORD)1000UL);
			fails += 1;
			continue;
		}

		// TODO parse message
		if (strcmp("HELLO_SERVER", msg) == 0)
		{
			if (send(info->client, "HELLO_CLIENT\r\n", 13, 0) == SOCKET_ERROR)
			{
				fprintf(stderr, "Client %d: Failed to send a resopnse.\r\n", info->client);
				fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
				Sleep((DWORD)1000UL);
				fails += 1;
				continue;
			}
			else
			{
				fprintf(stdout, "Client %d: Response sent ok.\r\n", info->client);
				break;
			}
		}
		else
		{
			if (send(info->client, "UNKNOWN_REQUEST\r\n", 18, 0) == SOCKET_ERROR)
			{
				fprintf(stderr, "Client %d: Failed to send a resopnse.\r\n", info->client);
				fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
				Sleep((DWORD)1000UL);
				fails += 1;
				continue;
			}
			else
			{
				fprintf(stdout, "Client %d: Response sent ok.\r\n", info->client);
				break;
			}
		}
	}

	fprintf(stdout, "Client %d: Client thread terminated.\r\n", info->client);

	if (closesocket(info->client) == SOCKET_ERROR)
	{
		fprintf(stderr, "ERROR: Failed to close client socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
	}

	// Set handle reference to null before terminating thread
	// This signals the main thread it can re-use the connection slot on pool
	free((LPVOID)msg);
	free((LPVOID)recvBuf);
	*(info->pool + info->index) = NULL;
	free((LPVOID)info);
}

int main(int argc, char const **argv)
{
	int res = server_init(&RUNNING, &process_client);

	if (res != EXIT_SUCCESS)
	{
		fprintf(stdout, "ERROR: Failed to init server. Code %d\r\n", res);
		return res;
	}

	fprintf(stdout, "Application running...\r\n");
	while (RUNNING)
	{
		// TODO create client, make requests
		// fprintf(stdout, "Main: Sleeping..\r\n");
		Sleep((DWORD)1UL);
	}
}

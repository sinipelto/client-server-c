#include "myserver.h"

#define MAX_CONNECTIONS 3

// Private struct for server main thread args
typedef struct MainArgs
{
	HANDLE *connectionPool; // connectionPool[MAX_CONN]
	HANDLER_PTR clientHandler;
	SOCKET serverSocket;
	SOCKADDR_IN *serverHost;
	BOOL *runFlag;
} MAIN_ARGS;

const char SERVER_ADDR[] = "127.0.0.1";
const USHORT SERVER_PORT = 9123;

int wsa_cleanup(void)
{
	int wsaResult = WSACleanup();

	if (wsaResult != 0)
	{
		fprintf(stderr, "ERROR: Failed to execute WSACleanup: %d\r\n", wsaResult);
		fprintf(stderr, "WSA WSA Error Code: %d\r\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	fprintf(stdout, "WSA Cleanup ok.\r\n");

	return EXIT_SUCCESS;
}

DWORD WINAPI main_thread(LPVOID arg)
{
	MAIN_ARGS *args = (MAIN_ARGS *)arg;

	BOOL *runFlag = args->runFlag;
	HANDLE *connectionPool = args->connectionPool;
	SOCKET serverSocket = args->serverSocket;
	SOCKADDR_IN *serverHost = args->serverHost;
	HANDLER_PTR clientHandler = args->clientHandler;

	BOOL warned = false;
	while (runFlag != NULL && *runFlag)
	{
		size_t currentHandle = 0;

		for (; currentHandle < MAX_CONNECTIONS; currentHandle++)
		{
			if (connectionPool[currentHandle] == NULL)
			{
				fprintf(stdout, "Found free slot: %d\r\n", currentHandle);
				break;
			}
		}

		if (currentHandle >= MAX_CONNECTIONS)
		{
			if (!warned)
			{
				fprintf(stderr, "ERROR: Limit reached - Connection pool is full. Cannot handle more clients. Waiting for free handles..\r\n");
				warned = true;
			}
			Sleep((DWORD)1UL);
			continue;
		}

		warned = false;

		CONNECTION_INFO *info = (CONNECTION_INFO *)calloc(1, sizeof(CONNECTION_INFO));
		info->pool = connectionPool;
		info->index = currentHandle;

		fprintf(stdout, "Waiting for incoming connections..\r\n");
		SOCKET client = accept(serverSocket, NULL, NULL);
		fprintf(stdout, "Client (%d) connected.\r\n", client);
		info->client = client;

		connectionPool[currentHandle] = CreateThread(NULL, (SIZE_T)0, (LPTHREAD_START_ROUTINE)clientHandler, (LPVOID)info, (DWORD)0, NULL);
		fprintf(stdout, "Client handler called.\r\n");
	}

	fprintf(stdout, "Waiting for clients to disconnect..\r\n");
	WaitForMultipleObjects(MAX_CONNECTIONS, connectionPool, true, INFINITE);

	fprintf(stdout, "All clients disconnected.\r\n");

	if (closesocket(serverSocket) == SOCKET_ERROR)
	{
		fprintf(stderr, "ERROR: Failed to close server socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
	}

	// runFlag, clientHandler not initialized by us!
	free((LPVOID)connectionPool);
	free((LPVOID)serverHost);
	free((LPVOID)args);

	if (wsa_cleanup() != EXIT_SUCCESS)
	{
		fprintf(stderr, "ERROR: Failed to execute cleanup().\r\n");
	}
}

int server_init(BOOL *runFlag, HANDLER_PTR clientHandler)
{
	if (runFlag == NULL || *runFlag == FALSE)
	{
		fprintf(stderr, "ERROR: Run flag not in ready state. Ensure provided initial value as true.\r\n");
		return EXIT_FAILURE;
	}

	// Allocate struct for shared arguments in heap
	// To share info with the main thread
	MAIN_ARGS *mainArgs = (MAIN_ARGS *)calloc(1, sizeof(MAIN_ARGS));
	mainArgs->runFlag = runFlag;
	mainArgs->clientHandler = clientHandler;

	WSADATA wsaData;
	int wsaResult;

	wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (wsaResult != 0)
	{
		fprintf(stderr, "ERROR: Failed to execute WSAStartup: %d\r\n", wsaResult);
		fprintf(stderr, "WSA WSA Error Code: %d\r\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Debug print
	fprintf(stdout, "Got socket: %d\r\n", serverSocket);

	if (serverSocket == INVALID_SOCKET)
	{
		fprintf(stderr, "ERROR: Failed to create socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		wsa_cleanup();
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Socket create ok.\r\n");

	const char optVal = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == SOCKET_ERROR)
	{
		fprintf(stderr, "ERROR: Failed to configure socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		wsa_cleanup();
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Socket setsockopt ok.\r\n");

	LPSOCKADDR_IN serverHost = (LPSOCKADDR_IN)calloc(1, sizeof(SOCKADDR_IN));
	serverHost->sin_family = AF_INET;
	serverHost->sin_addr.S_un.S_addr = inet_addr(SERVER_ADDR);
	serverHost->sin_port = htons(SERVER_PORT);

	if (bind(serverSocket, (const LPSOCKADDR)serverHost, sizeof(*serverHost)) == SOCKET_ERROR)
	{
		fprintf(stderr, "ERROR: Failed to bind socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		wsa_cleanup();
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Socket bind ok.\r\n");

	mainArgs->serverHost = serverHost;

	if (listen(serverSocket, (int)MAX_CONNECTIONS) == SOCKET_ERROR)
	{
		fprintf(stderr, "ERROR: Failed to start listening socket.\r\n");
		fprintf(stderr, "WSA Error Code: %d\r\n", WSAGetLastError());
		wsa_cleanup();
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Socket listen ok.\r\n");

	mainArgs->serverSocket = serverSocket;

	// Init the connection pool
	LPHANDLE pool = calloc(MAX_CONNECTIONS, sizeof(HANDLE));
	mainArgs->connectionPool = pool;

	// Start server main thread
	CreateThread(NULL, (SIZE_T)0, (LPTHREAD_START_ROUTINE)main_thread, (LPVOID)mainArgs, (DWORD)0, NULL);

	return EXIT_SUCCESS;
}

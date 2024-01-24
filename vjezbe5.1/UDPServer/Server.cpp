// UDP server that use non-blocking sockets
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

// Port numbers of server that will be used for communication with clients
#define SERVER_PORT1 15011	
#define SERVER_PORT2 15012

// Size of buffer that will be used for sending and receiving messages to clients
#define BUFFER_SIZE 512		

int main()
{

	sockaddr_in serverAddress1;
	sockaddr_in serverAddress2;

	char dataBuffer[BUFFER_SIZE];

	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	memset((char*)&serverAddress1, 0, sizeof(serverAddress1));
	serverAddress1.sin_family = AF_INET; //set server address protocol family
	serverAddress1.sin_addr.s_addr = INADDR_ANY;
	serverAddress1.sin_port = htons(SERVER_PORT1);

	memset((char*)&serverAddress2, 0, sizeof(serverAddress2));
	serverAddress2.sin_family = AF_INET; //set server address protocol family
	serverAddress2.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress2.sin_port = htons(SERVER_PORT2);

	SOCKET serverSocket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket1 == INVALID_SOCKET) {
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}


	SOCKET serverSocket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket2 == INVALID_SOCKET) {
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	int iResult = bind(serverSocket1, (struct sockaddr*)&serverAddress1, sizeof(serverAddress1));	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error %d\n", WSAGetLastError());
		closesocket(serverSocket1);
		WSACleanup();
		return 1;
	}

	iResult = bind(serverSocket2, (struct sockaddr*)&serverAddress2, sizeof(serverAddress2));	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error %d\n", WSAGetLastError());
		closesocket(serverSocket2);
		WSACleanup();
		return 1;
	}

	unsigned long mode = 1; //non-blocking mode
	if (ioctlsocket(serverSocket1, FIONBIO, &mode) != 0 || (ioctlsocket(serverSocket1, FIONBIO, &mode) != 0)) {
		printf("ioctlsocket failed with error: %ld\n", iResult);
		closesocket(serverSocket1);
		closesocket(serverSocket2);
		WSACleanup();
		return 1;
	}

	printf("Simple UDP server started and waiting client message.\n");

	while (true) {

		sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof(clientAddress));

		memset(dataBuffer, 0, BUFFER_SIZE);

		int sockAddrLen = sizeof(clientAddress);

		fd_set readfds;
		FD_ZERO(&readfds);

		FD_SET(serverSocket1, &readfds);
		FD_SET(serverSocket2, &readfds);

		fd_set exceptionfds;
		FD_ZERO(&exceptionfds);

		FD_SET(serverSocket1, &exceptionfds);
		FD_SET(serverSocket2, &exceptionfds);

		int result = select(0, &readfds, NULL, &exceptionfds, NULL);

		if (result == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
			printf("select failed with errpr: %d\n", WSAGetLastError());
			break;
		}
		else if(result > 0){
			unsigned long portNumber = 0;

			if (FD_ISSET(serverSocket1, &readfds)) {
				// izvrsenje operacije

				iResult = recvfrom(serverSocket1, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress,
					&sockAddrLen);

				portNumber = SERVER_PORT1;
				if (iResult == SOCKET_ERROR)
				{
					printf("recvfrom failed with error %d\n", WSAGetLastError());
					continue;
				}
				else {
					dataBuffer[iResult] = '\0';

					printf("Server recieved message on %d port. Client sent: %s.\n", portNumber, dataBuffer);
				}
			}

			if (FD_ISSET(serverSocket2, &readfds)) {
				// izvrsenje operacije

				iResult = recvfrom(serverSocket2, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress,
					&sockAddrLen);

				portNumber = SERVER_PORT2;
				if (iResult == SOCKET_ERROR)
				{
					printf("recvfrom failed with error %d\n", WSAGetLastError());
					continue;
				}
				else {

					dataBuffer[iResult] = '\0';

					printf("Server recieved message on %d port. Client sent: %s.\n", portNumber, dataBuffer);
				}
			}

			if (FD_ISSET(serverSocket1, &exceptionfds)) {
				// izvrsenje operacije
				break;
			}

			if (FD_ISSET(serverSocket2, &exceptionfds)) {
				// izvrsenje operacije
				break;
			}
			else {
				continue;
			}
		}

	}


	if ((closesocket(serverSocket1) == SOCKET_ERROR) || (closesocket(serverSocket2) == SOCKET_ERROR))
	{
		printf("closesocket failed with error %d\n", WSAGetLastError());
		return 1;
	}

	WSACleanup();

	return 0;
}
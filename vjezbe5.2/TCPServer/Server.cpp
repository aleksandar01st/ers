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

#pragma pack(1)

#define SERVER_PORT 27016
#define BUFFER_SIZE 256
#define MAX_CLIENTS 3

struct studentInfo {
	char ime[15];
	char prezime[20];
	short poeni;
};


// TCP server that use non-blocking sockets
int main()
{

	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET clientSocket[MAX_CLIENTS];

	int iResult;
	char dataBuffer[BUFFER_SIZE];

	short lastIndex = 0;

	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	sockaddr_in serverAddress;

	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET; //set server address protocol family
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(SERVER_PORT);

	memset(clientSocket, 0, MAX_CLIENTS*sizeof(SOCKET));

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	


	/// /////////////////////
	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}
	/// /////////////////////

	unsigned long mode = 1; //non-blocking mode
	iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR)
		printf("ioctlsocket failed with error: %ld\n", iResult);


	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");



	fd_set readfds;

	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	studentInfo* student;

	while (true) {

		FD_ZERO(&readfds);

		if (lastIndex != MAX_CLIENTS) {
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < lastIndex; i++) {
			FD_SET(clientSocket[i], &readfds);
		}

		int result = select(0, &readfds, NULL, NULL, &timeVal);

		if (result == 0) {
			// vreme za cekanje je isteklo
			if (_kbhit()) {
				getch();
				printf("Primena racunarskih mreza u infrstrukturnim sistemima 2019/2020\n");
			}continue;
		}
		else if (result == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else {
			if (FD_ISSET(listenSocket, &readfds)) {
				// izvrsenje operacije
				sockaddr_in clientAddr;
				int clientAddrSize = sizeof(struct sockaddr_in);

				clientSocket[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
				if (clientSocket[lastIndex] == INVALID_SOCKET)
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
					WSACleanup();
				}
				else {
					if (ioctlsocket(clientSocket[lastIndex], FIONBIO, &mode) != 0)
					{
						printf("ioctlsocket failed with error.");
						continue;
					}
					lastIndex++;
					printf("New client request accepted (%d). Client address: %s : %d\n", lastIndex, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

				}
			}
			else {
				for (int i = 0; i < lastIndex; i++) {
					if (FD_ISSET(clientSocket[i], &readfds)) {
						// izvrsenje operacije

						iResult = recv(clientSocket[i], dataBuffer, BUFFER_SIZE, 0);

						if(iResult > 0){
							dataBuffer[iResult] = '\0';
							printf("Message recieved from client (%d):\n", i + 1);
							student = (studentInfo*)dataBuffer;

							printf("Ime i prezime: %s %s\n", student->ime, student->prezime);
							printf("Poeni studenta: %d\n\n", ntohs(student->poeni));
						}
						else if (iResult == 0) {
							printf("Connection with client (%d) closed.\n", i+1);
							closesocket(clientSocket[i]);

							for (int j = i; j < lastIndex - 1; j++) {
								clientSocket[j] = clientSocket[j + 1];
							}
							clientSocket[lastIndex - 1] = 0;

							lastIndex--;
						}
						else {
							printf("recv failed with error: %d\n", WSAGetLastError());
							closesocket(clientSocket[i]);

							for (int j = i; j < lastIndex - 1; j++) {
								clientSocket[j] = clientSocket[j + 1];
							}
							clientSocket[lastIndex - 1] = 0;

							lastIndex--;
						}
					}
				}
			}
		}
	}

	closesocket(listenSocket);

	
	WSACleanup();

	return 0;
}

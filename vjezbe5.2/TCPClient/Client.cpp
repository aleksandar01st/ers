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

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 256


struct studentInfo {
	char ime[15];
	char prezime[20];
	short poeni;
};


// TCP client that use non-blocking sockets
int main()
{
	SOCKET connectSocket = INVALID_SOCKET;

	int iResult;

	char dataBuffer[BUFFER_SIZE];

	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
	serverAddress.sin_port = htons(SERVER_PORT);


	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	studentInfo student; 
	short poeni;

	while (true) {

		printf("Unesite ime studenta: ");
		gets_s(student.ime, 15);

		printf("Unesite prezime studenta: ");
		gets_s(student.prezime, 20);

		printf("Unesite osvojene poene na testu: ");
		scanf("%d", &poeni);
		student.poeni = htons(poeni);
		getchar();

		iResult = send(connectSocket, (char*)&student, (int)sizeof(studentInfo), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Message succefully sent. Total bytes: %ld\n", iResult);

		printf("\nPress 'x' to exit or any other key to continue: ");
		if (getch() == 'x') break;
	}


	iResult = shutdown(connectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	closesocket(connectSocket);

	WSACleanup();

	return 0;
}

/*
 ============================================================================
 Name        : Client.c
 Author      : Ingusci Paolo - De Matteis Davide
 ============================================================================
 */

#include "UDP_protocol.h"
#include "check.h"
#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


void ErrorHandler(char *errorMessage) {
	printf(errorMessage);
}

void ClearWinSock() {
#if defined WIN32
WSACleanup();
#endif
}


int main(char args[]) {
#if defined WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup() \n");
		printf("A usable WinSock DLL cannot be found\n");
		return EXIT_FAILURE;
	}
#endif

int sock;
struct sockaddr_in echoServAddr;
struct sockaddr_in fromAddr;
unsigned int fromSize;
char echoString[BUFFER_SIZE];
char echoBuffer[BUFFER_SIZE];
int echoStringLen;
int respStringLen;

	// CREAZIONE DELLA SOCKET
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		ErrorHandler("socket() creation failed.\n");
		ClearWinSock();
		return -1;
	}

	// COSTRUZIONE DELL'INDIRIZZO DEL SERVER
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = PF_INET;
	echoServAddr.sin_port = htons(PORT);
	echoServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// INVIO DELL'OPERAZIONE AL SERVER
	memset(echoString, '\0', BUFFER_SIZE);
	puts("Insert the string useful for the calculation (maximum 64 characters) \n(format: \"operator[space]num1[space]num2\"):");
	gets(echoString);

	//string format controls
	while(lenght_check(strlen(echoString)) || string_check(echoString)) //loop that ends when a string of the correct format is entered
	{
		system("PAUSE");
		system("cls");
		puts("Insert the string useful for the calculation (maximum 64 characters) \n(format: \"operator[space]num1[space]num2\" ):");
		gets(echoString);
	}

	//Sending the string to the server
	echoStringLen = strlen(echoString);
	//printf("Waiting for connection --> ");
	//printf("Connection established with %s:%d\n", inet_ntoa(echoServAddr.sin_addr), ntohs(echoServAddr.sin_port));
	if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) != echoStringLen){
		ErrorHandler("send() sent a different number of bytes than expected");
		closesocket(sock);
		ClearWinSock();
		return -1;
	}

	fromSize = sizeof(fromAddr);
	//loop that exchanges messages with the client until "=" is entered
	while (strcmp(echoString, "=")!=0){
		memset(echoBuffer, '\0', BUFFER_SIZE);
		if((respStringLen = recvfrom(sock, echoBuffer, BUFFER_SIZE, 0, (struct sockaddr*)&fromAddr, &fromSize)) <= 0){
			ErrorHandler("recvfrom() failed or connection closed prematurely");
			closesocket(sock);
			ClearWinSock();
			return -1;
		}
		printf("Received result from server %s, ip %s: %s\n", inet_ntoa(fromAddr.sin_addr), inet_ntoa(fromAddr.sin_addr), echoBuffer);
		system("pause");
		system("cls");
		memset(echoString, '\0', BUFFER_SIZE);

		//inserting the new string
		puts("Insert the string useful for the calculation (maximum 64 characters) \n(format: \"operator[space]num1[space]num2\"):");
		gets(echoString);
		while(string_check(echoString)) //loop that ends when a string of the correct format is entered
		{
			system("PAUSE");
			system("cls");
			puts("Insert the string useful for the calculation (maximum 64 characters) \n(format: \"operator[space]num1[space]num2\" ):");
			gets(echoString);
		}

		//Sending the string to the server
		echoStringLen = strlen(echoString);
		if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) != echoStringLen){
			ErrorHandler("sendto() sent a different number of bytes than expected");
			closesocket(sock);
			ClearWinSock();
			return -1;
		}
	}

	//closing the connection
	closesocket(sock);
	ClearWinSock();
	system("pause");
	return EXIT_SUCCESS;
}
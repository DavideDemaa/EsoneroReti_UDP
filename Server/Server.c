/*
 * Server.c
 *      Author: Paolo Ingusci - De Matteis Davide
 */

#include "UDP_protocol.h"
#include "calculation.h"
#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define HOSTNAME_SIZE 256  // O una dimensione appropriata in base alle tue esigenze

// Dichiarazione della variabile clientHostName
char clientHostName[HOSTNAME_SIZE];

void ErrorHandler(char *errorMessage) {

	printf(errorMessage);
}

void ClearWinSock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]){
#if defined WIN32
	WSADATA wsa_data;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsa_data);

	if(iResult != NO_ERROR) {
		printf("Error at WSAStartup() \n");
		printf("A usable WinSock DLL cannot be found\n");
		return EXIT_FAILURE;
	}
	#endif

	int sock;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;
	unsigned int cliAddrLen;
	char echoBuffer[BUFFER_SIZE];
	int recvMsgSize;

	// CREAZIONE DELLA SOCKET
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		ErrorHandler("Socket creation failed. \n");
		ClearWinSock();
		return -1;
	}

	// COSTRUZIONE DELL'INDIRIZZO DEL SERVER
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_port = htons(PORT);
	echoServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// BIND DELLA SOCKET
	if((bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))) < 0){
		ErrorHandler("bind() failed. \n");
		closesocket(sock);
		return -1;
	}

	printf( "In attesa di connessioni UDP... \n");

	// RICEZIONE DELLA STRINGA ECHO DAL CLIENT
	while(1) {
		cliAddrLen = sizeof(echoClntAddr);
		memset(echoBuffer, '\0', BUFFER_SIZE);	// ensures extra bytes contain '\0'

		//string "Connection established\n" sent to the client
		/*char input_string = "Connection established\n";
		int string_len = strlen(input_string);
		if (sendto(sock, input_string, BUFFER_SIZE, 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != string_len){
			ErrorHandler("send() sent a different number of bytes than expected\n");
			system("pause");
			closesocket(sock);
			ClearWinSock();
			return -1;
		}*/

		//string reception from server
		if((recvMsgSize = recvfrom(sock, echoBuffer, BUFFER_SIZE, 0, (struct sockaddr*)&echoClntAddr, &cliAddrLen)) <= 0){
			ErrorHandler("recvfrom() failed or connection closed prematurely\n");
			system("pause");
			closesocket(sock);
			ClearWinSock();
			return -1;
		}

		// Ricezione del nome dell'host del client
    	struct hostent *clientHost;
    	clientHost = gethostbyaddr((const char *)&echoClntAddr.sin_addr.s_addr, sizeof(echoClntAddr.sin_addr.s_addr), AF_INET);

    	if (clientHost == NULL) {
        	ErrorHandler("gethostbyaddr() failed\n");
        	system("pause");
        	closesocket(sock);
        	ClearWinSock();
        	return -1;
    	}

    	// Copia il nome dell'host del client nella variabile clientHostName
    	strncpy(clientHostName, clientHost->h_name, HOSTNAME_SIZE - 1);
    	clientHostName[HOSTNAME_SIZE - 1] = '\0';

    	// Visualizza il messaggio con il nome dell'host, l'indirizzo IP e l'operazione
    	printf("Richiesta operazione '%s' dal client %s, IP %s\n", echoBuffer, clientHostName, inet_ntoa(echoClntAddr.sin_addr));

		// receives strings from server until "=" is received
		while(strcmp(echoBuffer,"=") != 0){
			//calculation
			int num1 = 0, num2 = 0; // numbers between which the operation will be performed
			char op, result_out[100]; //operator
			float result; //result of the operation

			set_operation(echoBuffer, &num1, &num2, &op);
			result = calculation(num1, num2, op); //result calculation

			if(num2==0 && op=='/')
				strcpy(result_out, "inf");
			/*else
				sprintf(result_out, "%.3f", result);*/	

			// INVIA IL RISULTATO AL CLIENT
			int result_len = snprintf(result_out, sizeof(result_out), "%d %c %d = %.2f", num1, op, num2, result);
			if (sendto(sock, result_out, result_len, 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != result_len){
				ErrorHandler("\nsendto() sent a different number of bytes than expected\n");
				system("pause");
				closesocket(sock);
				ClearWinSock();
				return -1;
			}

			// receive the next string from the client
    		if ((recvMsgSize = recvfrom(sock, echoBuffer, BUFFER_SIZE, 0, (struct sockaddr *)&echoClntAddr, &cliAddrLen)) <= 0) {
        		ErrorHandler("recvfrom() failed or connection closed prematurely\n");
        		system("pause");
        		closesocket(sock);
        		ClearWinSock();
        		return -1;
    		}

    		// Ricezione del nome dell'host del client
    		struct hostent *clientHost;
    		clientHost = gethostbyaddr((const char *)&echoClntAddr.sin_addr.s_addr, sizeof(echoClntAddr.sin_addr.s_addr), AF_INET);

    		if (clientHost == NULL) {
        		ErrorHandler("gethostbyaddr() failed\n");
        		system("pause");
        		closesocket(sock);
        		ClearWinSock();
        		return -1;
    		}

    		// Copia il nome dell'host del client nella variabile clientHostName
    		strncpy(clientHostName, clientHost->h_name, HOSTNAME_SIZE - 1);
    		clientHostName[HOSTNAME_SIZE - 1] = '\0';

    		// Visualizza il messaggio con il nome dell'host, l'indirizzo IP e l'operazione
    		printf("Richiesta operazione '%s' dal client %s, IP %s\n", echoBuffer, clientHostName, inet_ntoa(echoClntAddr.sin_addr));
		}
	}
}
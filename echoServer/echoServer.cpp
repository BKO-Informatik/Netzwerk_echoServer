/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include<ctime>

#include <iostream>
#include <chrono>
#include <string>


#ifdef _WIN32 
/* Headerfiles f�r Windows */
#include <winsock.h>
#include <io.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996) // disable windows warning for ctime 

#else
/* Headerfiles f�r Unix/Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

/* Portnummer */
#define PORT 1234
/* Puffer f�r eingehende Nachrichten */
#define RCVBUFSIZE 1024

#ifdef _WIN32
static void echo(SOCKET);
#else
static void echo(int);
#endif

static void error_exit(char *errorMessage);

/* Funktion gibt Daten vom Client auf dem stdout aus,
* welche dieser mit der Kommandozeile �bergibt  */
#ifdef _WIN32
static void echo(SOCKET client_socket)
#else
static void echo(int client_socket)
#endif
{
	char echo_buffer[RCVBUFSIZE];
	int recv_size;


	if ((recv_size =
		recv(client_socket, echo_buffer, RCVBUFSIZE, 0)) < 0)
		error_exit("Fehler bei recv()");
	echo_buffer[recv_size] = '\0';
	
	//printf("Nachrichten vom Client : %s ",
	//	echo_buffer);

	std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(p);
				
	std::cout << "Nachricht vom Client: " << echo_buffer << " " << std::ctime(&t) << std::endl; // for example : Tue Dec 16 14:21:13 2014
			
}

/* Funktion gibt aufgetrenene Fehler aus und
* beendet die Anwendung */
static void error_exit(char *error_message) {

#ifdef _WIN32
	fprintf(stderr, "%s: %d\n", error_message, WSAGetLastError());
#else
	fprintf(stderr, "%s: %s\n", error_message, strerror(errno));
#endif

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	struct sockaddr_in server, client;

#ifdef _WIN32
	SOCKET sock, fd;
#else
	int sock, fd;
#endif

	int len;

#ifdef _WIN32  
	/* Initialisiere TCP f�r Windows ("winsock") */
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
		error_exit("Fehler beim Initialisieren von Winsock");
	else
		printf("Winsock initialisiert\n");
#endif

	/* Erzeuge das Socket */
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
		error_exit("Fehler beim Anlegen eines Socket");

	/* Erzeuge die Socketadresse des Servers */
	memset(&server, 0, sizeof(server));
	/* IPv4-Verbindung */
	server.sin_family = AF_INET;
	/* INADDR_ANY Jede IP-Adresse annehmen */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Portnummer */
	server.sin_port = htons(PORT);

	/* Erzeuge die Bindung an die Serveradresse
	* (genauer an einen bestimmten Port) */
	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
		error_exit("Kann das Socket nicht \"binden\"");

	/* Teile dem Socket mit, dass Verbindungsw�nsche
	* von Clients entgegengenommen werden */
	if (listen(sock, 5) == -1)
		error_exit("Fehler bei listen");

	printf("Server bereit - wartet auf Anfragen ...\n");
	/* Bearbeite die Verbindungsw�nsche von Clients
	* in einer Endlosschleife
	* Der Aufruf von accept() blockiert solange,
	* bis ein Client Verbindung aufnimmt */
	for (;;) {
		len = sizeof(client);
		fd = accept(sock, (struct sockaddr*)&client, &len);
		if (fd < 0)
			error_exit("Fehler bei accept");
		printf("Bearbeite Client mit der Adresse: %s\n",
			inet_ntoa(client.sin_addr));
		/* Daten vom Client auf dem Bildschirm ausgeben */
		echo(fd);

		/* Schlie�e die Verbindung */
#ifdef _WIN32
		closesocket(fd);
#else 
		close(fd);
#endif
	}
	return EXIT_SUCCESS;
}


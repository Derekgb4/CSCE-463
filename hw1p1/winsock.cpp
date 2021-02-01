/* winsock.cpp
 * Derek Grey, Section 500, Spring 2021
 */

#include "pch.h"
#include "URLParse.h"
#include "socket.h"
#include <iostream>
#include <chrono.>
#include <fstream>
#pragma warning(disable : 4996)

using namespace std;
using namespace std::chrono;
#pragma comment(lib, "ws2_32.lib")

void winsock_test(URLParse url)
{
	// string pointing to an HTTP server (DNS name or IP)
	//char str [] = "www.tamu.edu";
	//char str [] = "128.194.135.72";
	//URLParse url = url;
	//cout << "testing: " << url.host << endl;
	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2,2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError ());
		WSACleanup();	
		return;
	}

	// open a TCP socket
	SOCKET sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf ("socket() generated error %d\n", WSAGetLastError ());
		WSACleanup ();	
		return;
	}

	// structure used in DNS lookups
	struct hostent *remote; 

	// structure for connecting to server
	struct sockaddr_in server;

	// first assume that the string is an IP address
	cout << '\t' << "Doing DNS. . . ";
	auto start = high_resolution_clock::now();

	DWORD IP = inet_addr (url.host.c_str());
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname (url.host.c_str())) == NULL)
		{
			printf ("Invalid string: neither FQDN, nor IP address\n");
			return;
		}
		else // take the first IP address and copy into sin_addr
			memcpy ((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	printf("done in %d ms, found %s\n", duration.count() / 1000, inet_ntoa (server.sin_addr));

	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons (80);		// host-to-network flips the byte order


	cout << '\t' << "Loading. . . ";
	start = high_resolution_clock::now();
	// connect to the server on port 80
	if (connect (sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf ("Connection error: %d\n", WSAGetLastError ());
		return;
	}

	// send HTTP requests here
	string request = "GET / HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0Host: www.tamu.edu\r\nConnection: close\r\n\r\n";
	/*char crequete[5000];
	strncpy(crequete, request.c_str(), request.size()+1);
	send(sock, crequete, strlen(crequete), 0);*/

	char* sendBuf = new char[request.size() + 1];
	strcpy(sendBuf, request.c_str());
	if (send(sock, sendBuf, request.size(), 0) == SOCKET_ERROR) {
		printf("Send error: %d\n", WSAGetLastError());
		return;
	}
	//
#define DEFAULT_BUFLEN 32000
	int recvbuflen = 32000;
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int bytes = 0;
	do {
		iResult = recv(sock, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);
			bytes = bytes + iResult;
			if (sizeof(recvbuf) == iResult) {
				char* newBuf = new char[sizeof(recvbuf) + recvbuflen];
				for (int i = 0; i < sizeof(recvbuf); i++) {
					newBuf[i] = recvbuf[i];
				}
				delete[] recvbuf;
				char recvbuf[sizeof(newBuf)];
				for (int i = 0; i < sizeof(recvbuf); i++) {
					recvbuf[i] = newBuf[i];
				}
				delete[] newBuf;
			}
		}
		else if (iResult == 0) {
			closesocket(sock);
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);

	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop - start);
	printf("done in %d ms with %d bytes", duration.count() / 1000, bytes);

	// close the socket to this server; open again for the next one
	closesocket (sock);

	// call cleanup when done with everything and ready to exit program
	WSACleanup ();
}

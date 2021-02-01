/* winsock.cpp
 * Derek Grey, Section 500, Spring 2021
 */

#include "pch.h"
#include "URLParse.h"
#include "socket.h"
#include <iostream>
#include <chrono.>
#include <fstream>
#include <string>
#include "HTMLParserBase.h"
#include <cstring>

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

	cout << "      * Connecting on page. . . ";
	start = high_resolution_clock::now();

	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons (80);		// host-to-network flips the byte order


	
	// connect to the server on port 80
	if (connect (sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf ("Connection error: %d\n", WSAGetLastError ());
		return;
	}

	stop = high_resolution_clock::now();
	duration = duration_cast<microseconds>(stop - start);
	printf("done in %d ms\n", duration.count() / 1000);

	cout << '\t' << "Loading. . . ";
	start = high_resolution_clock::now();

	// send HTTP requests here
	string request = "GET / HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0Host: 128.194.135.72\r\nConnection: close\r\n\r\n";
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
#define DEFAULT_BUFLEN 4000
	int recvbuflen = 4000;
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int bytes = 0;
	string result;
	do {
		iResult = recv(sock, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);
			bytes = bytes + iResult;
			result = result + recvbuf;
			//cout << endl << recvbuf << endl;
			if (sizeof(recvbuf) == iResult) {
				char* newBuf = new char[sizeof(recvbuf) + recvbuflen];
				for (int i = 0; i < sizeof(recvbuf); i++) {
					newBuf[i] = recvbuf[i];
				}
//				delete[] recvbuf;
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
	printf("done in %d ms with %d bytes\n", duration.count() / 1000, bytes);

	cout << '\t' << "Verifying header. . . status code ";
	string resultTemp = result;
	string HTTPCheck = resultTemp.substr(0, resultTemp.find(" "));
	string afterCheck = resultTemp.erase(0, resultTemp.find(" ") + 1);
	string StatusCode = afterCheck.substr(0, afterCheck.find(" "));
	if (HTTPCheck == "HTTP/1.0" || "HTTP/1.1") {
		//cout << "sucess" << endl;
	}
	else {
		cout << "failed with non-HTTP header" << endl;
		exit(EXIT_FAILURE);
	}

	if (StatusCode == "200") {
		cout << StatusCode << endl;
		cout << "      + Parsing page. . . ";
		auto start = high_resolution_clock::now();
		//parse html

		ofstream file("GivenWebAddress.html");
		file << result;
		file.close();
		 
		char filename[] = "GivenWebAddress.html";
		
		// open html file
		HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		// process errors
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf("CreateFile failed with %d\n", GetLastError());
			exit(EXIT_FAILURE);
		}

		// get file size
		LARGE_INTEGER li;
		BOOL bRet = GetFileSizeEx(hFile, &li);
		// process errors
		if (bRet == 0)
		{
			printf("GetFileSizeEx error %d\n", GetLastError());
			exit(EXIT_FAILURE);
		}

		// read file into a buffer
		int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
		DWORD bytesRead;
		// allocate buffer
		char* fileBuf = new char[fileSize];
		// read into the buffer
		bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
		// process errors
		if (bRet == 0 || bytesRead != fileSize)
		{
			printf("ReadFile failed with %d\n", GetLastError());
			exit(EXIT_FAILURE);
		}

		// done with the file
		CloseHandle(hFile);

		// create new parser object
		HTMLParserBase* parser = new HTMLParserBase;

		char* baseUrl = url.getBaseURL();		// where this page came from; needed for construction of relative links
		

		int nLinks;
		char* linkBuffer = parser->Parse(fileBuf, fileSize, baseUrl, (int)strlen(baseUrl), &nLinks);

		// check for errors indicated by negative values
		if (nLinks < 0)
			nLinks = 0;

		//printf("Found %d links:\n", nLinks);

		// print each URL; these are NULL-separated C strings
		for (int i = 0; i < nLinks; i++)
		{
			//printf("%s\n", linkBuffer);
			linkBuffer += strlen(linkBuffer) + 1;
		}

		delete parser;		// this internally deletes linkBuffer
		delete fileBuf;





		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);




		printf("done in %d ms with %d links\n", duration.count() / 1000, nLinks);
		cout << endl << "--------------------------------------------" << endl << result << endl;
	}
	else {
		cout << endl << "--------------------------------------------" << endl << result << endl;
	}
	

	

	// close the socket to this server; open again for the next one
	closesocket (sock);

	// call cleanup when done with everything and ready to exit program
	WSACleanup ();
}

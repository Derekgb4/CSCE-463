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
#include <unordered_set>
#include <stdio.h>
#include <sstream>
#include <cstring>



#pragma warning(disable : 4996)

using namespace std;
using namespace std::chrono;
#pragma comment(lib, "ws2_32.lib")
unordered_set<string> seenIP; 

unordered_set<string> UniqueIP(string IP, unordered_set<string> seenIP) {

	int prevSize = seenIP.size();
	seenIP.insert(IP);
	if (seenIP.size() > prevSize) { // unique host
		printf("passed\n");
	}
	else {
		printf("failed\n");
			return seenIP;
	}
	// duplicate host
	return seenIP;
}

void ipCheck(string IPAddress, bool args) {
	if (args == 1) {
		cout << "\tChecking IP uniqueness. . . ";
		//cout << "testing IP value:" << IP << endl;
		int prevSize = seenIP.size();
		seenIP = UniqueIP(IPAddress, seenIP);
		if (seenIP.size() > prevSize) { // unique host

		}
		else {
			return;
		}
	}
}

bool robotRequest(sockaddr_in server, SOCKET sock, string path, string host, bool args)
{
	bool robotBool = 0;
	if (args == 1) {
		cout << "\tConnecting on robots. . . ";
		auto start = high_resolution_clock::now();

		// string pointing to an HTTP server (DNS name or IP)
		WSADATA wsaData;

		//Initialize WinSock; once per program run
		WORD wVersionRequested = MAKEWORD(2, 2);
		if (WSAStartup(wVersionRequested, &wsaData) != 0) {
			printf("WSAStartup error %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		// open a TCP socket
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			printf("socket() generated error %d\n", WSAGetLastError());
			WSACleanup();
			return robotBool;
		}

		// setup the port # and protocol type
		server.sin_family = AF_INET;
		server.sin_port = htons(80);		// host-to-network flips the byte order

		// connect to the server on port 80
		if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			printf("Connection error: %d\n", WSAGetLastError());
			return robotBool;
		}


		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		printf("done in %d ms\n", duration.count() / 1000);

		///////////////////////////////////////////////////////
		string pageData;

		cout << '\t' << "Loading. . . ";
		start = high_resolution_clock::now();
		// send HTTP requests here
		string request = "HEAD /robots.txt HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";

		char* sendBuf = new char[request.size() + 1];
		strcpy(sendBuf, request.c_str());
		if (send(sock, sendBuf, request.size(), 0) == SOCKET_ERROR) {
			printf("Send error: %d\n", WSAGetLastError());
			return robotBool;
		}

		//
#define DEFAULT_BUFLEN 4000
		int recvbuflen = 4000;
		int iResult;
		char recvbuf[DEFAULT_BUFLEN];
		int bytes = 0;
		timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		fd_set readset;
		int ret;
		bool check = true;
		clock_t timer = clock();


		timeout.tv_sec -= floor(((clock() - timer) / (double)CLOCKS_PER_SEC));
		//timeout.tv_usec = 0;
		FD_ZERO(&readset);
		FD_SET(sock, &readset);
		if ((ret = select(1, &readset, 0, 0, &timeout)) > 0) {
			do {
				check = true;
				if (sizeof(recvbuf) / sizeof(char) >= 16000) {
					printf("failed with exceeding max\n");
					return robotBool;
				}else
				iResult = recv(sock, recvbuf, recvbuflen, 0);
				if (iResult > 0) {
					//printf("Bytes received: %d\n", iResult);
					bytes = bytes + iResult;
					pageData = pageData + recvbuf;
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
						//delete[] recvbuf;
					}
				}
				else if (iResult == 0) {
					closesocket(sock);
				}
				else {
					printf("recv failed: %d\n", WSAGetLastError());
				}
			} while (iResult > 0);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			printf("done in %d ms with %d bytes\n", duration.count() / 1000, bytes);
		}
		else {
			cout << "failed with timeout" << endl;
			return robotBool;
		}
		//pageData;
		//cout << endl << "PAGE DATA:" << endl << pageData << endl;
		///////////////////////////////////////////////////
		cout << '\t' << "Verifying header. . . status code ";
		string resultTemp = pageData;
		string HTTPCheck = resultTemp.substr(0, resultTemp.find(" "));
		string afterCheck = resultTemp.erase(0, resultTemp.find(" ") + 1);
		string StatusCode = afterCheck.substr(0, afterCheck.find(" "));
		if (HTTPCheck == "HTTP/1.0" || "HTTP/1.1") {
			cout << StatusCode << endl;
		}
		else {
			cout << "failed with non-HTTP header" << endl;
			return robotBool;
		}

		if (StatusCode[0] == '4') {
			robotBool = 1;
			return robotBool;
		}
		else {
			robotBool = 0;
			return robotBool;
		}

	}
}

void pageConnect(sockaddr_in server, SOCKET sock) {
	cout << "      * Connecting on page. . . ";


	auto start = high_resolution_clock::now();

	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons(80);		// host-to-network flips the byte order



	// connect to the server on port 80
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("Connection error: %d\n", WSAGetLastError());
		return;
	}

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	printf("done in %d ms\n", duration.count() / 1000);
}

string checkHTTP(string pageData) {
	cout << '\t' << "Verifying header. . . status code ";
	string resultTemp = pageData;
	string HTTPCheck = resultTemp.substr(0, resultTemp.find(" "));
	string afterCheck = resultTemp.erase(0, resultTemp.find(" ") + 1);
	string StatusCode = afterCheck.substr(0, afterCheck.find(" "));
	if (HTTPCheck == "HTTP/1.0" || "HTTP/1.1") {
		//cout << "sucess" << endl;
		return StatusCode;
	}
	else {
		cout << "failed with non-HTTP header" << endl;
		exit(EXIT_FAILURE);
	}
}

void pageParse(string StatusCode, string result, URLParse url, bool args) {
	if (StatusCode[0] != '2') {
		cout << StatusCode << endl;
		return;
	}
	if (StatusCode[0] == '2') {
		cout << StatusCode << endl;
		cout << "      + Parsing page. . . ";
		auto start = high_resolution_clock::now();
		//parse html

		ofstream file(url.getFileName());
		file << result;
		file.close();

		char* filename = url.getFileName();

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
		if (args == 0) {
			cout << endl << "--------------------------------------------" << endl << result << endl;
		}
	}
	else {
		cout << StatusCode << endl;
		if (args == 0) {
			cout << endl << "--------------------------------------------" << endl << result << endl;
		}
	}
}

string loadPage(SOCKET sock, URLParse url) {

	string pageData;

	cout << '\t' << "Loading. . . ";
	auto start = high_resolution_clock::now();
	// send HTTP requests here
	string requestPath = url.path.substr(0, url.path.size()-1);
	//cout << endl << "This is the request portion: " << requestPath << ":" << endl;
	//if ( requestPath !=)
	string request = "GET /" +requestPath+ " HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0\r\nHost: " + url.host + "\r\nConnection: close\r\n\r\n";
	//cout << endl << "This is the Get Request: " << endl << request << endl;

	char* sendBuf = new char[request.size() + 1];
	strcpy(sendBuf, request.c_str());
	if (send(sock, sendBuf, request.size(), 0) == SOCKET_ERROR) {
		printf("Send error: %d\n", WSAGetLastError());
		return pageData;
	}
	//
#define DEFAULT_BUFLEN 4000
	int recvbuflen = 4000;
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int bytes = 0;
	timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	fd_set readset;
	int ret;
	bool check = true;
	clock_t timer = clock();


	timeout.tv_sec -= floor(((clock() - timer) / (double)CLOCKS_PER_SEC));
	//timeout.tv_usec = 0;
	FD_ZERO(&readset);
	FD_SET(sock, &readset);
	if ((ret = select(1, &readset, 0, 0, &timeout)) > 0) {
		do {
			check = true;
			if (sizeof(recvbuf) / sizeof(char) >= 16000) {
				printf("failed with exceeding max\n");
				return pageData;
			}
			else
			iResult = recv(sock, recvbuf, recvbuflen, 0);
			if (iResult > 0) {

				//printf("Bytes received: %d\n", iResult);
				bytes = bytes + iResult;
				pageData = pageData + recvbuf;
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
					//delete[] recvbuf;
				}
			}
			else if (iResult == 0) {
				closesocket(sock);
			}
			else {
				printf("recv failed: %d\n", WSAGetLastError());
			}
		} while (iResult > 0);
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		printf("done in %d ms with %d bytes\n", duration.count() / 1000, bytes);
	}
	else {
		cout << "failed with timeout" << endl;
		return pageData;
	}
	return pageData;
}



bool dnsLookup(URLParse url) {
	auto start = high_resolution_clock::now();
	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;
	cout << '\t' << "Doing DNS. . . ";
	// first assume that the string is an IP address
	DWORD IP = inet_addr(url.host.c_str());
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(url.host.c_str())) == NULL)
		{
			printf("failed with 11001\n");
			return 0;
		}
		else {
			return 1;
		}
	}
}


struct sockaddr_in dnsDoLookup(URLParse url) {
	auto start = high_resolution_clock::now();
	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;

	// first assume that the string is an IP address
	DWORD IP = inet_addr(url.host.c_str());
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(url.host.c_str())) == NULL)
		{
		}
		else // take the first IP address and copy into sin_addr
			memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		printf("done in %d ms, found %s\n", duration.count() / 1000, inet_ntoa(server.sin_addr));
		return server;
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		printf("done in %d ms, found %s\n", duration.count() / 1000, inet_ntoa(server.sin_addr));
		return server;
	}
}

void winsock_test(URLParse url, bool args)
{
	// string pointing to an HTTP server (DNS name or IP)
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

	if (dnsLookup(url)) {
		struct sockaddr_in server = dnsDoLookup(url);


		ostringstream base;
		base << inet_ntoa(server.sin_addr);
		string IPAddress = base.str();

		ipCheck(IPAddress, args);
		bool robotBool = robotRequest(server, sock, url.path, url.host, args);
		if (robotBool) {
			pageConnect(server, sock);
			string pageData = loadPage(sock, url);
			//cout << endl << "This is the page data:\n" << pageData << endl;
			string StatusCode = checkHTTP(pageData);
			pageParse(StatusCode, pageData, url, args);
		}
	}
	// close the socket to this server; open again for the next one
	closesocket (sock);

	// call cleanup when done with everything and ready to exit program
	WSACleanup ();
}

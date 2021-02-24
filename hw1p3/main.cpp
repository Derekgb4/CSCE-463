/* main.cpp
 * Derek Grey, Section 500, Spring 2021
 */
#include "pch.h"
#include "URLParse.h"
#include "socket.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <unordered_set>
#include "HTMLParserBase.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#pragma warning(disable : 4996)



using namespace std;
// this class is passed to all threads, acts as shared memory
struct Parameters {

	HANDLE mutex;
	HANDLE urlListM;
	HANDLE listEmptyS;
	HANDLE listFullS;
	HANDLE QueueM;
	HANDLE hostM;
	HANDLE IPM;

	unordered_set<string> seenHosts;
	unordered_set<string> seenIps;
	queue<URLParse> urls;
	string inputFile;
	char* filename;
	int numLines;
	int currentThreads;
	int extractedUrls;
	int uniquehostNum;
	int successfulldnsNum;
	int uniqueipNum;
	int robotpassNum;
	int successfullurlsCrawled; //those with a valid HTTP code
	int totalLinks;
	int totalTime;
	int attemptedRobots;
	int pagesParsed;
	int twoxx;
	int threexx;
	int fourxx;
	int fivexx;
	int otherxx;
	UINT64 downloadedTotal;
};

// function inside winsock.cpp
void winsock_test(URLParse url, bool args);
void UrlDownload(char* filename);

struct sockaddr_in dnsDoLookups(URLParse url) {
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
		return server;
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
		return server;
	}
}

bool dnsLookups(URLParse url) {
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
			//printf("failed with 11001\n");
			return 0;
		}
		else {
			return 1;
		}
	}
}

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

void pageConnects(sockaddr_in server, SOCKET sock) {
	//cout << "      * Connecting on page. . . ";

	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons(80);		// host-to-network flips the byte order



	// connect to the server on port 80
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		//printf("Connection error: %d\n", WSAGetLastError());
		return;
	}
}

bool robotRequests(sockaddr_in server, SOCKET sock, string path, string host)
{
	bool robotBool = 0;
	//cout << "\tConnecting on robots. . . ";


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
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
		//printf("Connection error: %d\n", WSAGetLastError());
		return robotBool;
	}

	///////////////////////////////////////////////////////
	string pageData;

	//cout << '\t' << "Loading. . . ";
	// send HTTP requests here
	string request = "HEAD /robots.txt HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";

	char* sendBuf = new char[request.size() + 1];
	strcpy(sendBuf, request.c_str());
	if (send(sock, sendBuf, request.size(), 0) == SOCKET_ERROR) {
		//printf("Send error: %d\n", WSAGetLastError());
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
		//do {
		check = true;
		if (sizeof(recvbuf) / sizeof(char) >= 16000) {
			//printf("failed with exceeding max\n");
			return robotBool;
		}
		else
			iResult = recv(sock, recvbuf, recvbuflen, 0);
		if (iResult < 0) {
			//cout << WSASetLastError << endl;
			return robotBool;
		}
		if (iResult == 0) {
			closesocket(sock);
		}
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
		//else if (iResult == 0) {
			//closesocket(sock);
		//}
		//else {
			//printf("recv failed: %d\n", WSAGetLastError());
		//}
	//} while (iResult > 0);
	}
	else if (ret == 0) {
		//cout << "failed with timeout" << endl;
		return robotBool;
	}
	else {
		//cout << "connection error: " << WSAGetLastError << endl;
		return robotBool;
	}
	//pageData;
	//cout << endl << "PAGE DATA:" << endl << pageData << endl;
	///////////////////////////////////////////////////
	//cout << '\t' << "Verifying header. . . status code ";
	string resultTemp = pageData;
	string HTTPCheck = resultTemp.substr(0, resultTemp.find(" "));
	string afterCheck = resultTemp.erase(0, resultTemp.find(" ") + 1);
	string StatusCode = afterCheck.substr(0, afterCheck.find(" "));

	if (HTTPCheck == "HTTP/1.0" || "HTTP/1.1") {
		//cout << StatusCode << endl;
	}
	else {
		//cout << "failed with non-HTTP header" << endl;

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

string loadPages(SOCKET sock, URLParse url) {

	string pageData;

	//cout << '\t' << "Loading. . . ";
	// send HTTP requests here
	string requestPath = url.path.substr(0, url.path.size() - 1);
	//cout << endl << "This is the request portion: " << requestPath << ":" << endl;
	//if ( requestPath !=)
	string request = "GET /" + requestPath + " HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0\r\nHost: " + url.host + "\r\nConnection: close\r\n\r\n";
	//cout << endl << "This is the Get Request: " << endl << request << endl;

	char* sendBuf = new char[request.size() + 1];
	strcpy(sendBuf, request.c_str());
	if (send(sock, sendBuf, request.size(), 0) == SOCKET_ERROR) {
		//printf("Send error: %d\n", WSAGetLastError());
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
				//printf("failed with exceeding max\n");
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
				//printf("recv failed: %d\n", WSAGetLastError());
				//pageData = "";
				return pageData;
			}
		} while (iResult > 0);
	}
	else {
		//cout << "failed with timeout" << endl;
		//pageData = "";
		return pageData;
	}
	return pageData;
}

string checkHTTPs(string pageData) {
	//cout << '\t' << "Verifying header. . . status code ";
	string resultTemp = pageData;
	string HTTPCheck = resultTemp.substr(0, resultTemp.find(" "));
	string afterCheck = resultTemp.erase(0, resultTemp.find(" ") + 1);
	string StatusCode = afterCheck.substr(0, afterCheck.find(" "));
	if (HTTPCheck == "HTTP/1.0" || "HTTP/1.1") {
		//cout << "sucess" << endl;
		return StatusCode;
	}
	else {
		//cout << "failed with non-HTTP header" << endl;
		exit(EXIT_FAILURE);
	}
}

int pageParses(string StatusCode, string result, URLParse url) {
	if (StatusCode[0] != '2') {
		//cout << StatusCode << endl;
		return 0;
	}
	if (StatusCode[0] == '2') {
		//cout << StatusCode << endl;
		//cout << "      + Parsing page. . . ";
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
			//printf("CreateFile failed with %d\n", GetLastError());
			exit(EXIT_FAILURE);
		}

		// get file size
		LARGE_INTEGER li;
		BOOL bRet = GetFileSizeEx(hFile, &li);
		// process errors
		if (bRet == 0)
		{
			//printf("GetFileSizeEx error %d\n", GetLastError());
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
			//printf("ReadFile failed with %d\n", GetLastError());
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
		return nLinks;
	}
}

UINT fileSharedQueue(LPVOID queueParams) {
	Parameters* p = (Parameters*)queueParams;
	string fileLine;
	ifstream input(p->inputFile, ios::binary);
	HANDLE urlListM = p->urlListM;
	HANDLE urlListFullS = p->listFullS;
	HANDLE urlListEmptyS = p->listEmptyS;
	int numLines = p->numLines;
	char* filename = p->filename;


	input.seekg(0, ios::end);
	int fileSize = input.tellg();
	cout << "Opened " << p->inputFile << " with size " << fileSize << endl;

	char mystring[300];
	FILE* pFile;
	pFile = fopen(filename, "r");

	while (!feof(pFile)) {
		if (fgets(mystring, 200, pFile)) {
			//cout << "this is the test string: " << mystring << endl;
			WaitForSingleObject(urlListEmptyS, INFINITE);
			p->urls.emplace(URLParse(mystring));
			ReleaseSemaphore(urlListFullS, 1, NULL);
		}
	}
	//printf("size is %i\n", p->urls.size());
	return 0;
}

UINT crawlThread(LPVOID crawlParams) {

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	HTMLParserBase* parser = new HTMLParserBase;

	Parameters* p = (Parameters*)crawlParams;
	unordered_set<string>* seenHosts = &p->seenHosts;
	unordered_set<string>* seenIps = &p->seenIps;
	HANDLE urlListM = p->urlListM;
	HANDLE urlListFullS = p->listFullS;
	HANDLE urlListEmptyS = p->listEmptyS;
	HANDLE urlQueueM = p->QueueM;
	HANDLE mutex = p->mutex;
	HANDLE hostM = p->hostM;
	HANDLE IPM = p->IPM;
	queue<URLParse>* urls = &p->urls;



	while (TRUE) {

		WaitForSingleObject(urlListFullS, INFINITE);
		WaitForSingleObject(urlQueueM, INFINITE);

		if (urls->size() == 0) {

			ReleaseMutex(urlQueueM);
			ReleaseSemaphore(urlListFullS, 1, NULL);

			WaitForSingleObject(mutex, INFINITE);
			p->currentThreads--;
			ReleaseMutex(mutex);
			return 0;
		}

		URLParse ParsedURL = urls->front();

		try {
			ParsedURL = urls->front();
			urls->pop();
		}
		catch (int errno) {
			ReleaseMutex(urlQueueM);
			ReleaseSemaphore(urlListFullS, 1, NULL);
			continue;
		}

		ReleaseMutex(urlQueueM);
		ReleaseSemaphore(urlListFullS, 1, NULL);

		//increases extractedUrls safely
		WaitForSingleObject(mutex, INFINITE);
		p->extractedUrls++;
		ReleaseMutex(mutex);

		//check for host uniqueness
		WaitForSingleObject(hostM, INFINITE);

		int prevSize = p->seenHosts.size();
		seenHosts->insert(ParsedURL.host);

		if (seenHosts->size() != prevSize) {
			p->uniquehostNum++;
			ReleaseMutex(hostM);
		}
		else {
			ReleaseMutex(hostM);
			continue;
		}

		WSADATA wsaData;

		//Initialize WinSock; once per program run
		WORD wVersionRequested = MAKEWORD(2, 2);
		if (WSAStartup(wVersionRequested, &wsaData) != 0) {
			printf("WSAStartup error %d\n", WSAGetLastError());
			WSACleanup();
			continue;
		}

		// open a TCP socket
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			printf("socket() generated error %d\n", WSAGetLastError());
			WSACleanup();
			continue;
		}

		if (dnsLookups(ParsedURL)) {

			WaitForSingleObject(mutex, INFINITE);
			p->successfulldnsNum++;
			ReleaseMutex(mutex);

			struct sockaddr_in server = dnsDoLookups(ParsedURL);

			WaitForSingleObject(IPM, INFINITE);

			ostringstream base;
			base << inet_ntoa(server.sin_addr);
			string IPAddress = base.str();

			int prevSize = seenIps->size();

			seenIps->insert(IPAddress);
			//cout << "seenIp size: " << seenIps->size() << endl;
			if (seenIps->size() != prevSize) {
				//printf("unique Host\n");
				p->uniqueipNum++;
				ReleaseMutex(IPM);

				WaitForSingleObject(mutex, INFINITE);
				p->attemptedRobots++;
				ReleaseMutex(mutex);

				bool robotBool = 0;

				// string pointing to an HTTP server (DNS name or IP)
				WSADATA wsaData;

				//Initialize WinSock; once per program run
				WORD wVersionRequested = MAKEWORD(2, 2);
				if (WSAStartup(wVersionRequested, &wsaData) != 0) {
					printf("WSAStartup error %d\n", WSAGetLastError());
					WSACleanup();
					continue;
				}

				// open a TCP socket
				sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (sock == INVALID_SOCKET)
				{
					printf("socket() generated error %d\n", WSAGetLastError());
					WSACleanup();
					continue;
				}

				// setup the port # and protocol type
				server.sin_family = AF_INET;
				server.sin_port = htons(80);		// host-to-network flips the byte order

				// connect to the server on port 80
				if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
				{
					//printf("Connection error: %d\n", WSAGetLastError());
					continue;
				}

				///////////////////////////////////////////////////////
				string pageData;

				//cout << '\t' << "Loading. . . ";
				// send HTTP requests here
				string request = "HEAD /robots.txt HTTP/1.0\r\nUser-agent: Derekgb4Crawler/1.0\r\nHost: " + ParsedURL.host + "\r\nConnection: close\r\n\r\n";

				char* sendBuf = new char[request.size() + 1];
				strcpy(sendBuf, request.c_str());
				if (send(sock, sendBuf, request.size(), 0) == SOCKET_ERROR) {
					//printf("Send error: %d\n", WSAGetLastError());
					continue;
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
					//do {
					check = true;
					if (sizeof(recvbuf) / sizeof(char) >= 16000) {
						//printf("failed with exceeding max\n");
						continue;
					}
					else
						iResult = recv(sock, recvbuf, recvbuflen, 0);
					if (iResult < 0) {
						//cout << WSASetLastError << endl;
						continue;
					}
					if (iResult == 0) {
						closesocket(sock);
					}
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
				}
				else if (ret == 0) {
					//cout << "failed with timeout" << endl;
					continue;
				}
				else {
					//cout << "connection error: " << WSAGetLastError << endl;
					continue;
				}

				WaitForSingleObject(mutex, INFINITE);
				p->downloadedTotal += pageData.length();
				ReleaseMutex(mutex);

				string resultTemp = pageData;
				string HTTPCheck = resultTemp.substr(0, resultTemp.find(" "));
				string afterCheck = resultTemp.erase(0, resultTemp.find(" ") + 1);
				string StatusCode = afterCheck.substr(0, afterCheck.find(" "));

				if (HTTPCheck == "HTTP/1.0" || "HTTP/1.1") {
					//cout << StatusCode << endl;
				}
				else {
					//cout << "failed with non-HTTP header" << endl;

					continue;
				}

				if (StatusCode[0] == '4') {

					robotBool = 1;
				}
				else {

					robotBool = 0;
					continue;
				}
				////////////////////////////////////////////////////////////////////

				if (robotBool) {

					WaitForSingleObject(mutex, INFINITE);
					p->robotpassNum++;
					ReleaseMutex(mutex);

					WSADATA wsaData;

					//Initialize WinSock; once per program run
					WORD wVersionRequested = MAKEWORD(2, 2);
					if (WSAStartup(wVersionRequested, &wsaData) != 0) {
						printf("WSAStartup error %d\n", WSAGetLastError());
						WSACleanup();
						continue;
					}

					// open a TCP socket
					SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					if (sock == INVALID_SOCKET)
					{
						printf("socket() generated error %d\n", WSAGetLastError());
						WSACleanup();
						continue;
					}

					pageConnects(server, sock);
					string pageData = loadPages(sock, ParsedURL);
					//if (pageData == "") {
						//continue;
					//}

					WaitForSingleObject(mutex, INFINITE);
					p->pagesParsed++;
					ReleaseMutex(mutex);

					WaitForSingleObject(mutex, INFINITE);
					p->downloadedTotal += pageData.length();
					ReleaseMutex(mutex);

					string StatusCode = checkHTTPs(pageData);
					//cout << "status code: " << StatusCode << endl;
					if (StatusCode[0] == '2') {
						WaitForSingleObject(mutex, INFINITE);
						p->twoxx++;
						ReleaseMutex(mutex);
					}
					else if (StatusCode[0] == '3') {
						WaitForSingleObject(mutex, INFINITE);
						p->threexx++;
						ReleaseMutex(mutex);
					}
					else if (StatusCode[0] == '4') {
						WaitForSingleObject(mutex, INFINITE);
						p->fourxx++;
						ReleaseMutex(mutex);
					}
					else if (StatusCode[0] == '5') {
						WaitForSingleObject(mutex, INFINITE);
						p->fivexx++;
						ReleaseMutex(mutex);
					}
					else {
						WaitForSingleObject(mutex, INFINITE);
						p->otherxx++;
						ReleaseMutex(mutex);
					}

					WaitForSingleObject(mutex, INFINITE);
					p->successfullurlsCrawled++;
					ReleaseMutex(mutex);

					int numLinks = pageParses(StatusCode, pageData, ParsedURL);
					//cout << "number of links found: " << numLinks << endl;
					WaitForSingleObject(mutex, INFINITE);
					p->totalLinks += numLinks;
					ReleaseMutex(mutex);
				}

			}
			else {
				//printf("not a unique Host\n");
				ReleaseMutex(IPM);
			}
		}
		closesocket(sock);

		WSACleanup();
	}
}

UINT statThread(LPVOID statParams) {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	Parameters* p = (Parameters*)statParams;
	unordered_set<string>* seenHosts = &p->seenHosts;
	unordered_set<string>* seenIps = &p->seenIps;
	queue<URLParse>* urls = &p->urls;

	int pendingQueueSize = urls->size();
	int extractedUrls = p->extractedUrls;



	clock_t start = clock();
	int time = 0;
	int urldiff = 0;
	int downloaddiff = 0;
	while (1) {
		urldiff = p->successfullurlsCrawled;
		downloaddiff = p->downloadedTotal;
		Sleep(2000);
		if (p->currentThreads == 0) {
			break;
		}
		time += 2.0;
		printf("[%3d] %5d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n", time, p->currentThreads, urls->size(), p->extractedUrls, seenHosts->size(), p->successfulldnsNum, seenIps->size(), p->robotpassNum, p->successfullurlsCrawled, p->totalLinks/1000);
		printf("       *** crawling %.1f pps @ %.3f Mbps\n", (static_cast<double>(p->successfullurlsCrawled)-static_cast<double>(urldiff))/ static_cast<double>(time), (static_cast<double>(p->downloadedTotal)/(static_cast<double>(time*1000000))));
		
		urldiff = p->successfullurlsCrawled - urldiff;
		downloaddiff = p->downloadedTotal - downloaddiff;
	}
	return 0;
}

bool fexists(const char* filename)
{
	ifstream file(filename);
	if (file) {
		return 1;
	}
	else
		return 0;
}

int main(int argc, char* argv[])
{

	if (argc == 3) {					// if there is 2 arguments perform p2

		// parse command line args
		int numThreads = atoi(argv[1]);
		string inputFile = argv[2];
		//vector<URLParse> urls;
		int numLines = 0;
		ifstream in(inputFile);
		std::string unused;
		while (std::getline(in, unused))
			++numLines;
		// initialize shared data structures & parameters sent to threads
		Parameters parameters;

		///////////////////
		parameters.mutex = CreateMutex(NULL, 0, NULL);
		parameters.urlListM = CreateMutex(NULL, 0, NULL);
		parameters.listEmptyS = CreateSemaphore(NULL, numLines, numLines, NULL);
		parameters.listFullS = CreateSemaphore(NULL, 1, numLines, NULL);
		parameters.QueueM = CreateMutex(NULL, 0, NULL);
		parameters.hostM = CreateMutex(NULL, 0, NULL);
		parameters.IPM = CreateMutex(NULL, 0, NULL);

		parameters.seenHosts = unordered_set<string>();
		parameters.seenIps = unordered_set<string>();
		parameters.urls = queue<URLParse>();
		parameters.inputFile = inputFile;
		parameters.filename = argv[2];
		parameters.numLines = numLines;
		parameters.currentThreads = 0;
		parameters.extractedUrls = 0;
		parameters.uniquehostNum = 0;
		parameters.successfulldnsNum = 0;
		parameters.uniqueipNum = 0;
		parameters.robotpassNum = 0;
		parameters.successfullurlsCrawled = 0; //those with a valid HTTP code
		parameters.totalLinks = 0;
		parameters.totalTime = 0;
		parameters.downloadedTotal = 0;
		parameters.attemptedRobots = 0;
		parameters.pagesParsed = 0;
		parameters.twoxx = 0;
		parameters.threexx = 0;
		parameters.fourxx = 0;
		parameters.fivexx = 0;
		parameters.otherxx = 0;

		// read file and populate shared queue
		HANDLE Queue = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fileSharedQueue, &parameters, 0, NULL);
		WaitForSingleObject(Queue, INFINITE);
		CloseHandle(Queue);
		//printf("\nsize of queue: %s", parameters.urls.front());

		// start stats thread
		HANDLE statHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)statThread, &parameters, 0, NULL);

		// start N crawling threads
		HANDLE* crawlerHandle = new HANDLE[numThreads];
		for (int i = 0; i < numThreads; i++) {
			crawlerHandle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)crawlThread, &parameters, 0, NULL);
			WaitForSingleObject(parameters.mutex, INFINITE);
			parameters.currentThreads++;
			ReleaseMutex(parameters.mutex);
		}
		// wait for N crawling threads to finish
		for (int i = 0; i < numThreads; i++) {
			WaitForSingleObject(crawlerHandle[i], INFINITE);
			CloseHandle(crawlerHandle[i]);
		}

		// signal stats thread to quit; wait for it to terminate
		WaitForSingleObject(statHandle, INFINITE);
		CloseHandle(statHandle);

		// cleanup 
		printf("number of unique hosts %i\n", parameters.uniquehostNum);
		printf("number of unique IPs %i\n", parameters.uniqueipNum);
		printf("number of extracted urls %i\n", parameters.extractedUrls);
		printf("number of attempted robots %i\n", parameters.attemptedRobots);
		printf("number of pages parsed %i\n", parameters.pagesParsed);
		printf("number of 2xx codes %i\n", parameters.twoxx);
		printf("number of 3xx codes %i\n", parameters.threexx);
		printf("number of 4xx codes %i\n", parameters.fourxx);
		printf("number of 5xx codes %i\n", parameters.fivexx);
		printf("number of other codes %i\n", parameters.otherxx);
		printf("links parsed %i\n", parameters.totalLinks);
		return 0;
	}
	return 0;
}

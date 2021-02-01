#pragma once
#include <winsock.h>


class Socket {
public:
	SOCKET sock; // socket handle
	char* buf; // current buffer
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
	int currentBufferSize;

	struct hostent* remote;
	struct sockaddr_in server;

	Socket();
	bool Read(void);
	bool SendRequest(URLParse url);
};
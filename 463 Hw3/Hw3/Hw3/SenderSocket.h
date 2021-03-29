// SenderSocket.h
#pragma once
#define MAGIC_PORT 22345 // receiver listens on this port
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver
#define MAGIC_PROTOCOL 0x8311AA 
#define FORWARD_PATH 0
#define RETURN_PATH 1 

#define MAX_ATTEMPTS 3 //actually 3, it is starting at 1 instead of 0

// possible status codes from ss.Open, ss.Send, ss.Close
#define STATUS_OK 0 // no error
#define ALREADY_CONNECTED 1 // second call to ss.Open() without closing connection
#define NOT_CONNECTED 2 // call to ss.Send()/Close() without ss.Open()
#define INVALID_NAME 3 // ss.Open() with targetHost that has no DNS entry
#define FAILED_SEND 4 // sendto() failed in kernel
#define TIMEOUT 5 // timeout after all retx attempts are exhausted
#define FAILED_RECV 6 // recvfrom() failed in kernel
using namespace std;


class Flags {
public:
	DWORD reserved : 5; // must be zero
	DWORD SYN : 1;
	DWORD ACK : 1;
	DWORD FIN : 1;
	DWORD magic : 24;
	Flags() { memset(this, 0, sizeof(*this)); magic = MAGIC_PROTOCOL; }
};

class ReceiverHeader {
public:
	Flags flags;
	DWORD recvWnd; // receiver window for flow control (in pkts) 
	DWORD ackSeq; // ack value = next expected sequence
};

class SenderDataHeader {
public:
	Flags flags;
	DWORD seq; // must begin from 0
};

class LinkProperties {
public:
	// transfer parameters
	float RTT; // propagation RTT (in sec)
	float speed; // bottleneck bandwidth (in bits/sec)
	float pLoss[2]; // probability of loss in each direction
	DWORD bufferSize; // buffer size of emulated routers (in packets)
	LinkProperties() { memset(this, 0, sizeof(*this)); }
};

class SenderSynHeader {
public:
	SenderDataHeader sdh;
	LinkProperties lp;
};


class SenderSocket {
public:
	//bool connection; //bool to pass into open, send, and close. 

	

	int Open(char* targetHost, int port, int senderWindow, LinkProperties *lp, bool connection) {
		
		if (connection == 1) {
			return ALREADY_CONNECTED;
		}
		clock_t time_elaps;
		time_elaps = clock();

		float rtpd = lp->RTT;
		float problossf = lp->pLoss[FORWARD_PATH];
		float problossr = lp->pLoss[RETURN_PATH];
		float bottleneck = lp->speed;
		DWORD bufferSize = lp->bufferSize;

		WSADATA wsaData;
		//Initialize WinSock; once per program run
		WORD wVersionRequested = MAKEWORD(2, 2);
		if (WSAStartup(wVersionRequested, &wsaData) != 0) {
			printf("WSAStartup error %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}
		
		SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == INVALID_SOCKET) {
			printf("socket() generated error %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}
		
		struct sockaddr_in local;
		memset(&local, 0, sizeof(local));
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		local.sin_port = htons(0);

		if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
			printf("socket() generated error %d\n", WSAGetLastError());// handle errors
		}

		struct sockaddr_in remote;
		memset(&remote, 0, sizeof(remote));
		remote.sin_family = AF_INET;
		if (inet_addr(targetHost) == INADDR_NONE) {
			//cout << "target host is a hostname" << endl;
			//not sure exactly how to do this, figure it out - i think this may be correct but unsure
			struct hostent* host;
			if ((host = gethostbyname(targetHost)) == NULL) { //if null is returned there is not a hostent structure and something is wrong, to find out specific use WSAGetLastError
				time_elaps = clock() - time_elaps;
				float loadTime = (time_elaps);
				printf("[ %.3f] --> target %s is invalid\n", loadTime, targetHost);
				return INVALID_NAME;
			}
			memcpy((char*)&(remote.sin_addr), host->h_addr, host->h_length);
			
		}
		else {
			//cout << "target host is an ip" << endl;
			remote.sin_addr.s_addr = inet_addr(targetHost); // server’s IP
		}

		remote.sin_port = htons(port); // DNS port on server
		connection = 1;

		int count = 0;

		clock_t times;
		clock_t temp;
		times = clock();
		while (count++ < MAX_ATTEMPTS)
		{
			
			SenderSynHeader* sh = new SenderSynHeader();
			sh->lp.RTT = rtpd;
			sh->lp.speed = 1e6 * bottleneck; // convert to megabits
			sh->lp.pLoss[FORWARD_PATH] = problossf;
			sh->lp.pLoss[RETURN_PATH] = problossr;
			sh->lp.bufferSize = bufferSize; // + max number of retransmissions r;

			sh->sdh.seq = 0;
			sh->sdh.flags.SYN = 0x1;
			int status;
			
			
			temp = clock() - times;
			float loadTimes = (temp);

			printf("[ %.3f] --> SYN 0 (attempt %i of 3, RTO 1.000) to %s\n", loadTimes / 1000, count, targetHost); //need to fix SYN to grab non static value, also need to get the targetHost ip
			if (int size = (sendto(sock, (char*)sh, sizeof(SenderSynHeader), 0, (sockaddr*)&remote, sizeof(remote))) == SOCKET_ERROR) {
				//printf(": sendto() generated error %d\n", WSAGetLastError());
				//closesocket(sock);
				status = FAILED_SEND;
				//cout << "you made it past here" << endl;
				return status;
			}
			else //if (size > 0) { //i think this actually need to be set to >0 but idk yet
				status = STATUS_OK;
				//cout << "you made it past here, size of send: " << size << endl;
			//}
			


			if (status != STATUS_OK) { //this does nothing yet, need to implement sendSyn()
				//return TIMEOUT; --> send this after 3 attempts 
				if (count == 3) {
					return TIMEOUT;
				}
				continue;
			}
			else if (status == STATUS_OK) { //need to ask for ack --------------------------------------------------------------------------code stuck here
				
				SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
				if (sock == INVALID_SOCKET) {
					printf("socket() generated error %d\n", WSAGetLastError());
					WSACleanup();
					exit(EXIT_FAILURE);
				}

				fd_set fd;
				FD_ZERO(&fd);
				FD_SET(sock, &fd);
				const timeval RTO = { 1,0 };
				clock_t synack_time;
				synack_time = clock();
				int good = select(0, &fd, NULL, NULL, &RTO);
				
				cout << "test: " << good << endl;
				if (good > 0) {
					char Rbuffer[sizeof(SenderSynHeader)];
					int length = sizeof(remote);
					auto Rbytes = recvfrom(sock, Rbuffer, sizeof(SenderSynHeader), 0, reinterpret_cast<struct sockaddr*>(&remote),
						&length);
					if (Rbytes == INVALID_SOCKET) {
						synack_time = clock() - synack_time;
						float loadTime = (synack_time);
						printf("[% .3f] <-- failed recvfrom with %d\n",loadTime, WSAGetLastError());
						//closesocket(sock);
						return FAILED_RECV;
					}
					ReceiverHeader *rh = (ReceiverHeader*)Rbuffer;
					if (rh->flags.SYN == 0x1 && rh->flags.ACK == 0x1) {
						//everything went good
						synack_time = clock() - synack_time; //this timing is still messed up, need to find the time from last sendto attempt to going recv working
						float loadTime = (synack_time);
						printf("[ %.3f] <-- SYN-ACK 0 window 1; setting initial RTO to %.3f", loadTime / 1000, 3 * loadTime / 1000);//fix SYN and window #
						return STATUS_OK;
					}

				}
				else if (&RTO) {

				}
				
				
			}
		}
	}

	int sendSyn() {
		//send syn to server  
		//get syn response, this should be in a function
		//if send == socket_error return FAILED_SEND
		//else return STATUS_OK

		//return TIMEOUT;
		return STATUS_OK; //temp
	}

	int Send(char* charBuf, int bytes, bool connection) {
		if (connection == 0) {
			return NOT_CONNECTED; // call to ss.Send()/Close() without ss.Open()
		}
		//else go ahead and do send function
		SenderSynHeader sh;
		return 0;
	}

	int Close(bool connection) {
		if (connection == 0) {
			return NOT_CONNECTED; // call to ss.Send()/Close() without ss.Open()
		}
		//else go ahead and do close function

		//connection = 0;
		return 0;
	}

};
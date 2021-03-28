// Hw3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <winsock.h>
#include <stdlib.h>
#include "SenderSocket.h"/

/*
                                                                                TODO

1. implement sendSYN(); for sending, need to fix SYN to grab non static value, also need to get the targetHost ip i instead of host name. For returning, fix SYN and window #. The clocks could be messed up on both
2. implement Send(); not really sure if this is different than sendSYN() yet
3. implement Close();

                                                                                Along the way
potentially fix bool connection to be a global variable that is updated within each function
figure out if the hostname is setup correctcly SenderSocket.h line 111
fixing timing for the SenderPacker, whenever the constructor is called the clock starts instead of telling it to start in Open()

*/

#pragma once
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
bool connection = 0; //bool to pass into open, send, and close. 


int main(int argc, char* argv[])
{
    if (argc != 8) {
        printf("Incorrect number of arguments. Arguments should be entered as follows:\n");
        printf("(hostname or IP), (power of 2 tranmitting buffer size), (sender window, in packets), (roud trip prop delay, in seconds), (probability of loss in each direction), (speed of the bottleneck link, in Mbps)\n");
        return 0;
    }

    //initialization of input arguments
    char* targetHost = argv[1];
    int power = atoi(argv[2]);
    int windowpacket = atoi(argv[3]);
    float rtpd = atof(argv[4]);
    float problossf = atof(argv[5]);
    float problossr = atof(argv[6]);
    float bottleneck = atof(argv[7]);

    

    printf("Main:\tsender W = %d, RTT %.3f sec, loss %.0e / %.4f, link %.0f Mbps\n", windowpacket, rtpd, problossf, problossr, bottleneck);
    printf("Main:\tinitializing DWORD array with 2^%i elements...", power);

    clock_t time_elaps;
    time_elaps = clock(); //timer for initializing dword array
    //found next section is the assignment pdf

    UINT64 dwordBufSize = (UINT64)1 << power;
    DWORD* dwordBuf = new DWORD[dwordBufSize];
    for (UINT64 i = 0; i < dwordBufSize; i++) {
        dwordBuf[i] = i;

    }
    time_elaps = clock() - time_elaps;
    float loadTime = (time_elaps);

    printf("done in %.0f ms\n", loadTime);

    //bool connection; //bool to pass into open, send, and close. 

    //printf("Host: %d\ntransBuffer: %d\nwindowpacket: %d\nrtpd: %f\nproblossforward: %.5f\nproblossreturn: %.4f\nbottleneck: %d\n", host, transBuffer, windowpacket, rtpd, problossf, problossr, bottleneck);
    LinkProperties lp;
    lp.RTT = rtpd;
    lp.speed = 1e6 * bottleneck; // convert to megabits
    lp.pLoss[FORWARD_PATH] = problossf;
    lp.pLoss[RETURN_PATH] = problossr;
    lp.bufferSize = windowpacket; // + max number of retransmissions r;

    SenderSocket ss;

    //connection = 0;

    int status = ss.Open(targetHost, MAGIC_PORT, windowpacket, &lp, connection);
    if (status != STATUS_OK) {
        if (status == ALREADY_CONNECTED) {
            cout << "Main:\tconnect failed with status 1" << endl;
            return status;
        }
        if (status == INVALID_NAME) {
            cout << "Main:\tconnect failed with status 3" << endl;
            return status;
        }if (status == TIMEOUT) {
            cout << "Main:\tconnect failed with status 5" << endl;
            return status;
        }
       
    }
    else(connection = 1);

    char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes 

    UINT64 off = 0; // current position in buffer
    while (off < byteBufferSize)
    {
        // decide the size of next chunk
        int bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
        // send chunk into socket
        if ((status = ss.Send(charBuf + off, bytes, connection)) != STATUS_OK) {
            if (status == FAILED_SEND) {
                cout << "sendto() failed in kernel" << endl;
                return 0;
            }
            else if (status == TIMEOUT) {
                cout << "timeout after all retx attempts are exhausted" << endl;
                return 0;
            }
            else if (status == FAILED_RECV) {
                cout << "recvfrom() failed in kernel" << endl;
                return 0;
            }
        }
            // error handing: print status and quit
            off += bytes;
    }
    if ((status = ss.Close(connection)) != STATUS_OK) {
        if (status == NOT_CONNECTED) {
            cout << "status code Not Connected when closed was called" << endl;
        }
    }
    else(connection = 0);

        // error handing: print status and quit
}

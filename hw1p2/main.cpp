/* main.cpp
 * Derek Grey, Section 500, Spring 2021
 */
#include "pch.h"
#include "URLParse.h"
#include "socket.h"
#include <iostream>
#include <fstream>


using namespace std;
// this class is passed to all threads, acts as shared memory
class Parameters {
public:
	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;
};

// function inside winsock.cpp
void winsock_test (URLParse url);
void UrlDownload(char* filename);

// this function is where threadA starts
UINT threadA (LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject (p->mutex, INFINITE);					// lock mutex
	printf ("threadA %d started\n", GetCurrentThreadId ());		// print inside critical section to avoid screen garbage
	Sleep (1000);												// sleep 1 second
	ReleaseMutex (p->mutex);									// release critical section

	// signal that this thread has finished its job
	ReleaseSemaphore (p->finished, 1, NULL);

	// wait for threadB to allow us to quit
	WaitForSingleObject (p->eventQuit, INFINITE);

	// print we're about to exit
	WaitForSingleObject (p->mutex, INFINITE);					
	printf ("threadA %d quitting on event\n", GetCurrentThreadId ());		
	ReleaseMutex (p->mutex);										

	return 0;
}

// this function is where threadB starts
UINT threadB (LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	// wait for both threadA threads to quit
	WaitForSingleObject (p->finished, INFINITE);
	WaitForSingleObject (p->finished, INFINITE);

	printf ("threadB woken up!\n");				// no need to sync as only threadB can print at this time
	Sleep (3000);

	printf ("threadB setting eventQuit\n"); 	// no need to sync as only threadB can print at this time

	// force other threads to quit
	SetEvent (p->eventQuit);

	return 0;
}

bool fexists(const char* filename)
{
	ifstream file(filename);
	if (file) {
		return 1;
	}else
		return 0;
}

int main(int argc, char* argv[])
{
	//cout << argv[1] << ":" << argv[2] << endl;
	if (argc == 2) {						//if there is only 1 argument perform p1
		string website = argv[1];
		URLParse ParsedURL = URLParse(website);
		winsock_test(ParsedURL);
	}
	else if (argc == 3) {					// if there is 2 arguments perform p2
		if (strcmp(argv[1], "1") == 0) {		//checking for string numbers, needs to be 1
			//printf("Correct number of strings\n");

			if (fexists(argv[2])) {
				//printf("file found");
				UrlDownload(argv[2]);
			}
			else {
				printf("specified file not found");
				return 0;
			}


		}
		else if (strcmp(argv[1], "1") != 0) {
			printf("Invalid Number of Threads, please speicfy 1 string");
			return 0;
		}
	}	
	return 0; 
}

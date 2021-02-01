/* main.cpp
 * Derek Grey, Section 500, Spring 2021
 */
#include "pch.h"
#include "URLParse.h"
#include "socket.h"


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

int main(int argc, char* argv[])
{
	string website = argv[1];
	URLParse ParsedURL = URLParse(website);
	winsock_test(ParsedURL);
	
	return 0; 
}

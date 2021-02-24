/* main.cpp
 * Derek Grey, Section 500, Spring 2021
 */
#include "pch.h"
#include "URLParse.h"
#include "socket.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_set>
#include "HTMLParserBase.h"
#pragma warning(disable : 4996)


using namespace std;
// this class is passed to all threads, acts as shared memory
struct Parameters {

	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;
	HANDLE urlListM;
	HANDLE listEmptyS;
	HANDLE listFullS;
	
	unordered_set<string> seenHosts;
	unordered_set<string> seenIps;
	queue<URLParse> urls;
	string inputFile;
	int numLines;
};

// function inside winsock.cpp
void winsock_test (URLParse url, bool args);
void UrlDownload(char* filename);

UINT fileSharedQueue(LPVOID queueParams) {
	Parameters* p = (Parameters*)queueParams;
	string fileLine;
	ifstream input(p->inputFile, ios::binary);
	HANDLE urlListM = p->urlListM;
	HANDLE urlListFullS = p->listFullS;
	HANDLE urlListEmptyS = p->listEmptyS;
	int numLines = p->numLines;


	input.seekg(0, ios::end);
	int fileSize = input.tellg();
	cout << "Opened " << p->inputFile << " with size " << fileSize <<  endl;

	for (int i = 0; i < numLines; i++){
		WaitForSingleObject(urlListEmptyS, INFINITE);
		p->urls.emplace(URLParse(fileLine));
		ReleaseSemaphore(urlListFullS, 1, NULL);
	}
	//printf("size is %i", p->urls.size());
	return 0;
}

UINT crawlThread(LPVOID crawlParams) {

	HTMLParserBase* parser = new HTMLParserBase;

	Parameters* p = (Parameters*)crawlParams;
	unordered_set<string>* seenHosts = &p->seenHosts;
	unordered_set<string>* seenIP = &p->seenHosts;

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
		if (!ParsedURL.Bscheme){
			printf("failed with invalid scheme");
		}
		winsock_test(ParsedURL, 0);
	}
	else if (argc == 3) {					// if there is 2 arguments perform p2
		if (strcmp(argv[1], "1") == 0) {		//checking for string numbers, needs to be 1
			//printf("Correct number of strings\n");

			if (fexists(argv[2])) {
				//printf("file found");
				ifstream file(argv[2], ios::binary);
				file.seekg(0, ios::end);
				int fileSize = file.tellg();
				cout << "Opened " << argv[2] << " with size " << fileSize << endl;
				///
				UrlDownload(argv[2]);
			}
			else {
				printf("specified file not found");
				return 0;
			}


		}
		else if (strcmp(argv[1], "1") != 0) { //perform p3
			
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
			parameters.numLines = numLines;
			parameters.urlListM = CreateMutex(NULL, 0, NULL);
			parameters.listEmptyS = CreateSemaphore(NULL, numLines, numLines, NULL);
			parameters.listFullS = CreateSemaphore(NULL, 1, numLines, NULL);
			parameters.urls = queue<URLParse>();
			parameters.inputFile = inputFile;
			
			
			// read file and populate shared queue
			HANDLE Queue = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fileSharedQueue, &parameters, 0, NULL);
			WaitForSingleObject(Queue, INFINITE);
			CloseHandle(Queue);
			printf("size of queue: %i", parameters.urls.size());

			// start stats thread
			HANDLE* crawlerHandle = new HANDLE[numThreads];
			for (int i = 0; i < numThreads; i++) {
				crawlerHandle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)crawlThread, &parameters, 0, NULL);
			}
			
			// start N crawling threads
			
			// wait for N crawling threads to finish
			
			// signal stats thread to quit; wait for it to terminate
			
			// cleanup 
			
			return 0;
		}
	}	
	return 0; 
}

#include <iostream>
#include <stdio.h>
#include "pch.h"
//#include "URLParse.cpp"
#include <fstream>
#include <unordered_set>
#pragma warning(disable : 4996)

using namespace std;
void winsock_test(URLParse url, bool args);

unordered_set<string> UniqueHost(string host, unordered_set<string> seenHosts) {
	//DWORD IP = inet_addr(url.host.c_str());
	//unordered_set<DWORD> seenIPs;
	//seenIPs.insert(IP);
	
	//---------
	//unordered_set<string> seenHosts;

	int prevSize = seenHosts.size();
	seenHosts.insert(host);
	if (seenHosts.size() > prevSize) { // unique host
		printf("passed\n");
	}
	else {
		printf("failed\n");

	}
		// duplicate host
	return seenHosts;
}

void UrlDownload(char* filename)
{
	unordered_set<string> seenHosts;
	unordered_set<string> seenIP;
	char mystring[200];
	FILE* pFile;
	pFile = fopen(filename, "r");
	while (!feof(pFile)) {

		if (fgets(mystring, 200, pFile)) {
			string website(mystring);
			URLParse ParsedURL = URLParse(website);
			printf("\tChecking host uniqueness. . . ");
			int prevSize = seenHosts.size();
			seenHosts = UniqueHost(ParsedURL.getURL(), seenHosts);
			if (seenHosts.size() > prevSize) { // unique host
				winsock_test(ParsedURL, 1);
			}
			else {
				

			}
			
		}

		
	}

	return;
}
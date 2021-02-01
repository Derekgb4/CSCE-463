#include <iostream>
#include <stdio.h>
#include "pch.h"
//#include "URLParse.cpp"
#include <fstream>
#include <unordered_set>
#pragma warning(disable : 4996)

using namespace std;

void UniqueHost(char * host) {
	//DWORD IP = inet_addr(url.host.c_str());
	unordered_set<DWORD> seenIPs;
	//seenIPs.insert(IP);
	
	//---------
	unordered_set<string> seenHosts;

	// populate with some initial elements

	//seenHosts.insert("www.google.com");
	//seenHosts.insert("www.tamu.edu");

	string test = host;
	//printf("url host: %d", host);
	int prevSize = seenHosts.size();
	seenHosts.insert(test);
	//printf("size of seen host: %d", seenHosts.size());
	if (seenHosts.size() > prevSize) { // unique host
		//printf("unique host\n");
	}
	else {
		printf("not a unique host\n");
	}
		// duplicate host
}

void UrlDownload(char* filename)
{
	URLParse ParsedURL;
	char mystring[200];
	FILE* pFile;
	pFile = fopen(filename, "r");
	while (!feof(pFile)) {

		if (fgets(mystring, 200, pFile)) {
			//puts(mystring);
			string website = mystring;
			URLParse ParsedURL = URLParse(website);
			//UniqueHost(ParsedURL.host);
			printf("host url: %d\n", ParsedURL.getBaseURL());
		}
		
		UniqueHost(ParsedURL.getBaseURL());
	}
	return;
}
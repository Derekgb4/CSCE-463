/* 
 * URL.h
 * Derek Grey, Section 500, Spring 2021
 *
 */

#pragma once
#include <string>


using namespace std;


class URLParse
{
public:
	int port;
	string host;
	string path;
	string query;
	string fragment;
	string scheme;
	string request;
	bool Bquery;
	bool Bpath;
	bool Bfragment;

	URLParse();
	~URLParse();

	URLParse(string url);
	char* getBaseURL();
	char* getFileName();
	string getURL();
	string getpath();
	string generateRequest(string requestType, URLParse url);
	
};
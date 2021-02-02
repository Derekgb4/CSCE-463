/* 
 * CPU.cpp
 * Derek Grey, Section 500, Spring 2021
 *
 * Obtains the current CPU utilization
 */
#include "pch.h"
#include "URLParse.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>



URLParse::URLParse() {
}


URLParse::URLParse(string url) {
	cout << "URL: " << url;
	cout << '\t' << "Parsing URL. . . ";
	if (url.substr(0, 7) == "http://") {
		scheme = "http://";
		url.erase(0, 7);
	}
	else {
		printf("Incorrect scheme");
		exit(EXIT_FAILURE);
	}

	if (url.find("#") == -1) {
		Bfragment = 0;
	}else {
		Bfragment = 1;
		fragment = url.substr(url.find("#") + 1);
		url.erase(url.find("#"));
	}

	if (url.find("?") == -1) {
		Bquery = 0;
	}else {
		Bquery = 1;
		query = url.substr(url.find("?") + 1);
		url.erase(url.find("?"));// = url.substr(0, url.find("?"));
	}
	
	if (url.find("/") == -1) {
		Bpath = 0;
		path = "/";
	}else if (url.find("/") > 0){
		Bpath = 1;
		path = url.substr(url.find("/") + 1);
		url = url.substr(0, url.find("/"));
	}

	if (url.find(":") == -1) {
		port = 80;
	}
	else if (url.find(":") > 0) {
		
		port = stoi(url.substr(url.find(":") + 1));
		url = url.substr(0, url.find(":"));
	}

	host = url;
	if (!Bpath) {
		request = path;
	}
	else {
		request = "/" + path;
	}
	
	if (Bquery) {
		request = request + "?" + query;
	}

	cout << "host " << host << ", port " << port << endl; // ", request " << request << endl;


}

URLParse::~URLParse() {
}

char* URLParse::getBaseURL() {
	ostringstream base;
	base << "http://" << host;
		string baseurl = base.str();
		char* baseUrl = new char[baseurl.length() + 1];
		strcpy_s(baseUrl, (int)strlen(baseurl.c_str()) + 1, baseurl.c_str());
		return baseUrl;
}

string URLParse::getURL() {
	//ostringstream base;
	//string base = host;
	return host;
}

char* URLParse::getFileName() {
	ostringstream base;
	base << host + ".html";
	string baseurl = base.str();
	char* FileName = new char[baseurl.length() + 1];
	strcpy_s(FileName, (int)strlen(baseurl.c_str()) + 1, baseurl.c_str());
	return FileName;
}
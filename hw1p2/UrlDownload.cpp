#include <iostream>
#include <stdio.h>
#include "pch.h"
#include <fstream>
#pragma warning(disable : 4996)

void UrlDownload(char* filename)
{
	char mystring[100];
	FILE* pFile;
	pFile = fopen(filename, "r");
	while (!feof(pFile)) {

		if (fgets(mystring, 100, pFile)) {
			//puts(mystring);
			string website = mystring;
			URLParse ParsedURL = URLParse(website);
		}
	}
	return;
}
// 463Hw2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
using namespace std;    

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Incorrect number of arguments. Need two arguments, the first is the lookup string and the second is the DNS server IP");
    }
    int type = 1;
    string lookup = argv[1];
    string query = argv[1];
    
    //determining the type of Query
    if (lookup.substr(3,1) == "." && lookup.substr(7,1) == "." && lookup.substr(11,1) == ".") {
        type = 12;
    }

    //getting the reverse dns lookup if needed
    if (type == 12) {
        query = query.substr(12,2) + "." +  query.substr(8,3) + "." + query.substr(4,3) + "." + query.substr(0,3) + ".in-addr.arpa";
    }
 
    //printf("Lookup  : %s\nQuery   : %s, type %i, TXID \nServer  : %s\n*******************************************\n", lookup, query, type, argv[2]);
    cout << "Lookup  : " << lookup << endl << "Query   : " << query << ", type " << type << ", TXID " << endl << "Server  : " << argv[2] << endl << "***********************************" << endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "netreqchannel.hpp"

using namespace std;

string int2string(int n)
{
    stringstream ss;
    ss << n;
    return ss.str();
}

string servRead(int* fileDesc)
{
    char buff[255];
    read(*fileDesc,buff,255);
    string s = buff;
    return s;
}
int servWrite(int * fileDesc, string m)
{
    if(m.length()>=255)
    {
        cout << "Message too big" << endl;
        exit(1);
    }
    if(write(*fileDesc, m.c_str(), m.length()+1) < 0)
    {
        cout << "Error writing" << endl;
        exit(1);
    }
}
void processHello(int * fileDesc, const string& _request)
{
    servWrite(fileDesc, "Hello");
    return;
}

void processData(int * fileDesc, const string & _reqeust)
{
    usleep(1000 + (rand()%5000));

    servWrite(fileDesc, int2string(rand()%100));
    return;   
}

void processRequest(int *fileDesc, const string & _request)
{
    if (_request.compare(0, 5, "hello") == 0)
        processHello(fileDesc, _request);

    else if(_request.compare(0, 4, "data") == 0)
        processData(fileDesc, _request);
    return;
}

void *connection_handler(void * arg)
{
    int * fileDesc = (int*)arg;

    if(fileDesc == NULL)
    {
        cout << "File descriptor is null" << endl;
        exit(1);
    }
    while(true)
    {
        string request = servRead(fileDesc);

        if(request.compare("quit")==0)
        {
            servWrite(fileDesc, "Later.");
            usleep(6000);
            break;
        }

        processRequest(fileDesc,request);
    }
    cout << "Connection closed." << endl;

}

int main(int argc, char* argv[])
{
    int backlog = 100;
    unsigned short portNum = 9951;

    int c = 0;
    while ((c = getopt(argc, argv, "p:b:")) != -1)
    {
        switch(c)
        {
            case 'p':
                portNum = atoi(optarg);
                break;
            case 'b':
                backlog = atoi(optarg);
                break;
            case '?':
                cout << "Unknown options" << endl;
                abort();
            default:
                cout << "Command line error" << endl;
                abort();
        }
    }

    cout << "Server started on port " << portNum << endl;
    cout << "Backlog size is " << backlog << endl;

    NetworkRequestChannel server(portNum, connection_handler, backlog);

    server.~NetworkRequestChannel();
    return 0;
}
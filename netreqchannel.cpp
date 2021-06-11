#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#include "netreqchannel.hpp"

using namespace std;

void errorMsg();
int createClientConnection(const char* host, const char* portNum);
int createServerConnection(const char* svc, int backlog);

struct sockaddr_in serverIn;

NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no)
{
    stringstream ss;
    ss << _port_no;
    string port = ss.str();

    struct sockaddr_in sockIn;
    memset(&sockIn, 0, sizeof(sockIn));
    sockIn.sin_family = AF_INET;

    if(struct servent * pse = getservbyname(port.c_str(), "tcp"))
    {
        sockIn.sin_port = pse->s_port;
    }
    
    else if((sockIn.sin_port = htons((unsigned short) atoi (port.c_str())))==0)
    {
        cout << "Can't connect to the port" << endl;
        exit(1);
    }
    if(struct hostent* hn = gethostbyname(_server_host_name.c_str()))
    {
        memcpy(&sockIn.sin_addr, hn->h_addr, hn->h_length);
    }
    else if((sockIn.sin_addr.s_addr = inet_addr(_server_host_name.c_str()))==INADDR_NONE)
    {
        cout << "Can't determine the host " << _server_host_name.c_str() << endl;
        exit(1);
    }
    
    int netSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(netSocket < 0)
    {
        cout << "Can't create socket" << endl;
        exit(1);
    }

    // cout << connect(netSocket, (struct sockaddr *)&sockIn, sizeof(sockIn)) << endl;
    if(connect(netSocket, (struct sockaddr *)&sockIn, sizeof(sockIn)) < 0)
    {
        cout << "Can't connect to host " << _server_host_name.c_str() << " with port number " << port.c_str() << endl;
        // exit(1);
    }

    fd = netSocket;

    //fd = createClientConnection(_server_host_name.c_str(), port.c_str());
}

NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void * (connection_handler) (void *), int backlog)
{
    stringstream ss;
    ss << _port_no;
    string port = ss.str();

    // int master = createServerConnection(port.c_str()); rewrite this

    memset(&serverIn, 0, sizeof(serverIn));
    serverIn.sin_family = AF_INET;
    serverIn.sin_addr.s_addr = INADDR_ANY;

    if(struct servent* pse = getservbyname(port.c_str(), "tcp"))
    {
        serverIn.sin_port = pse->s_port;
    }
    else if((serverIn.sin_port = htons((unsigned short)atoi(port.c_str())))==0)
    {
        cout << "Can't get port" << endl;
        exit(1);
    }
    int socketNum = socket(AF_INET, SOCK_STREAM , 0);

    if(socketNum < 0)
    {
        cout << "Can't create socket" << endl;
        exit(1);
    }
    if(bind(socketNum, (struct sockaddr*)&serverIn, sizeof(serverIn))<0)
    {
        cout << "Cannot bind" << endl;
        exit(1);
    }
    listen(socketNum, backlog);

    int master = socketNum;

    int serverSize = sizeof(serverIn);

    while(true)
    {
        int * slave = new int;
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        *slave = accept(master,(struct sockaddr*)&serverIn, (socklen_t*)&serverSize);
        if(slave<0)
        {
            delete slave;
            if (errno == EINTR)
                continue;
            else
            {
                cout << "Error" << endl;
                exit(1);
            }
        }
        pthread_create(&thread, &attr, connection_handler, (int*)slave);
    }
}

NetworkRequestChannel::~NetworkRequestChannel()
{
    close(fd);
}

string NetworkRequestChannel::send_request(string _request)
{
    cwrite(_request);
    string s = cread();
    return s;
}

int NetworkRequestChannel::cwrite(string _msg)
{
    if(_msg.length()>=256)
    {
        cout << "Message too long" << endl;
        return -1;
    }
    const char * s = _msg.c_str();
    if(write(fd, s, strlen(s) + 1) < 0)
    {
        cout << "Problems with writing" << endl;
        exit(1);
    }
}

string NetworkRequestChannel::cread()
{
    char buff[256];
    if(read(fd, buff, 256) < 0)
    {
        cout << "Problems reading" << endl;
        // exit(1);
    }
    string s = buff;
    return s;
}

int NetworkRequestChannel::read_fd() {
  return fd;
}
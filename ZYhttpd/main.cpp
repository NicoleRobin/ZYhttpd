//*****************************************************************************
//@ProjectName      ZYhttpd
//@Description      my http server
//@Author           NicoleRobin
//@Date             2015/02/09
//*****************************************************************************
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
 
#define BUFFER_SIZE 1024
#define HOST "127.0.0.1"
#define INDEX "index.html"
#define PORT 9000
#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Server: ZYhttp_v1.0.1\r\n\
Content-Length: %lld\r\n\r\n\
"

// get file size
long long GetFileLength(string strPath);
// thread function
void *thread(void *arg);
// parse url
string ParseUrl(string strRecv);
 
int main(int argc, char **argv)
{
    // define and init an server sockaddr
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
	// addrServer.sin_addr.s_addr = inet_addr("192.168.0.160");
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(PORT);
	memset(addrServer.sin_zero, 0, 8);
 
    // create socket
	int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServer == -1)
    {
        printf("Create socket error!");
        return 1;
    }
 
    // bind server socket host
    if (-1 == bind(socketServer, (sockaddr*)&addrServer, sizeof(addrServer)))
    {
        printf("Bind server host failed!\nerrno : %d\n", errno);
        return 1;
    }
 
    // listen
    printf("Listening on port: %d ... \n", PORT);
    if (-1 == listen(socketServer, 10))
    {
        printf("Listen failed!");
        return 1;
    }
 
	pthread_t pid;
    while (true)
    {
        sockaddr_in addrClient;
        int nClientAddrLen = sizeof(addrClient);
 
        int socketClient = accept(socketServer, (sockaddr*)&addrClient, (socklen_t*)&nClientAddrLen);
        if (socketClient == -1)
		{
			printf("Accept failed!");
			break;
		}
		else
		{
			if (0 != pthread_create(&pid, NULL, thread, &socketClient))
			{
				printf("Create thread failed, errno : %d, error : %s\n", errno, strerror(errno));
			}
		}
    }
    close(socketServer);
 
    return 0;
}
 
long long GetFileLength(string strPath)
{
    ifstream fin(strPath.c_str(), ios::in | ios::binary);
    fin.seekg(0, ios_base::end);
    streampos pos = fin.tellg();
    long long llSize = static_cast<long long>(pos);
    fin.close();
    return llSize;
}

string ParseUrl(string strRecv)
{
	string strMethod, strUrl, strHeader;
	istringstream sIn(strRecv);
	sIn >> strMethod >> strUrl >> strHeader;
	if ((!strUrl.empty()) && (strUrl[0] == '/'))
	{
		strUrl = strUrl.substr(1, strUrl.length() - 1);
#ifdef DEBUG
		printf("%s\n", strUrl.c_str());
#endif
	}
	return strUrl;
}

void *thread(void *arg)
{
	int socketClient = *((int*)arg);
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(socketClient, buffer, BUFFER_SIZE, 0) < 0)
	{
		printf("Recvive data failed!");
	}
	else
	{
#ifdef DEBUG
		printf("%s\n", buffer);
#endif
	}
	
	string strPath = ParseUrl(buffer);
	if (strPath.empty())
	{
		strPath = INDEX;
	}
	// response 
	// send header
	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, HEADER, GetFileLength(strPath));
	if (send(socketClient, buffer, strlen(buffer), 0) < 0)
	{
		printf("Send data failed!");
	}
	ifstream fin(strPath.c_str(), ios::in | ios::binary);
	if (!fin.is_open())
	{
		fin.open(INDEX, ios::in | ios::binary);
	}
	memset(buffer, 0, BUFFER_SIZE);
	while (fin.read(buffer, BUFFER_SIZE - 1))
	{
		if (send(socketClient, buffer, strlen(buffer), 0) < 0)
		{
			printf("Send data failed!");
		}
		memset(buffer, 0, BUFFER_SIZE);
	}
	if (send(socketClient, buffer, strlen(buffer), 0) < 0)
	{
		printf("Send data failed!");
	}
	
	fin.close();
	close(socketClient);
	return (void*)0;
}

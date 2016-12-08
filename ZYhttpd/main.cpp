//*****************************************************************************
//@ProjectName      ZYhttpd
//@Description      my http server
//@Author           NicoleRobin
//@Date             2015/02/09
//*****************************************************************************
#include <unistd.h>
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
#include <map>

#include "ZYLog.h"

using namespace std;
 
#define BUFFER_SIZE (1024 * 1024)
#define HOST "127.0.0.1"
#define ERROR "error.html"
#define INDEX "index.html"
#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Server: ZYhttp_v1.0.1\r\n\
Date: Thu, 08 Dec 2016 03:03:17 GMT\r\n\
Content-Length: %lld\r\n\r\n\
"
#define PNG_HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: image/png\r\n\
Server: ZYhttp_v1.0.1\r\n\
Date: Thu, 08 Dec 2016 03:03:17 GMT\r\n\
Content-Length: %lld\r\n\r\n\
"

static map<string, string> s_mapContentType;

// get file size
long long GetFileLength(string strPath);
// thread function
void *thread(void *arg);
// parse url
string ParseUrl(string strRecv);
// parse postfix
string ParsePostfix(string strPath);
// init content type map
void InitContentTypeMap();

int main(int argc, char **argv)
{
	int iPort = 0;
	if (argc == 2)
	{
		iPort = atoi(argv[1]);
	}
	else if (argc == 3)
	{
		if (strcmp(argv[1], "-D") == 0)
		{
			int iRet = daemon(1, 1);
			if (iRet != 0)
			{
				LOG_ERROR("daemon failed, errno[%d], error[%s]", errno, strerror(errno));
				return -1;
			}

		}
		else
		{
			LOG_ERROR("Error parameter[%s]", argv[1]);
			return -1;
		}
		iPort = atoi(argv[2]);
	}
	else
	{
		LOG_ERROR("Usage:%s port or %s -D port", basename(argv[0]), basename(argv[0]));
		return -1;
	}

    // define and init an server sockaddr
    sockaddr_in addrServer;
	memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(iPort);
 
    // create socket
	int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServer == -1)
    {
        LOG_ERROR("Create socket error, errno[%d], error[%s]!", errno, strerror(errno));
        return 1;
    }

	// reuse addr
	int iOpt = 1;
	if (-1 == setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt)))
	{
		LOG_ERROR("setsockopt error, errno[%d], error[%s]!", errno, strerror(errno));
		return 1;
	}
 
    // bind server socket host
    if (-1 == bind(socketServer, (sockaddr*)&addrServer, sizeof(addrServer)))
    {
        LOG_ERROR("Bind server host failed, errno[%d], error[%s]!", errno, strerror(errno));
        return 1;
    }
 
    // listen
    LOG_ERROR("Listening on port: %d ... ", iPort);
    if (-1 == listen(socketServer, 10))
    {
        LOG_ERROR("Listen failed!");
        return 1;
    }
 
	pthread_t pid;
    while (true)
    {
        sockaddr_in addrClient;
        int nClientAddrLen = sizeof(addrClient);
 
		int *pSocket = new int;
        *pSocket = accept(socketServer, (sockaddr*)&addrClient, (socklen_t*)&nClientAddrLen);
		if (*pSocket == -1)
		{
			LOG_ERROR("Accept failed!");
			break;
		}
		else
		{
			LOG_DEBUG("Recv new connect, socket[%d]", *pSocket);
			if (0 != pthread_create(&pid, NULL, thread, (void*)pSocket))
			{
				LOG_ERROR("Create thread failed, errno : %d, error : %s", errno, strerror(errno));
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
	}
	return strUrl;
}

string ParsePostfix(string strPath)
{
	string strPostfix;
	string::size_type sizeIndex = strPath.find_last_of('.');
	strPostfix = strPath.substr(sizeIndex + 1);

	return strPostfix;
}

void *thread(void *arg)
{
	LOG_DEBUG("In thread");
	int iSendCnt = 0;
	int iRecvCnt = 0;
	int socketClient = *(int*)arg;
	delete (int*)arg;
	char szRecvBuffer[BUFFER_SIZE];
	char szSendBuffer[BUFFER_SIZE];
	memset(szRecvBuffer, 0, BUFFER_SIZE);
	if ((iRecvCnt = recv(socketClient, szRecvBuffer, BUFFER_SIZE - 1, 0)) < 0)
	{
		LOG_ERROR("Receive data error, socket[%d], errno[%d], error[%s]", socketClient, errno, strerror(errno));
		close(socketClient);
		return (void*)0;
	}
	else
	{
		LOG_DEBUG("Recv data succ, socket[%d], data length[%d]", socketClient, iRecvCnt);
	}
	
	string strPath = ParseUrl(szRecvBuffer);
	if (strPath.empty())
	{
		strPath = INDEX;
	}
	LOG_DEBUG("strPath=%s", strPath.c_str());

	// response 
	// send header
	string strPostfix = ParsePostfix(strPath);
	LOG_DEBUG("strPostfix=%s", strPostfix.c_str());
	if (strPostfix.compare("html") == 0)
	{
		memset(szSendBuffer, 0, BUFFER_SIZE);
		sprintf(szSendBuffer, HEADER, GetFileLength(strPath));
		if ((iSendCnt = send(socketClient, szSendBuffer, strlen(szSendBuffer), 0)) < 0)
		{
			LOG_ERROR("Send data error, socket[%d], errno[%d], error[%s]!", socketClient, errno, strerror(errno));
			close(socketClient);
			return (void*)0;
		}
		else
		{
			LOG_DEBUG("Send data succ, socket[%d], data length[%d]", socketClient, iSendCnt);
		}
	}
	else if (strPostfix.compare("png") == 0)
	{
		memset(szSendBuffer, 0, BUFFER_SIZE);
		sprintf(szSendBuffer, PNG_HEADER, GetFileLength(strPath));
		if ((iSendCnt = send(socketClient, szSendBuffer, strlen(szSendBuffer), 0)) < 0)
		{
			LOG_ERROR("Send data error, socket[%d], errno[%d], error[%s]!", socketClient, errno, strerror(errno));
			close(socketClient);
			return (void*)0;
		}
		else
		{
			LOG_DEBUG("Send data succ, socket[%d], data length[%d]", socketClient, iSendCnt);
		}
	}
	
	ifstream fin(strPath.c_str(), ios::in | ios::binary);
	if (!fin.is_open())
	{
		fin.open(ERROR, ios::in | ios::binary);
	}

	while (true)
	{
		memset(szSendBuffer, 0, BUFFER_SIZE);
		fin.read(szSendBuffer, BUFFER_SIZE - 1);
		if (fin.eof())
		{
			break;
		}
		else
		{
			LOG_DEBUG("read %d data from file[%s]", fin.gcount(), strPath.c_str());
		}

		if ((iSendCnt = send(socketClient, szSendBuffer, fin.gcount(), 0)) < 0)
		{
			LOG_ERROR("Send data error, socket[%d], errno[%d], error[%s]!", socketClient, errno, strerror(errno));
			close(socketClient);
			return (void*)0;
		}
		else
		{
			LOG_DEBUG("Send data succ, socket[%d], data length[%d]", socketClient, iSendCnt);
		}
	}

	if ((iSendCnt = send(socketClient, szSendBuffer, fin.gcount(), 0)) < 0)
	{
		LOG_ERROR("Send data error, socket[%d], errno[%d], error[%s]!", socketClient, errno, strerror(errno));
		close(socketClient);
		return (void*)0;
	}
	else
	{
		LOG_DEBUG("Send data succ, socket[%d], data length[%d]", socketClient, iSendCnt);
	}
	
	fin.close();
	close(socketClient);
	return (void*)0;
}

void InitContentTypeMap()
{
	s_mapContentType.insert(make_pair<string, string>("html", "text/html"));
	s_mapContentType.insert(make_pair<string, string>("png", "img/"));
}

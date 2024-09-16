
#include </opt/itchTest/MarketDataReceiver-2/connection.h>
#include </opt/itchTest/MarketDataReceiver-2/itchDefinitions.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
namespace Lib
{
	int CConnection::createUdpSocket(const char *localIp, int timeoutSec)
	{
		int reusePort = 1;
		int sd;
		struct sockaddr_in localSocket;

		sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sd < 0)
		{
			return -1;
		}

		if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reusePort, sizeof(reusePort)) < 0)
		{
			close(sd);
			return -2;
		}

		struct timeval tv;
		tv.tv_sec = timeoutSec;
		tv.tv_usec = 0;

		if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		{
			close(sd);
			return -4;
		}

		memset(&localSocket, 0, sizeof(localSocket));
		localSocket.sin_family = AF_INET;
		localSocket.sin_port = 0;
		localSocket.sin_addr.s_addr = inet_addr(localIp);
		if (bind(sd, (struct sockaddr *) &localSocket, sizeof(localSocket)))
		{
			close(sd);
			return -3;
		}

		return sd;
	}
	int CConnection::createUdpMulticastSocket(const char *remoteIp, const int remotePort, const char *localIp)
	{
		int reusePort = 1;
		int sd;
		struct sockaddr_in localSocket;
		struct ip_mreq group;

		sd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sd < 0)
		{
			printf("Socket creation error\n");
            fflush(stdout);
            return -1;
		}
        int setOptionError = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reusePort, sizeof(reusePort));
		if (setOptionError < 0)
		{
			printf("Socket set option error\n");
			fflush(stdout);
            close(sd);
			return -2;
		}

		memset(&localSocket, 0, sizeof(localSocket));
		localSocket.sin_family = AF_INET;
		localSocket.sin_port = htons(remotePort);
		localSocket.sin_addr.s_addr = INADDR_ANY;
        
		if (bind(sd, (struct sockaddr *) &localSocket, sizeof(localSocket)))
		{
            printf("Socket bind error\n");
			fflush(stdout);			
            close(sd);
			return -3;
		}

		group.imr_multiaddr.s_addr = inet_addr(remoteIp);
		group.imr_interface.s_addr = inet_addr(localIp);
		int error = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));
		if (error < 0)
		{
            printf("Socket connection error\n");
			fflush(stdout);			
            close(sd);
			return -4;
		}

		return sd;
	}
	
	
}
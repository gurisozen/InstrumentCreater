#include </opt/itchTest/InstrumentCreater/connection.h>
#include </opt/itchTest/InstrumentCreater/itchDefinitions.h>
#include </opt/itchTest/InstrumentCreater/instrumentCreater.h>
#include </opt/itchTest/Configs/configuration.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <pthread.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>
#include <fstream>
#include <fcntl.h>
#include </opt/itchTest/InstrumentCreater/instrumentDefinitions.h>

#ifdef __CYGWIN__
#define pthread_setname_np(...) ;
#define pthread_attr_setaffinity_np(...) 0
#define cpu_set_t int
#define CPU_ZERO(...) ;
#define CPU_SET(...) ;
#endif

#define OUTPUT_DEBUG_FILE

InstrumentCreater::InstrumentCreater(int part)
{
    //start everything
	_continue = 1;
	//sequence set
	_lastHbSequence = 1;
	_nextProcessSequence = 1;
	//rewinder set
	//_rewinderStart = 0;
	//_rewinderStartSequence = 1;
	//_rewinderEndSequence = 1;

	
	//mutexes ready
	//pthread_mutex_init(&_rewinderMutex, 0);
	//pthread_cond_init(&_rewinderCond, 0);
	//set buffers
	memset(_buffer, 0, sizeof(_buffer));
	//memset(_rewinderBuffer, 0, sizeof(_rewinderBuffer));
	memset(_itchMessageList, 0, sizeof(_itchMessageList));
	//memset(&_rewinderMessage, 0, sizeof(_rewinderMessage));
	//set pointers
	_bufferPtr = _buffer;
	//_rewinderBufferPtr = _rewinderBuffer;
	//set output file
	outputFile = 0;
	//thread set
	//_multicastThread = 0;
	_processorThread = 0;
	//_rewinderThread = 0;
	
	//set sockets
	//_multicastSocket = 0;
	//_rewinderSocket = 0;
    std::string basePath = "/opt/itchTest/Configs/InstrumentCreater/InstrumentCreater";
    std::string fileExtension = ".config";

    configFile = basePath + std::to_string(part) + fileExtension;
    configFile = basePath + std::to_string(part) + fileExtension;
	//directory = configManager.readConfig(configFile, "directory");
    //rewinderPort = std::stoi(configManager.readConfig(configFile, "rewinderPort"));
    //rewinderLocalIp = configManager.readConfig(configFile, "rewinderLocalIp");
    //multicastPort = std::stoi(configManager.readConfig(configFile, "multicastPort"));
    //multicastLocalIp = configManager.readConfig(configFile, "multicastLocalIp");
    //multicastRemoteIp = configManager.readConfig(configFile, "multicastRemoteIp");
    //rewinderRemoteIp = configManager.readConfig(configFile, "rewinderRemoteIp");
	//printf("\n");
	//printf(rewinderLocalIp);
	//printf("\n");
	//printf(rewinderRemoteIp);
	//printf("\n");
	//printf("%d",rewinderPort);
	//printf("\n");
	//printf(multicastLocalIp);
	//printf("\n");
	//printf(multicastRemoteIp);
	//printf("\n");
	//printf("%d",multicastPort);
	//printf("\n");
    //get partition
	partition = part;
}
static void *startMulticastThread(void *obj)
{
	((InstrumentCreater*) obj)->multicastThreadHandler();
	return 0;
}

static void *startProcessorThread(void *obj)
{
	((InstrumentCreater*) obj)->processorThreadHandler();
	return 0;
}

static void *startRewinderThread(void *obj)
{
	((InstrumentCreater*) obj)->rewinderThreadHandler();
	return 0;
}


int InstrumentCreater::init()
{
    sender->init();
	//CREATE SOCKETS
	//create multicast socket
	//_multicastSocket = Lib::CConnection::createUdpMulticastSocket(multicastRemoteIp, multicastPort, multicastLocalIp);
	//if (_multicastSocket  <= 0)
	//{
	//	std::cerr << "Multicast Socket NOT created " << _multicastSocket  << std::endl;
	//	return -1;
	//}
	//else
	//{
	//	std::cout << "Multicast Socket created " << _multicastSocket  << std::endl; 
	//}
	//create rewinder socket
	//_rewinderSocket = Lib::CConnection::createUdpSocket(rewinderLocalIp, 5);
	//if (_rewinderSocket < 0)
	//{
	//	std::cerr << "Rewinder Socket NOT created " << _multicastSocket  << std::endl;
	//	return -2;
	//}
	//else
	//{
	//	std::cout << "Rewinder Socket created " << _multicastSocket  << std::endl; 
	//}
	//START THREADS//
	//start multicast thread
	
	
	//if (pthread_create(&_multicastThread, nullptr, startMulticastThread, this)< 0)
	//{
		//std::cerr << "Multicast Thread not created " << _multicastSocket  << std::endl;
	//}
	//else
	//{
	//	std::cout << "Multicast Thread created" << std::endl;
	//}
	//start rewinder thread
	//if (pthread_create(&_rewinderThread, 0, startRewinderThread, this) < 0)
	//{
	//	std::cerr << "Rewinder Thread not created " << _multicastSocket  << std::endl;
	//}
	//else
	//{
	//	std::cout << "Rewinder Thread created" << std::endl;
	//}
	//start processor thread
	if (pthread_create(&_processorThread, nullptr, startProcessorThread, this) < 0)
	{
		std::cerr << "Processor Thread not created " << _multicastSocket  << std::endl;
	}
	else
	{
		std::cout << "Processor Thread created" << std::endl;
	}
	
	return 0;
}
void InstrumentCreater::setMarketInstruments(const char *filename)
{
	int hasOldData = 0;
	FILE *f = fopen(filename, "rb");
	ulong sequence = 1;
	ushort messageCount;
	Itch::TItchMessage *firstMessage = 0;
	Itch::TItchMessage *msg = 0;
	Itch::TItchMessageBase *msgBase = 0;
	if (f)
	{
		hasOldData = 1;
		fseek(f, 0, SEEK_END);
		ulong fileSize = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		fread(_buffer, fileSize, 1, f);
		fclose(f);
		Itch::TItchBlock *block = (Itch::TItchBlock*) _buffer;
		Itch::TItchMessage *msg;
		Itch::TItchMessageBase *msgBase;
		ulong lastSequence;
		char *msgAddr;
		char *bufferPtr = _buffer;
		int orderBookId;
		ushort messageLength = 0;
		while (((char*)block) - _buffer < fileSize)
		{
			
			sequence = be64toh(block->sequenceNumber);
			_lastHbSequence = sequence;
			messageCount = be16toh(block->messageCount);
			//printf("Multicast Seq Num %d\n",sequence);
			//printf("Multicast Message count %d\n",messageCount);
			firstMessage = _itchMessageList + sequence;
			lastSequence = sequence + htobe16(block->messageCount);
			firstMessage->lastMessage = _itchMessageList + lastSequence - 1;
			msgAddr = bufferPtr + sizeof(Itch::TItchBlock);
			for (; sequence < lastSequence; ++sequence)
			{
				msg = _itchMessageList + sequence;
				msgBase = (Itch::TItchMessageBase*) msgAddr;
				messageLength = be16toh(msgBase->length);
				msg->addr = msgAddr;
				msgAddr += messageLength + sizeof(ushort);
				msg->state = Itch::EItchMessageState::Read;
			}
			bufferPtr = msgAddr;
			block = (Itch::TItchBlock*) bufferPtr;
		}
		_bufferPtr = bufferPtr;
	}
	lastMessageFromFile = sequence;
	
}
void InstrumentCreater::start()
{
	char filename[1024] = {0};
	char timestamp[9] = {0};  
	// Get the current time
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	// Format the date as YYYYMMDD
	strftime(timestamp, sizeof(timestamp), "%Y%m%d", t);
	sprintf(filename, "/opt/itchTest/itchFiles/itch.%d.%s.dat",partition,timestamp);
	printf("Opening file for writing: ");
	printf(filename);
	printf("\n");
	setMarketInstruments(filename);
	init();
	startConnect();
}
void InstrumentCreater::stop()
{
	disconnect();
}
void InstrumentCreater::startConnect()
{
	
	//if (pthread_join(_multicastThread, nullptr))
    //{
        //std::cerr << "Failed to join multicast thread. Error: " << strerror(errno) << std::endl;
    //}
	//else
	//{
		//std::cout << "Multicast Thread Joined" << std::endl;
	//}

	//if (pthread_join(_rewinderThread, nullptr))
    //{
        //std::cerr << "Failed to join rewinder thread. Error: " << strerror(errno) << std::endl;
    //}
	//else
	//{
		//std::cout << "Rewinder Thread created" << std::endl;
	//}

	if (pthread_join(_processorThread, nullptr))
    {
        std::cerr << "Failed to Processor thread. Error: " << strerror(errno) << std::endl;
    }
	else
	{
		std::cout << "Processor Thread created" << std::endl;
	}
		
}
void InstrumentCreater::disconnect()
{
	_continue = 0;
	if (_multicastSocket > 0)
	{
		close(_multicastSocket);
		_multicastSocket = 0;
	}
	if (_rewinderSocket > 0)
	{
		close(_rewinderSocket);
		_rewinderSocket = 0;
	}
}
void InstrumentCreater::waitRewinder(ulong *startSequence, ulong *endSequence)
{
    pthread_mutex_lock(&_rewinderMutex);
	while (!_rewinderStart) pthread_cond_wait(&_rewinderCond, &_rewinderMutex);
	_rewinderRunning = 1;
	*startSequence = _rewinderStartSequence;
	*endSequence = _rewinderEndSequence;
	_rewinderStart = 0;
	fflush(stdout);
	pthread_mutex_unlock(&_rewinderMutex);
}

void InstrumentCreater::startRewinder(ulong startSequence, ulong endSequence)
{
    pthread_mutex_lock(&_rewinderMutex);
	_rewinderStartSequence = startSequence;
	_rewinderEndSequence = endSequence;
	
	_rewinderStart = 1;
	_rewinderRunning = 1;
	
	pthread_cond_signal(&_rewinderCond);
	pthread_mutex_unlock(&_rewinderMutex);
}

void InstrumentCreater::rewinderThreadHandler()
{
    struct sockaddr_in remoteAddr;
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(rewinderPort); // Replace with your port number
    remoteAddr.sin_addr.s_addr = inet_addr(rewinderRemoteIp);
	socklen_t senderAddrLen = sizeof(remoteAddr);
	
	_rewinderBufferPtr = _rewinderBuffer;
	ulong startSequence = 0;
	ulong endSequence = 0;
	Itch::TItchMessage *firstMessage = 0;
	Itch::TItchBlock *block = 0;
	ulong sequence = 0;
	char *msgAddr;
	ushort messageCount;
	ulong lastSequence;
	ushort messageLength;
	Itch::TItchMessage *msg = 0;
	Itch::TItchMessageBase *msgBase = 0;
	ulong count = 0;
	int readLen = 0;
	int bytesSent = 0;

    while (_continue)
	{
		waitRewinder(&startSequence, &endSequence);
		if (!_continue) break;
		_rewinderMessage.sequence = htobe64(startSequence);
		//printf("Start Seq Num %d\n",startSequence);
		//printf("End Seq Num %d\n",endSequence);
		count = endSequence - startSequence;
		
		//printf("Message Session  %s\n",_rewinderMessage.session);
		if (count > 32767) count = 32767;
		//printf("Message Count  %d\n",count);
		_rewinderMessage.count = htobe16(count);
		fflush(stdout);
		bytesSent = sendto(_rewinderSocket, (char *)&_rewinderMessage, sizeof(_rewinderMessage), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
		//printf("Bytes Sent To Rewinder %d\n",bytesSent);
		readLen = recvfrom(_rewinderSocket, _rewinderBufferPtr, 65536, 0, (struct sockaddr *)&remoteAddr, &senderAddrLen);
		if (readLen > 0)
		{
			block = (Itch::TItchBlock*) _rewinderBufferPtr;
			sequence = be64toh(block->sequenceNumber);
			//printf("Rewinder Seq Num %d\n",sequence);
			//printf("Rewinder Message Count  %d\n",be16toh(block->messageCount));
			firstMessage = _itchMessageList + sequence;
			firstMessage->rewinder = 1;
			if (firstMessage->state == Itch::EItchMessageState::None)
			{
				msgAddr = _rewinderBufferPtr + sizeof(Itch::TItchBlock);
				messageCount = be16toh(block->messageCount);
				lastSequence = sequence + messageCount;
				firstMessage->lastMessage = _itchMessageList + lastSequence - 1;
				for (; sequence < lastSequence; ++sequence)
				{
					msg = _itchMessageList + sequence;
					msgBase = (Itch::TItchMessageBase*) msgAddr;
					messageLength = be16toh(msgBase->length);
					msg->addr = msgAddr;
					msgAddr += messageLength + sizeof(ushort);
					if (msg != firstMessage)
					{
						msg->state = Itch::EItchMessageState::Read;
					}
				}
				if (firstMessage->state != Itch::EItchMessageState::None)
				{
					fflush(stdout);
				}
				firstMessage->state = Itch::EItchMessageState::Read;
				//_rewinderBufferPtr += readLen;
			}
		}
		else
		{
			perror("recvfrom failed");
			// Or inspect errno
			printf("Error: %s\n", strerror(errno));
		}
		pthread_mutex_lock(&_rewinderMutex);
		if (_rewinderRunning) _rewinderRunning = 0;
		pthread_mutex_unlock(&_rewinderMutex);
		
	}
}

void InstrumentCreater::processorThreadHandler()
{
    
    Itch::TItchMessage *msg = _itchMessageList + _nextProcessSequence;
    Itch::TItchMessage *lastMsg = 0;
    Itch::TItchMessage *afterLastMsg = 0;
    Itch::TItchMessageBase *msgBase = 0;
    int orderBookId = 0;
    char lastInst[32];
    while (_continue) // check while loop
    {
         if (_nextProcessSequence >= lastMessageFromFile)
		 {
			sendInstrumentsFlag = true;
			sender->start();
			break;
		 }
		 else
		 {
			if (msg->state == Itch::EItchMessageState::None) 
            {
                if (_nextProcessSequence < _lastHbSequence) 
                {
                    if (_rewinderRunning)
                        continue;
                    //startRewinder(_nextProcessSequence, _lastHbSequence);
                }
                continue;
            }
			
            lastMsg = msg->lastMessage;
            if (!lastMsg)
            {
                continue;
            }
            while (lastMsg->state == Itch::EItchMessageState::None && _continue) // check while
                ;
            if (!_continue)
                break;
            afterLastMsg = lastMsg + 1;

            while (msg != afterLastMsg)
            {
                msgBase = (Itch::TItchMessageBase *)msg->addr;
                int totalMessagesInBlock = afterLastMsg - msg;
                
            
                switch (msgBase->type)
                {
                    case ITCH_MESSAGE_TYPE_ORDERBOOKDIRECTORY:
                    {
                        /*
						if (!instruments.empty())
                        {
                            const Instrument &lastInstrument = instruments.back();
							
                            //std::cout << "Written instrument: name " << lastInstrument.title << std::endl;
                            //std::cout << "Written instrument: name " << lastInstrument.internalName << std::endl;
                            //std::cout << "Written instrument: name " << lastInstrument.orderBookId << std::endl;
                            //std::cout << "Written instrument Prize From:  " << lastInstrument.tickSizeMap[2].start << std::endl;
                            //std::cout << "Written instrument: Prize To: " << lastInstrument.tickSizeMap[2].end << std::endl;
                            //std::cout << "Written instrument: Tick Size: " << lastInstrument.tickSizeMap[2].tickSize << std::endl;
                            //std::cout <<std::endl;
                        }
						*/
						
                        Itch::TItchMessageOrderBookDirectory *m = (Itch::TItchMessageOrderBookDirectory *)msgBase;
                        instrumentCount = instrumentCount + 1;
                        orderBookId = htobe32(m->orderBookId);
                        int arderBookId = be32toh(m->orderBookId);
                        Instrument instrument;
                        instrument.id = instrumentCount;
                        instrument.orderBookId = be32toh(m->orderBookId);
                        char output[32];
                        memset(output, 0, sizeof(output));
                        size_t i = 0;

                        while (m->symbol[i] != '.' && m->symbol[i] != '\0' && i < sizeof(m->symbol) - 1) 
                        {
                            output[i] = m->symbol[i];
                            i++;
                        }
                        if (m->symbol[i + 1] == 'E') // check if
                        {
                            std::strncpy(instrument.internalName, output, sizeof(instrument.internalName));
                            output[i] = '.';
                            output[i + 1] = 'E';
                            std::strncpy(instrument.title, output, sizeof(instrument.internalName));
                            instrument.instrumentType = EInstrumentType::Equity;
							//None diye doldur            
                            instrument.orderBookType = EOrderBookType::Full;                
                            instrument.marketGatewayType = EGatewayType::BistechFpgaMarket; 
                            instrument.marketGatewayNo = 1;
                            instrument.orderGatewayType = EGatewayType::BistechFpgaOrder; 
                            instrument.originalOrderGatewayNo = 1;
                            for (int i = 0; i < 10; ++i) 
                            {
                                instrument.orderGroupGatewayIdxList[i] = i;
                            }
                            instrument.decimalsInPrice = be16toh(m->decimalsInPrice);
                            instrument.quantityMultiplier = 1;
                            instrument.allowShortSell = 0;
                            _tickSizeIndex = 0;
                            instruments.push_back(instrument);
                        }
                        else if (m->symbol[i + 1] == 'F') // check else if
                        {
                            std::strncpy(instrument.internalName, output, sizeof(instrument.internalName)); 
                            output[i] = '.';
                            output[i + 1] = 'F';
                            std::strncpy(instrument.title, output, sizeof(instrument.internalName));
                            instrument.instrumentType = EInstrumentType::Equity;            
                            instrument.orderBookType = EOrderBookType::None;                
                            instrument.marketGatewayType = EGatewayType::BistechFpgaMarket; 
                            instrument.marketGatewayNo = 1;
                            instrument.orderGatewayType = EGatewayType::BistechFpgaOrder; 
                            instrument.originalOrderGatewayNo = 1;
                            for (int i = 0; i < 10; ++i) 
                            {
                                instrument.orderGroupGatewayIdxList[i] = i;
                            }
                            instrument.decimalsInPrice = be16toh(m->decimalsInPrice);
                            instrument.quantityMultiplier = 1;
                            instrument.allowShortSell = 0;
                            _tickSizeIndex = 0;
                            instruments.push_back(instrument);
                        }
                        break;
                    }
                    case ITCH_MESSAGE_TYPE_TICKTABLEENTRY:
                    {
                        Itch::TItchMessagePriceTickTableEntry *m = (Itch::TItchMessagePriceTickTableEntry *)msgBase;
                        Instrument &lastInstrument = instruments.back();
                        if (be16toh(m->priceFrom) != 0 && be16toh(m->priceTo) != 0) // check if
                        {
                            lastInstrument.tickSizeMap[_tickSizeIndex] = {be16toh(m->priceFrom), be16toh(m->priceTo), be16toh(m->tickSize)};
                            _tickSizeIndex = _tickSizeIndex + 1;
                        }
                        else // check else
                        {
                        }
                        break;
                    }
                }
                
                msg->state = Itch::EItchMessageState::Processed;
                msg++;
                _nextProcessSequence++;
            }
		 }        
    }     
}



void InstrumentCreater::multicastThreadHandler()
{
    int readLen = 0;
	ulong sequence = 0;
	ulong lastSequence = 0;
	ushort messageCount = 0;
	char *msgAddr = 0;
	ushort messageLength = 0;
	Itch::TItchMessage *firstMessage = 0;
	Itch::TItchMessage *msg = 0;
	Itch::TItchMessageBase *msgBase = 0;
	int isFirstMessage = 1;
    while (_continue)
    {
        readLen = read(_multicastSocket, _bufferPtr, 65536);
		if (!_continue) break;
        if (readLen > 0)
        {
            Itch::TItchBlock *block = (Itch::TItchBlock*) _bufferPtr;
			sequence = be64toh(block->sequenceNumber);
			messageCount = be16toh(block->messageCount);
            //multiMessageCount = sequence;
            if (isFirstMessage)
			{
				memcpy(_rewinderMessage.session, block->session, sizeof(_rewinderMessage.session));
				isFirstMessage = 0;
			}
            _lastHbSequence = sequence;
            msgAddr = _bufferPtr + sizeof(Itch::TItchBlock);
            //printf("Multicast Seq Num %d\n",sequence);
			//printf("Multicast Message count %d\n",messageCount);
            msg = _itchMessageList + sequence;
            msgBase = (Itch::TItchMessageBase *)msgAddr;
            messageLength = be16toh(msgBase->length);
            msg->addr = msgAddr;
            msg->state = Itch::EItchMessageState::Read;
            _bufferPtr += readLen;
            break;
        }
    }
    
}



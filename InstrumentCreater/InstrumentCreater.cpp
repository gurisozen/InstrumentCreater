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
#include <unordered_map>

#ifdef __CYGWIN__
#define pthread_setname_np(...) ;
#define pthread_attr_setaffinity_np(...) 0
#define cpu_set_t int
#define CPU_ZERO(...) ;
#define CPU_SET(...) ;
#endif

#define OUTPUT_DEBUG_FILE

InstrumentCreater::InstrumentCreater(int part, int* gatewayInstrumentCount, std::unordered_map<unsigned int,Instrument*>* marketMap, std::vector<unsigned int>* orderBookIds, std::condition_variable* cv, std::mutex* mtx)
    : partition(part), gatewayInstrumentCount(gatewayInstrumentCount), marketMapByOrderBookId(marketMap),orderBookIds(orderBookIds), cv(cv), mtx(mtx) 
{
	_continue = 1;
	_lastHbSequence = 1;
	_nextProcessSequence = 1;
	_instFileBuffer = (char*) realloc(_instFileBuffer, bufferSize);
	memset(_buffer, 0, sizeof(_buffer));
	memset(_itchMessageList, 0, sizeof(_itchMessageList));
	_bufferPtr = _buffer;
	outputFile = 0;
	_marketProcessorThread = 0;
    std::string basePath = "/opt/itchTest/Configs/InstrumentCreater/InstrumentCreater";
    std::string fileExtension = ".config";
    configFile = basePath + std::to_string(part) + fileExtension;
    configFile = basePath + std::to_string(part) + fileExtension;
	partition = part;
}
static void *startMulticastThread(void *obj)
{
	((InstrumentCreater*) obj)->multicastThreadHandler();
	return 0;
}

static void *startMarketProcessorThread(void *obj)
{
	((InstrumentCreater*) obj)->marketInstrumentsProcessorHandler();
	return 0;
}

static void *startRewinderThread(void *obj)
{
	((InstrumentCreater*) obj)->rewinderThreadHandler();
	return 0;
}


int InstrumentCreater::init()
{
	if (pthread_create(&_marketProcessorThread, nullptr, startMarketProcessorThread, this) < 0)
	{
		std::cerr << "Market Processor Thread not created " << _multicastSocket  << std::endl;
	}
	else
	{
		std::cout << "Market Processor Thread created" << partition<<std::endl;
	}
	
	return 0;
}
//Function to read ITCH market data from files for each partition.
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
	else if (!f) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;  // Return early if file is not found
    }
	lastMessageFromFile = sequence;
}

bool InstrumentCreater::isFinished() {
    return finished;
}
void InstrumentCreater::start()
{
	char filename[1024] = {0};
	char timestamp[9] = {0};  
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(timestamp, sizeof(timestamp), "%Y%m%d", t);
	sprintf(filename, "/opt/itchTest/itchFiles/itch.%d.%s.dat",partition,timestamp);
	printf("Opening file for reading market instrument info: ");
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
	if (pthread_join(_marketProcessorThread, nullptr))
    {
		std::cerr << "Failed to join market processor thread. Error: " << strerror(errno) << std::endl;
    }
	else
	{
		std::cout << "Market Processor Thread joined" <<partition<< std::endl;
	}
}
void InstrumentCreater::disconnect()
{
	_continue = 0;
}


void InstrumentCreater::marketInstrumentsProcessorHandler()
{
    std::unique_lock<std::mutex> lock(*mtx);
    Itch::TItchMessage *msg = _itchMessageList + _nextProcessSequence;
    Itch::TItchMessage *lastMsg = 0;
    Itch::TItchMessage *afterLastMsg = 0;
    Itch::TItchMessageBase *msgBase = 0;
    int orderBookId = 0;
    char lastInst[32];
    int lastOrderBookId;
    while (_continue) // check while loop
    {

        if (_nextProcessSequence >= lastMessageFromFile)
        {
            finished = true;
            cv->notify_one();
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
                }
                continue;
            }
            lastMsg = msg->lastMessage;
            if (!lastMsg)
            {
                continue;
            }
            while (lastMsg->state == Itch::EItchMessageState::None && _continue) 
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
						//Instrument info 
						Itch::TItchMessageOrderBookDirectory *m = (Itch::TItchMessageOrderBookDirectory *)msgBase;
						orderBookId = htobe32(m->orderBookId);
						
						auto it = marketMapByOrderBookId->find(orderBookId);
						if (it != marketMapByOrderBookId->end()) 
						{
							//Some orderbookids duplicate ignore if 
							Instrument* existingInstrument = it->second;
							_tickSizeIndex = 0;
							break;
						}
						Instrument* instrument = new Instrument();
						instrument->isActive = 0;
						instrument->orderBookId = orderBookId;
						char output[32];
						memset(output, 0, sizeof(output));
						size_t i = 0;
                        while (m->symbol[i] != '.' && m->symbol[1] != '\0' && i < sizeof(m->symbol) - 1)
                        {
                            output[i] = m->symbol[i];
                            i++;
                        }
						// Parse symbol and assign appropriate title
						//Instruments with titles .E, .F and  F_ 
						if ((m->symbol[i] == '.' && (m->symbol[i + 1] == 'E' || m->symbol[i + 1] == 'R')) && !(m->symbol[0] == 'O' && m->symbol[1] == '_'))
						{
							(*gatewayInstrumentCount)++;
							 instrument->id = *gatewayInstrumentCount;
                            std::strncpy(instrument->internalName, output, sizeof(instrument->internalName));
                            output[i] = '.';
                            output[i + 1] = 'E';
                            std::strncpy(instrument->title, output, sizeof(instrument->title));
							instrument->decimalsInPrice = be16toh(m->decimalsInPrice);
							instrument->instrumentType = EInstrumentType::Equity;
                            instrument->orderBookType = EOrderBookType::None;
                            instrument->marketGatewayType = EGatewayType::BistechItch;
                            instrument->marketGatewayNo = partition;
                            instrument->orderGatewayType = EGatewayType::BistechFpgaOrder;
                            instrument->originalOrderGatewayNo = 0;
							instrument->contractSize = 1;
                            for (int i = 0; i < 10; ++i)
                            {
                                instrument->orderGroupGatewayIdxList[i] = i;
                            }
							_tickSizeIndex = 0;  
                            instrument->quantityMultiplier = 1;
                            instrument->allowShortSell = 0;
							instrument->nearLegOrderBookId =0;
							instrument->farLegOrderBookId = 0;
							instrument->leggedInstrument = 0;
							instrument->fpgaSessionNo = 1;
							(*marketMapByOrderBookId)[instrument->orderBookId] = instrument;
							orderBookIds->push_back(instrument->orderBookId);
						}
						else if ((m->symbol[i] == '.' && m->symbol[i + 1] == 'F') || (m->symbol[0] == 'F' && m->symbol[1] == '_'))
						{
							(*gatewayInstrumentCount)++;
							instrument->id = *gatewayInstrumentCount;
                            std::strncpy(instrument->internalName, output, sizeof(instrument->internalName));
                            if (!(m->symbol[0] == 'F' && m->symbol[1] == '_'))  // Only add .F if it does NOT start with F_
							{
								output[i] = '.';
								output[i + 1] = 'F';
							}
                            std::strncpy(instrument->title, output, sizeof(instrument->title));
							instrument->decimalsInPrice = be16toh(m->decimalsInPrice);
							instrument->instrumentType = EInstrumentType::Future;
                            instrument->orderBookType = EOrderBookType::None;
                            instrument->marketGatewayType = EGatewayType::BistechItch;
                            instrument->marketGatewayNo = partition;
                            instrument->orderGatewayType = EGatewayType::BistechFpgaOrder;
							instrument->originalOrderGatewayNo = 0;
							instrument->contractSize = 1;
                            for (int i = 0; i < 10; ++i)
                            {
                                instrument->orderGroupGatewayIdxList[i] = i;
                            }
                            instrument->quantityMultiplier = 1;
                            instrument->allowShortSell = 0;
							instrument->nearLegOrderBookId = 0;
							instrument->farLegOrderBookId = 0;
							instrument->leggedInstrument = 0;
							instrument->fpgaSessionNo = 2;
                            _tickSizeIndex = 0;
							(*marketMapByOrderBookId)[instrument->orderBookId] = instrument;
							orderBookIds->push_back(instrument->orderBookId);
						}
						else
						{
						}
						break;
					}
                    case ITCH_MESSAGE_TYPE_TICKTABLEENTRY:
					{
						Itch::TItchMessagePriceTickTableEntry *m = (Itch::TItchMessagePriceTickTableEntry *)msgBase;
						unsigned int orderBookId = htobe32(m->orderBookId);
						Instrument* lastInstrument =nullptr;
						auto it = marketMapByOrderBookId->find(orderBookId);
						if (it != marketMapByOrderBookId->end()) 
						{
							lastInstrument = it->second;
							if (!lastInstrument) 
							{
								std::cerr << "Instrument for orderBookId " << orderBookId << " not found!" << std::endl;
								break;
							}
							if (_tickSizeIndex < 10) 
							{
								if (htobe32(m->priceFrom) != 0 && htobe32(m->priceTo) != 0) 
								{
									lastInstrument->tickSizeMap[_tickSizeIndex] = {
										(int)htobe32(m->priceFrom), 
										(int)htobe32(m->priceTo), 
										(int)htobe64(m->tickSize)
									};
									_tickSizeIndex++;
								}
							} 
							else 
							{
								std::cerr << "Tick size index out of bounds for orderBookId " << orderBookId << std::endl;
							}
						} 
						else 
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
    std::cout << "Market processor handler finished for partition: " << partition << std::endl;
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


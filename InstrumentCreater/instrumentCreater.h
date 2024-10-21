#ifndef INSTRUMENT_CREATER_H_
#define INSTRUMENT_CREATER_H_

#include </opt/itchTest/InstrumentCreater/connection.h>
#include </opt/itchTest/InstrumentCreater/itchDefinitions.h>
#include <vector>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include </opt/itchTest/Configs/configuration.h>
#include </opt/itchTest/InstrumentCreater/instrumentDefinitions.h>
#include </opt/itchTest/InstrumentCreater/instrumentSender.h>
#include "khashInt.h"
#include <unordered_map>

#define MAX_ITCH_BUFFER_SIZE		(1024 * 1024 * 768)
#define MAX_ITCH_MESSAGE_COUNT		30000000

class InstrumentCreater
	{

        public:
            InstrumentCreater(int part, int* gatewayInstrumentCount,  std::unordered_map<unsigned int,Instrument*>* marketMap, std::vector<unsigned int>* orderBookIds, std::condition_variable* cv, std::mutex* mtx);
            int init();
            void start();
            void stop();
            
            void startConnect();
            void disconnect();
            bool isFinished();
            void multicastThreadHandler();
			void marketInstrumentsProcessorHandler();
			void rewinderThreadHandler();
            void setMarketInstruments(const char *filename);
            std::unordered_map<unsigned int, Instrument*>* getMarketMap(); 
		private:
			void waitRewinder(ulong *startSequence, ulong *endSequence);
			void startRewinder(ulong startSequence, ulong endSequence);
            ConfigurationManager configManager;
            std::string configFile;
            //Connection information
            const char* rewinderLocalIp;
            const char* rewinderRemoteIp;
			const char* multicastLocalIp;
            const char* multicastRemoteIp;
            int rewinderPort;
            int multicastPort;
            //sockets
            int _multicastSocket;
            int _rewinderSocket;
            //fileNames
            char* directory;
            char* fileName;
            FILE* outputFile;
            //partition
            int partition;
            //sequence numbers
            volatile ulong _lastHbSequence;
			volatile ulong _nextProcessSequence;
            volatile ulong _rewinderStartSequence;
			volatile ulong _rewinderEndSequence;
            //threads
            pthread_t _multicastThread;
			pthread_t _marketProcessorThread;
			pthread_t _rewinderThread;
			pthread_t _writerThread;
			//flags
            volatile int _rewinderRunning;
			volatile int _continue;
            volatile int _rewinderStart;
            //rewinder related mutexes
			pthread_mutex_t _rewinderMutex;
			pthread_cond_t _rewinderCond;
			//buffer pointers
            char *_bufferPtr;
			char *_rewinderBufferPtr;
            char *_instFileBuffer;
            int bufferSize = 8192;
            int _fileReadOffset = 0;
			//buffers
            char _buffer[MAX_ITCH_BUFFER_SIZE];
			char _rewinderBuffer[MAX_ITCH_BUFFER_SIZE];
            //message list
            Itch::TItchMessage _itchMessageList[MAX_ITCH_MESSAGE_COUNT];
            //rewinder message
            Itch::TItchRewinderMessage _rewinderMessage;
            volatile bool sendInstrumentsFlag = false;
            
            std::unordered_map<unsigned int, Instrument*> *marketMapByOrderBookId;
            std::vector<unsigned int>* orderBookIds;
            
            std::vector<Instrument*>* gatewayInstruments ;
            
            int tickSize =0;
            int* gatewayInstrumentCount;
            int _inactiveCounter; 
            const int _inactiveThreshold = 1000; 
            int _tickSizeIndex = 0;
            int processFlag = 0;
                  
            int lastMessageFromFile = 0;

            bool finished = false;

            std::mutex* mtx;  
            std::condition_variable* cv; 
	};
	



#endif /* INSTRUMENT_CREATER_H_ */
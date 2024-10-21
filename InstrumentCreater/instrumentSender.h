/*
 * socket.h
 *
 *  Created on: Oct 5, 2023
 *      Author: RestinB
 */

#ifndef INSTRUMENT_SENDER_H_
#define INSTRUMENT_SENDER_H_
#include <netinet/in.h>
#include <thread> // Include for std::thread
#include <future> 
#include </opt/itchTest/Configs/configuration.h>
#include </opt/itchTest/InstrumentCreater/instrumentDefinitions.h>
#include "khashInt.h"
#include <unordered_map>

namespace Libs
{   
    class InstrumentSender
    {
        public:
            InstrumentSender(int part, int* gatewayInstrumentCount, int* nextiId, std::unordered_map<unsigned int,Instrument*>* marketMap,Lib::Dictionary::CKhashInt<Instrument*>* activeMap,std::vector<unsigned int>* orderBookIds,std::vector<unsigned int>* activeIds, volatile bool* flag);
            ~InstrumentSender();
            void join();
            void stop();
            int start();
            void startThreads();
            void init();
            void connect();
            void resetFlags();
            void connectionCheckerThreadHandler();
            void fileSenderThreadHandler();
            void writeInstruments(int count);
            void setMarket();
            void messageReaderThreadHandler();
            void setWriteMessageCallback(std::function<void()> callback);
            void compareInstruments();
            void compareActiveInstruments();
        private:
            int server_fd;
            int client_fd;
            struct sockaddr_in address;
            int addrlen;
            std::string configFile;
            std::streampos lastReadPosition;
            int64_t _lastByte = 0;
            int totalFileSize = 0;
            int _continue = 0;
            ConfigurationManager configManager;
            char* fileName;
            const char* directory;
            char* fullPath;
            int partition;
            int port;
            int _indexAck;
            int _connect;
            
            volatile bool* sendInstrumentsFlag;
            bool _readFlag;
            std::vector<unsigned int>* orderBookIds;//all
            std::vector<unsigned int>* activeOrderBookIds;//actives
            std::vector<unsigned int> newActiveOrderBookIds;//newactives
            std::unordered_map<unsigned int, Instrument*>* marketMapByOrderBookId;
            Lib::Dictionary::CKhashInt<Instrument*>* activeMapByOrderBookId;//all
            Lib::Dictionary::CKhashInt<Instrument*> newActiveMapByOrderBookId;//newactives
            int* nextId;
            pthread_t _fileSenderThread;
            pthread_t _connectionCheckerThread;
            pthread_t _readerThread;
            int* gatewayInstrumentCount;
            std::function<void()> writeMessageCallback;
    };
}

#endif /* INSTRUMENT_SENDER_H_ */

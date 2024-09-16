/*
 * socket.h
 *
 *  Created on: Oct 5, 2023
 *      Author: RestinB
 */

#ifndef ENGINE_LIB_SERVER_H_
#define ENGINE_LIB_SERVER_H_
#include <netinet/in.h>
#include <thread> // Include for std::thread
#include <future> 
#include </opt/itchTest/Configs/configuration.h>
#include </opt/itchTest/InstrumentCreater/instrumentDefinitions.h>
namespace Lib
{   
    class InstrumentSender
    {
        public:
            InstrumentSender(int part, std::vector<Instrument>* instruments, volatile bool* flag);
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
            std::vector<Instrument>* instruments;
            volatile bool* sendInstrumentsFlag;
            
            pthread_t _fileSenderThread;
            pthread_t _connectionCheckerThread;
            
    };
}

#endif /* ENGINE_LIB_SERVER_H_ */

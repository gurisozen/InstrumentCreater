#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  // Include for TCP keepalive options
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>   // Include for select() system call
#include "instrumentSender.h"
#include "/opt/itchTest/Configs/configuration.h"
#include <execinfo.h>
#include <unordered_map>
#define BUFFER_SIZE (1024 * 1024 * 2)

namespace Libs
{
    InstrumentSender::InstrumentSender(int part,int* gatewayInstrumentCount,int* nextId, std::unordered_map<unsigned int,Instrument*>* marketMap,Lib::Dictionary::CKhashInt<Instrument*>* activeMap, std::vector<unsigned int>* activeIds,std::vector<unsigned int>* orderBookIds, volatile bool* flag)
    : partition(part),gatewayInstrumentCount(gatewayInstrumentCount),nextId(nextId),marketMapByOrderBookId(marketMap),activeMapByOrderBookId(activeMap),activeOrderBookIds(activeIds), orderBookIds(orderBookIds), sendInstrumentsFlag(flag)
    {
        // Flags
        _indexAck = 0;
        _continue = 0;
        _connect = 1;
        _readFlag = false;
        std::string basePath = "/opt/itchTest/Configs/InstrumentSender/InstrumentSender";
        std::string fileExtension = ".config";
        printf("The Partition is %d\n",part);
        printf("The Port is %d\n",port);
        configFile = basePath + std::to_string(part) + fileExtension;
        directory = configManager.readConfig(configFile, "directory");
        port = std::stoi(configManager.readConfig(configFile, "port"));
        partition = part;     
    }
    
    InstrumentSender::~InstrumentSender()
    {
        close(server_fd);
    }
    static void *startFileSenderThread(void *obj)
    {
        ((InstrumentSender*) obj)->fileSenderThreadHandler();
        return 0;
    }
    static void *startMessageReaderThread(void *obj)
    {
        ((InstrumentSender*) obj)->messageReaderThreadHandler();
        return 0;
    }
    static void *startConnectionCheckerThread(void *obj)
    {
        ((InstrumentSender*) obj)->connectionCheckerThreadHandler();
        return 0;
    }
    void InstrumentSender::init()
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0)
        {
            std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Socket creation succeeded" << std::endl;

        int optval = 1;
        socklen_t optlen = sizeof(optval);
        if (setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
        {
            std::cerr << "Error enabling TCP keepalive: " << strerror(errno) << std::endl;
        }
        int keepalive_time = 60; 
        int keepalive_interval = 10; 
        int keepalive_probes = 5; 
        //Create sockets
        setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time));
        setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval));
        setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(keepalive_probes));
        // Set up the address
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        addrlen = sizeof(address);   
    }
    void InstrumentSender::startThreads()
    {

        if (pthread_create(&_connectionCheckerThread, nullptr, startConnectionCheckerThread, this)< 0)
        {
            std::cerr << "Connection checker thread NOT created " << strerror(errno) << std::endl;
        }
        else
        {
            std::cout << "Connection Checker Thread created" << std::endl;
        } 
        if (pthread_create(&_fileSenderThread, nullptr, startFileSenderThread, this)< 0)
        {
            std::cerr << "File Sender Thread NOT created " <<  server_fd  << std::endl;
        }
        else
        {
            std::cout << "File Sender Thread created" << std::endl;
        } 
        if (pthread_create(&_readerThread, nullptr, startMessageReaderThread, this)< 0)
        {
            std::cerr << "Message Reader Thread NOT created " <<  server_fd  << std::endl;
        }
        else
        {
            std::cout << "Message Reader Thread created" << std::endl;
        } 
    }
    int InstrumentSender::start()
    {
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            std::cerr << "Bind failed: " << strerror(errno) << std::endl;
            return -1;
        }
        if (listen(server_fd, 3) < 0)
        {
            std::cerr << "Listen failed: " << strerror(errno) << std::endl;
            return -1;
        }
        std::cout << "Server started. Waiting for connections..." << std::endl;
        connect();
        return 0;
    }
    
    //This function writes instruments to file
    void InstrumentSender::writeInstruments(int instrumentCount)
    {
        std::ofstream outFile("instrument.dat", std::ios::binary);
        if (!outFile) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;  
        }  
        int tag = 1; 
        writeInt(outFile, tag);
        int version = 1; 
        writeInt(outFile, version);
        writeInt(outFile, instrumentCount);
        int header = 0; 
        writeInt(outFile, header);
        int entrySize = sizeof(WriteInstrument);
        for (const auto& orderBookId : newActiveOrderBookIds)  
        {
            Instrument* activeInstrument = nullptr;
            if (newActiveMapByOrderBookId.get(orderBookId, &activeInstrument))  
            {
                //Create WriteInstruments of each from activeInstrument info.
                if (activeInstrument)  
                {
                    WriteInstrument wInstrument;
                    wInstrument.id = activeInstrument->id;
                    wInstrument.orderBookId = activeInstrument->orderBookId;
                    strncpy(wInstrument.title, activeInstrument->title, sizeof(wInstrument.title));
                    strncpy(wInstrument.internalName, activeInstrument->internalName, sizeof(wInstrument.internalName));
                    wInstrument.instrumentType = activeInstrument->instrumentType;
                    wInstrument.orderBookType = activeInstrument->orderBookType;
                    wInstrument.marketGatewayType = activeInstrument->marketGatewayType;
                    wInstrument.marketGatewayNo = activeInstrument->marketGatewayNo;
                    wInstrument.orderGatewayType = activeInstrument->orderGatewayType;
                    wInstrument.originalOrderGatewayNo = activeInstrument->originalOrderGatewayNo;
                    for (int i = 0; i < MAX_GATEWAY_COUNT; ++i) {
                        wInstrument.orderGroupGatewayIdxList[i] = activeInstrument->orderGroupGatewayIdxList[i];
                    }
                    wInstrument.decimalsInPrice = activeInstrument->decimalsInPrice;
                    wInstrument.contractSize = activeInstrument->contractSize;
                    wInstrument.quantityMultiplier = activeInstrument->quantityMultiplier;
                    wInstrument.allowShortSell = activeInstrument->allowShortSell;
                    for (int i = 0; i < MAX_TICKSIZE_COUNT; ++i) {
                        wInstrument.tickSizeMap[i] = activeInstrument->tickSizeMap[i];
                    }
                    wInstrument.leggedInstrument = activeInstrument->leggedInstrument;
                    wInstrument.farLegOrderBookId = activeInstrument->farLegOrderBookId;
                    wInstrument.nearLegOrderBookId = activeInstrument->nearLegOrderBookId;
                    wInstrument.fpgaSessionNo = activeInstrument->fpgaSessionNo;
                    writeInt(outFile, entrySize);
                    writeInstrument(outFile, wInstrument);  // Write instrument details to the file
                    std::cout << "Written instrument " << activeInstrument->id 
                            << " Name " << activeInstrument->title << std::endl;
                    std::cout << "Written instrument " << activeInstrument->isActive 
                            << " OrderbookId " << activeInstrument->orderBookId << std::endl;
                }
            }
        }
        //Send write confirm to server
        WriteMessage writeMessage('S', 1);
        size_t totalBytesSent = 0;
        size_t writeMessageSize = sizeof(writeMessage);
        totalBytesSent = 0;
        const char* writeMessageData = reinterpret_cast<const char*>(&writeMessage);
        while (totalBytesSent < writeMessageSize)
        {
            ssize_t bytesSent = send(client_fd, writeMessageData + totalBytesSent, writeMessageSize - totalBytesSent, 0);
            
            if (bytesSent == -1) 
            {
                std::cerr << "Failed to send instrument information" << std::endl;
                return;
            }
            totalBytesSent += bytesSent;
        }        
    }
    void InstrumentSender::setWriteMessageCallback(std::function<void()> callback) 
    {
        writeMessageCallback = callback;
        
    }
    void InstrumentSender::messageReaderThreadHandler()
    {
        char buffer[BUFFER_SIZE];  // Buffer to receive the instrument
        int expectedInstrumentCount = 0; 
        int instrumentCount = 0;
        while (_continue) 
        {
                int messageLength;
                ssize_t bytesReceived = recv(client_fd, &messageLength, sizeof(messageLength), 0);
                if (bytesReceived <= 0) 
                {
                    if (bytesReceived == 0) 
                    {
                        std::cerr << "Connection closed by client." << std::endl;
                    }
                    else 
                    {
                        std::cerr << "Error receiving message length: " << strerror(errno) << std::endl;
                    }
                    close(client_fd);
                    break; // Exit the loop since the connection is closed or failed
                }
                char messageType;
                bytesReceived = recv(client_fd, &messageType, sizeof(messageType), 0);
                if (bytesReceived <= 0) 
                {
                    if (bytesReceived == 0) 
                    {
                        std::cerr << "Connection closed by client." << std::endl;
                    }
                    else 
                    {
                        std::cerr << "Error receiving message type: " << strerror(errno) << std::endl;
                    }
                    close(client_fd);
                    break; // Exit the loop since the connection is closed or failed
                }
                //Instrument Message
                if (messageType == 'R')  // Receiving an instrument
                {
                    InstMessage receivedMessage;
                    receivedMessage.length = messageLength;
                    receivedMessage.messageType = messageType;
                    ssize_t instrumentSize = sizeof(receivedMessage.instrument);
                    ssize_t totalBytesReceived = 0;
                    instrumentCount++;
                    while (totalBytesReceived < instrumentSize) 
                    {
                        bytesReceived = recv(client_fd, reinterpret_cast<char*>(&receivedMessage.instrument) + totalBytesReceived, instrumentSize - totalBytesReceived, 0);
                        if (bytesReceived <= 0) 
                        {
                            if (bytesReceived == 0) 
                            {
                                std::cerr << "Connection closed by client." << std::endl;
                            }
                            else 
                            {
                                std::cerr << "Error receiving instrument data: " << strerror(errno) << std::endl;
                            }
                            close(client_fd);
                            return;
                        }
                        totalBytesReceived += bytesReceived;
                    }
                    Instrument* newInstrument = new Instrument(receivedMessage.instrument);
                    Instrument* existingIns = nullptr;
                    if(!activeMapByOrderBookId->get(newInstrument->orderBookId, &existingIns))
                    { 
                        newInstrument->id = *nextId;
                        (*nextId)++;
                    }
                    newActiveMapByOrderBookId.add(newInstrument->orderBookId, newInstrument); 
                    newActiveOrderBookIds.push_back(newInstrument->orderBookId);
                    //Send back instruments to server
                    InstMessage instrumentMessage('R', *newInstrument);
                    size_t instrumentMessageSize = sizeof(instrumentMessage);
                    size_t totalBytesSent = 0;
                    const char* instrumentMessageData = reinterpret_cast<const char*>(&instrumentMessage);
                    while (totalBytesSent < instrumentMessageSize)
                    {
                        ssize_t bytesSent = send(client_fd, instrumentMessageData + totalBytesSent, instrumentMessageSize - totalBytesSent, 0);
                        if (bytesSent == -1) 
                        {
                            std::cerr << "Failed to send instrument with OrderBookId: " << newInstrument->orderBookId << std::endl;
                            return; 
                        }
                        totalBytesSent += bytesSent;
                    }
                } 
                else if (messageType == 'C')  
                {
                    //Count message
                    CountMessage receivedMessage;
                    receivedMessage.length = messageLength;
                    receivedMessage.messageType = messageType;
                    ssize_t countSize = sizeof(receivedMessage.count);
                    ssize_t totalBytesReceived = 0;
                    while (totalBytesReceived < countSize) 
                    {
                        bytesReceived = recv(client_fd, reinterpret_cast<char*>(&receivedMessage.count) + totalBytesReceived, countSize - totalBytesReceived, 0);
                        if (bytesReceived <= 0) 
                        {
                            if (bytesReceived == 0) 
                            {
                                std::cerr << "Connection closed by client." << std::endl;
                            }
                            else 
                            {
                                std::cerr << "Error receiving instrument count: " << strerror(errno) << std::endl;
                            }
                            close(client_fd);
                            return;
                        }
                        totalBytesReceived += bytesReceived;
                    }
                    expectedInstrumentCount = receivedMessage.count;
                    writeInstruments(expectedInstrumentCount);
                } 
                else if (messageType == 'S') 
                {
                    //Write confirm message
                    WriteMessage receivedMessage;
                    receivedMessage.length = messageLength;
                    receivedMessage.messageType = messageType;
                    ssize_t statusSize = sizeof(receivedMessage.status);
                    ssize_t totalBytesReceived = 0;
                    while (totalBytesReceived < statusSize) 
                    {
                        bytesReceived = recv(client_fd, reinterpret_cast<char*>(&receivedMessage.status) + totalBytesReceived, statusSize - totalBytesReceived, 0);
                        if (bytesReceived <= 0) 
                        {
                            if (bytesReceived == 0) 
                            {
                                std::cerr << "Connection closed by client." << std::endl;
                            }
                            else 
                            {
                                std::cerr << "Error receiving instrument count: " << strerror(errno) << std::endl;
                            }
                            close(client_fd);
                            return;
                        }
                        totalBytesReceived += bytesReceived;
                    }
                    if (receivedMessage.status == 1)
                    {
                        compareActiveInstruments();
                    }
                    int lastDeleted;
                    std::ifstream inFile("lastDeleted.txt");
                    if (inFile.is_open()) 
                    {
                        inFile >> lastDeleted;  
                        inFile.close();    
                    } 
                    else 
                    {
                        std::cerr << "Error: Could not open file for reading." << std::endl;
                    }
                    if (lastDeleted + 1 > *nextId)
                    {
                        *nextId = lastDeleted + 1;
                    }
                } 
                else 
                {
                    std::cerr << "Unknown message tyspe received: " << messageType << std::endl;
                }
        }
    }
    void InstrumentSender::connect()
    {
        while (true)
        {
            if (_connect)
            {
                int lastDeleted;
                std::ifstream inFile("lastDeleted.txt");
                if (inFile.is_open()) {
                    inFile >> lastDeleted;  
                    inFile.close();  
                } 
                else 
                {
                    std::cerr << "Error: Could not open file for reading." << std::endl;
                }
                if (lastDeleted + 1 > *nextId)
                {
                    *nextId = lastDeleted+1;
                }
                std::cout << "Waiting for a client connection..." << std::endl;
                client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                if (client_fd < 0)
                {
                    std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                }
                else
                {
                    std::cout << "Connected to a client!" << std::endl;
                    _continue = 1;
                    _connect = 0; 
                    *sendInstrumentsFlag = true;
                    startThreads();
                }
            }
            sleep(1); 
        }
    }
    void InstrumentSender::setMarket()
    {
        if (marketMapByOrderBookId)  
        {
            for (auto& pair : *marketMapByOrderBookId)  
            {
                if (pair.second)  
                {
                    pair.second->orderBookType = EOrderBookType::None;
                    pair.second->marketGatewayType = EGatewayType::BistechFpgaMarket;
                    pair.second->orderGatewayType = EGatewayType::BistechFpgaOrder;
                    pair.second->originalOrderGatewayNo = 0;
                    pair.second->contractSize = 1;
                    for (int i = 0; i < 10; ++i)
                    {
                        pair.second->orderGroupGatewayIdxList[i] = i;
                    }
                    pair.second->quantityMultiplier = 1;
                    pair.second->allowShortSell = 0;
                    pair.second->nearLegOrderBookId = 0;
                    pair.second->farLegOrderBookId = 0;
                    pair.second->isActive = 0;  
                }
            }
        }
    } 
    
    
    //Function to match active instruments with market instruments
    void InstrumentSender::compareInstruments()
    {
        for (const auto& orderBookId : *activeOrderBookIds)
        {
            auto marketMapIt = marketMapByOrderBookId->find(orderBookId);
            if (marketMapIt != marketMapByOrderBookId->end())  
            {
                Instrument* marketMapInstrument = marketMapIt->second;  
                Instrument* insMapInstrument = nullptr;
                if (activeMapByOrderBookId->get(orderBookId, &insMapInstrument))  
                {
                    *marketMapInstrument = *insMapInstrument;
                }
            }
        }
    }
    void InstrumentSender::compareActiveInstruments()
    {
        for (const auto& orderBookId : newActiveOrderBookIds)
        {
            auto it = std::find(activeOrderBookIds->begin(), activeOrderBookIds->end(), orderBookId);
            if (it != activeOrderBookIds->end())
            {
                Instrument* existingInstrument = nullptr;
                if (activeMapByOrderBookId->get(orderBookId, &existingInstrument))  
                {
                    Instrument* newInstrument = nullptr;
                    if (newActiveMapByOrderBookId.get(orderBookId, &newInstrument))  
                    {
                        *existingInstrument = *newInstrument;  
                    }
                }
            }
            else
            {
                
                Instrument* newInstrument = nullptr;
                if(newActiveMapByOrderBookId.get(orderBookId,&newInstrument))
                {
                    activeMapByOrderBookId->add(orderBookId, newInstrument);
                }
            }
        } 
        for (const auto& orderBook : *activeOrderBookIds)
        {
            
            auto it = std::find(newActiveOrderBookIds.begin(), newActiveOrderBookIds.end(), orderBook);

            if (it != newActiveOrderBookIds.end())
            {
                

            }
            else
            {
                Instrument* existingIns = nullptr;
                std::ofstream outFile("lastDeleted.txt");
                if (activeMapByOrderBookId->get(orderBook, &existingIns))
                {
                    if (existingIns->id == (*nextId) - 1)
                    {
                        if (outFile.is_open()) 
                        {
                            outFile << existingIns->id;  
                            outFile.close();
                            std::cout << "Successfully wrote the integer to the file." << std::endl;
                        } 
                        else 
                        {
                            std::cerr << "Error: Could not open file for writing." << std::endl;
                        }
                        *nextId = (*nextId) - 1;
                    }
                    else if (existingIns->id < (*nextId) - 1)
                    {
                        
                        outFile << (*nextId) - 1;  
                        outFile.close();
                    }
                }
                
                activeMapByOrderBookId->remove(orderBook);
            }
        }  
        *activeOrderBookIds = newActiveOrderBookIds;
        newActiveMapByOrderBookId.clear();
        newActiveOrderBookIds.clear(); 
    }
    void InstrumentSender::fileSenderThreadHandler() 
    { 
        while (_continue) 
        {
            if (*sendInstrumentsFlag) 
            {
                size_t totalBytesSent = 0;
                compareInstruments();
                if (orderBookIds && marketMapByOrderBookId) 
                {
                    for (const auto& orderBookId : *orderBookIds) 
                    {
                        Instrument* instrument = nullptr;
                        auto it = marketMapByOrderBookId->find(orderBookId);
                        if (it != marketMapByOrderBookId->end()) 
                        {
                            instrument = it->second;
                            if (instrument) 
                            {
                                InstMessage instrumentMessage('R', *instrument);
                                size_t instrumentMessageSize = sizeof(instrumentMessage);
                                totalBytesSent = 0;
                                const char* instrumentMessageData = reinterpret_cast<const char*>(&instrumentMessage);
                                while (totalBytesSent < instrumentMessageSize)
                                {
                                    ssize_t bytesSent = send(client_fd, instrumentMessageData + totalBytesSent, instrumentMessageSize - totalBytesSent, 0);
                                    if (bytesSent == -1) 
                                    {
                                        std::cerr << "Failed to send instrument with OrderBookId: " << instrument->orderBookId << std::endl;
                                        return; // Exit the loop if sending fails
                                    }
                                    totalBytesSent += bytesSent;
                                } 
                                std::cout << "Successfully sent instrument with OrderBookId: " << instrument->orderBookId <<"fpga "<<instrument->fpgaSessionNo<< instrument->isActive <<" and Id "<<instrument->id<< " and title "<<instrument->title<<std::endl;
                            }
                        } 
                        else 
                        {
                            std::cerr << "Instrument with OrderBookId " << orderBookId << " not found in marketMap." << std::endl;
                        }
                    }
                }
                // Send count message
                CountMessage countMessage('C', *gatewayInstrumentCount);
                size_t countMessageSize = sizeof(countMessage);
                const char* countMessageData = reinterpret_cast<const char*>(&countMessage);
                totalBytesSent = 0;

                while (totalBytesSent < countMessageSize)
                {
                    ssize_t bytesSent = send(client_fd, countMessageData + totalBytesSent, countMessageSize - totalBytesSent, 0);
                    if (bytesSent == -1) 
                    {
                        std::cerr << "Failed to send instrument count message" << std::endl;
                        return; // Exit the loop if sending fails
                    }
                    totalBytesSent += bytesSent;
                }

                std::cout << "Sent gateway instrument count: " << *gatewayInstrumentCount << std::endl;
                *sendInstrumentsFlag = false;
                setMarket();
                _readFlag = true;
            }
            usleep(10);

        }
    }
    void InstrumentSender::connectionCheckerThreadHandler()
    {
        
        while (true)
        {
            if (_continue)
            {
                fd_set read_fds;
                FD_ZERO(&read_fds);
                FD_SET(client_fd, &read_fds);

                struct timeval tv;
                tv.tv_sec = 1;  // Check every second
                tv.tv_usec = 0;

                int retval = select(client_fd + 1, &read_fds, nullptr, nullptr, &tv);

                if (retval == -1)
                {
                    std::cerr << "select() error: " << strerror(errno) << std::endl;
                    close(client_fd);
                    _continue = 0;
                    _indexAck = 1;
                    _connect = 1;
                }
                else if (retval == 0)
                {
                    // No data available, continue to the next iteration
                    continue;
                }
                else
                {
                    if (FD_ISSET(client_fd, &read_fds))
                    {
                        char buffer[1];
                        int bytesRead = recv(client_fd, buffer, sizeof(buffer), MSG_PEEK);
                        if (bytesRead == 0)
                        {
                            std::cerr << "Client connection closed" << std::endl;
                            close(client_fd);
                            _continue = 0;
                            _indexAck = 1;
                            _connect = 1;
                        }
                        else if (bytesRead < 0)
                        {
                            std::cerr << "recv() error: " << strerror(errno) << std::endl;
                            close(client_fd);
                            _continue = 0;
                            _indexAck = 1;
                            _connect = 1;
                        }
                    }
                }
            }
            sleep(1); // Sleep to prevent busy waiting
        }
    }
}
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>   // Include for select() system call
#include "server.h"
#include "/opt/itchTest/Configs/configuration.h"
#include <execinfo.h>
#include </opt/itchTest/InstrumentCreater/instrumentDefinitions.h>
#define BUFFER_SIZE (1024 * 1024 * 2)

namespace Lib
{
    InstrumentSender::InstrumentSender(int part, std::vector<Instrument>* instruments, volatile bool* flag)
    : partition(part), instruments(instruments) , sendInstrumentsFlag(flag)
    {
        // Flags
        _indexAck = 0;
        _continue = 0;
        _connect = 1;
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
    static void *startConnectionCheckerThread(void *obj)
    {
        ((InstrumentSender*) obj)->connectionCheckerThreadHandler();
        return 0;
    }
    void InstrumentSender::init()
    {
        // Socket part
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0)
        {
            std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Socket creation succeeded" << std::endl;
        fflush(stdout);
        // Set up the address
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        addrlen = sizeof(address);   

         
    }
    void InstrumentSender::startThreads()
    {
        // Threads
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
    void InstrumentSender::connect()
    {
        while (true)
        {
            if (_connect)
            {
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
                    _connect = 0; // Prevent further accept attempts until the connection is lost
                    startThreads();
                }
            }
            
            sleep(1); // Sleep to prevent busy waiting
        }
    }

    void InstrumentSender::fileSenderThreadHandler() 
    {
       while (_continue) 
        {
            if (*sendInstrumentsFlag) 
            {
                if (instruments && !instruments->empty()) 
                {
                    for (auto& instrument : *instruments)
                    {
                        size_t instrumentSize = sizeof(instrument);
                        size_t totalBytesSent = 0;
                        const char* instrumentData = reinterpret_cast<const char*>(&instrument);
                        while (totalBytesSent < instrumentSize)
                        {
                            ssize_t bytesSent = send(client_fd, instrumentData + totalBytesSent, instrumentSize - totalBytesSent, 0);
                            if (bytesSent == -1) {
                                std::cerr << "Failed to send instrument information" << std::endl;
                                return;
                            }
                            totalBytesSent += bytesSent;
                        }

                        std::cout << "Sent full instrument with ID: " << instrument.id << std::endl;
                    }
                }
            }

            // Once done processing, reset the flag (if needed)
            *sendInstrumentsFlag = false;
        }

        // Sleep for a short time to prevent busy-waiting (adjust as needed)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
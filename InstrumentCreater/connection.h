/*
 * socket.h
 *
 *  Created on: Oct 5, 2023
 *      Author: RestinB
 */
#include <time.h>
#ifndef ENGINE_LIB_CONNECTION_H_
#define ENGINE_LIB_CONNECTION_H_
#define MAX_ITCH_BUFFER_SIZE		(1024 * 1024 * 768)
#define MAX_ITCH_MESSAGE_COUNT		30000000

namespace Lib
{   
    class CConnection
    {
        public:
            int _multicastSocket;
            char *_bufferPtr;
	        char *_rewinderBufferPtr;
            
            static const int _continue = 1;
            static int createUdpMulticastSocket(const char *remoteIp, const int remotePort, const char *localIp);
            static int createUdpSocket(const char *localIp, int timeoutSec);
            
            void multicastHandler();
        private:
                
                class EItchMessageState
                {
                    public:
                        typedef enum
                        {
                            None = 0,
                            Read = 1,
                            Processed = 2,
                            Writen = 3,
                        } Enum;
                };
                typedef struct __attribute__((packed))
                {
                    unsigned short length;
                    char type;
                } TItchMessageBase;
                typedef struct TItchMessage
                {
                    char *addr;
                    volatile EItchMessageState::Enum state;
                    struct TItchMessage *lastMessage;
                    timespec readTime;
                    timespec processTime;
                    timespec writeTime;
                    int rewinder;
                } TItchMessage;
    };
}

#endif /* ENGINE_LIB_CONNECTION_H_ */

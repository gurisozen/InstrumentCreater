#ifndef INSTRUMENT_DEFINITIONS_H_
#define INSTRUMENT_DEFINITIONS_H_

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
#include <fstream>
#include <cstring>

#define MAX_GATEWAY_COUNT				128	
#define MAX_INSTRUMENT_COUNT			64	// check datFiles & interface msging while changing
#define MAX_TICKSIZE_COUNT				10	// check datFiles & interface msging while changing

#pragma pack(push, 1)
class EInstrumentType
{
	public:
		typedef enum
		{
			None = 0,
			Equity = 1,
			Future = 2,
			Exchange = 3,
		} Enum;

		static void getName(EInstrumentType::Enum value, char *dst, int size)
		{
			switch (value)
			{
				case EInstrumentType::None: strncpy(dst, "None", size); break;
				case EInstrumentType::Equity: strncpy(dst, "Equity", size); break;
				case EInstrumentType::Future: strncpy(dst, "Future", size); break;
				case EInstrumentType::Exchange: strncpy(dst, "Exchange", size); break;
				default: strncpy(dst, "NOT FOUND", size);
			}
		}
};
class EOrderBookType
{
	public:
		typedef enum
		{
			None = 0,
			Full = 1
		} Enum;

		static void getName(EOrderBookType::Enum value, char *dst, int size)
		{
			switch (value)
			{
				case EOrderBookType::None: strncpy(dst, "None", size); break;
				case EOrderBookType::Full: strncpy(dst, "Full", size); break;
				default: strncpy(dst, "NOT FOUND", size);
			}
		}
};
class EGatewayType
{
	public:
		typedef enum
		{
			None,
			Internal,
			BistechFix,
			BistechOuch,
			BistechItch,
			BistechFpgaMarket,
			BistechFpgaOrder,
		} Enum;

		static void getName(EGatewayType::Enum value, char *dst, int size)
		{
			switch (value)
			{
				case EGatewayType::None: strncpy(dst, "None", size); break;
				case EGatewayType::Internal: strncpy(dst, "Internal", size); break;
				case EGatewayType::BistechFix: strncpy(dst, "BistechFix", size); break;
				case EGatewayType::BistechOuch: strncpy(dst, "BistechOuch", size); break;
				case EGatewayType::BistechItch: strncpy(dst, "BistechItch", size); break;
				case EGatewayType::BistechFpgaMarket: strncpy(dst, "BistechFpgaMarket", size); break;
				case EGatewayType::BistechFpgaOrder: strncpy(dst, "BistechFpgaOrder", size); break;
				default: strncpy(dst, "NOT FOUND", size);
			}
		}
};

struct __attribute__((packed)) TickSizeMap {
    int start;
    int end;
    int tickSize;
};

struct __attribute__((packed)) Instrument {
    int id;
    unsigned orderBookId;
    char title[128];
    char internalName[64];
    EInstrumentType::Enum instrumentType; // Use enum from provided class
    EOrderBookType::Enum orderBookType;   // Use enum from provided class
    EGatewayType::Enum marketGatewayType; // Use enum from provided class
    int marketGatewayNo;
    EGatewayType::Enum orderGatewayType; // Use enum from provided class
    int originalOrderGatewayNo;
    int orderGroupGatewayIdxList[MAX_GATEWAY_COUNT]; // Assuming MAX_GATEWAY_COUNT is 10
    int decimalsInPrice;
    int contractSize;
    int quantityMultiplier;
    int allowShortSell;
    TickSizeMap tickSizeMap[MAX_TICKSIZE_COUNT]; // Assuming MAX_TICKSIZE_COUNT is 10
    int leggedInstrument;
    int farLegOrderBookId;
    int nearLegOrderBookId;
	int isActive;
    int fpgaSessionNo;
};
struct __attribute__((packed)) WriteInstrument {
    int id;
    unsigned orderBookId;
    char title[128];
    char internalName[64];
    EInstrumentType::Enum instrumentType; // Use enum from provided class
    EOrderBookType::Enum orderBookType;   // Use enum from provided class
    EGatewayType::Enum marketGatewayType; // Use enum from provided class
    int marketGatewayNo;
    EGatewayType::Enum orderGatewayType; // Use enum from provided class
    int originalOrderGatewayNo;
    int orderGroupGatewayIdxList[MAX_GATEWAY_COUNT]; // Assuming MAX_GATEWAY_COUNT is 10
    int decimalsInPrice;
    int contractSize;
    int quantityMultiplier;
    int allowShortSell;
    TickSizeMap tickSizeMap[MAX_TICKSIZE_COUNT]; // Assuming MAX_TICKSIZE_COUNT is 10
    int leggedInstrument;
    int farLegOrderBookId;
    int nearLegOrderBookId;
    int fpgaSessionNo;
};

struct __attribute__((packed)) InstMessage 
{
    int length;          
    char messageType;           
    Instrument instrument; 
    
    // Default constructor
    InstMessage() : messageType(0){
        setLength();
    }

    // Constructor for 'R' type (instrument message)
    InstMessage(char type, const Instrument& instrument) : messageType(type), instrument(instrument) {
        setLength();
    }

    // Function to calculate the length based on the message type
    void setLength() {
        if (messageType == 'R') {
            length = sizeof(messageType) + sizeof(instrument); // 1 byte for char + size of Instrument
        } else {
            length = 0; // For undefined message types
        }
    }
};
struct __attribute__((packed)) CountMessage {
    int length;          
    char messageType;    
	int count;             
         
    

    // Default constructor
    CountMessage() : messageType(0), count(0) {
        setLength();
    }

    // Constructor for 'C' type (count message)
    CountMessage(char type, int value) : messageType(type), count(value) {
        setLength();
    }

    void setLength() {
        if (messageType == 'C') {
            length = sizeof(messageType) + sizeof(count); // 1 byte for char + 4 bytes for count (int)
        } 
    }
};
struct __attribute__((packed)) WriteMessage {
    int length;          
    char messageType;    
	int status;  
	// Default constructor
    WriteMessage() : messageType(0), status(0) {
        setLength();
    }

    // Constructor for 'C' type (count message)
    WriteMessage(char type, int value) : messageType(type), status(value) {
        setLength();
    }

    void setLength() {
        if (messageType == 'S') {
            length = sizeof(messageType) + sizeof(status); // 1 byte for char + 4 bytes for status (int)
        } 
    }             
};
void writeInt(std::ofstream &file, int value);
void writeInstrument(std::ofstream &file, WriteInstrument &instrument);
#pragma pack(pop)
#endif /* INSTRUMENT_DEFINITIONS_H_ */

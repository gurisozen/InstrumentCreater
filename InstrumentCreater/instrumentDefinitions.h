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
	
};

#pragma pack(pop)
#endif /* INSTRUMENT_DEFINITIONS_H_ */

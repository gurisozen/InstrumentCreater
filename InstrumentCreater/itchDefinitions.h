/*
 * itchDefinitions.h
 *
 *  Created on: Jan 4, 2023
 *      Author: RestinB
 */

#ifndef ITCH_ITCHDEFINITIONS_H_
#define ITCH_ITCHDEFINITIONS_H_

#include <sys/types.h>


#define ITCH_MESSAGE_TYPE_INSERTORDER			'A'
#define ITCH_MESSAGE_TYPE_EXECUTEORDER			'E'
#define ITCH_MESSAGE_TYPE_DELETEORDER			'D'
#define ITCH_MESSAGE_TYPE_ORDERBOOKSTATE		'O'
#define ITCH_MESSAGE_TYPE_EQUILIBRIUM			'Z'
#define ITCH_MESSAGE_TYPE_TRADE					'P'
#define ITCH_MESSAGE_TYPE_SECOND				'T'
#define ITCH_MESSAGE_TYPE_ORDERBOOKDIRECTORY	'R'
#define ITCH_MESSAGE_TYPE_TICKTABLEENTRY	    'L'

namespace Itch
{
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
		char session[10];
		ulong sequenceNumber;
		ushort messageCount;
	} TItchBlock;


	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
	} TItchMessageBase;
	typedef struct __attribute__((packed))
			{
				char session[10];
				ulong sequence;
				ushort count;
			} TItchRewinderMessage;
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

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
	} TItchMessageNanoBase;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		ulong orderId;
		uint orderBookId;
	} TItchMessageWOrderIdBase;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		uint orderBookId;
	} TItchMessageWOutOrderIdBase;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		uint orderBookId;
		ulong tickSize;
		uint priceFrom;
		uint priceTo;
	} TItchMessagePriceTickTableEntry;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		uint orderBookId;
		char symbol[32];
		char longName[32];
		char stockMarket[12];
		char financialProduct;
		char tradingCurrency[3];
		ushort decimalsInPrice;
		ushort decimalsInNominalPrice;
		char skip[20];
	} TItchMessageOrderBookDirectory;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint second;
	} TItchMessageSecond;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		uint orderBookId;
		char stateName[20];
	} TItchMessageOrderBookState;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		ulong matchId;
		char comboGroupId[4];
		char side;
		ulong quantity;
		uint orderBookId;
		uint tradePrice;
		char skip[16];
	} TItchMessageTrade;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		ulong orderId;
		uint orderBookId;
		char side;
		uint orderBookPosition;
		ulong quantity;
		uint price;
		char skip[3];
	} TItchMessageAddOrder;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		ulong orderId;
		uint orderBookId;
		char side;
	} TItchMessageDeleteOrder;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		ulong orderId;
		uint orderBookId;
		char side;
		uint newOrderBookPosition;
		ulong quantity;
		uint price;
		char skip[2];
	} TItchMessageReplaceOrder;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		ulong orderId;
		uint orderBookId;
		char side;
		ulong executedQuantity;
		ulong matchId;
		char comboGroupId[4];
		char skip[14];
	} TItchMessageOrderExecute;

	typedef struct __attribute__((packed))
	{
		ushort length;
		char type;
		uint nanoseconds;
		uint orderBookId;
		ulong eqBidQuantity;
		ulong eqAskQuantity;
		uint eqPrice;
		uint bestBidPrice;
		uint bestAskPrice;
		ulong bestBidQuantity;
		ulong bestAskQuantity;
	} TItchMessageEquilibriumPriceUpdate;

}

#endif /* ITCH_ITCHDEFINITIONS_H_ */

#include "instrumentGateway.h"
#include "instrumentCreater.h"
#include <iostream>
#include <unordered_map>
InstrumentGateway::InstrumentGateway() 
    : instrumentCount(0)
      {
        orderBookIds.reserve(1000); 
        sender=new Libs::InstrumentSender(1, &instrumentCount,&nextId, &marketMapByOrderBookId, &activeInsMapByOrderBookId,&activeOrderBookIds, &orderBookIds, &sendInstrumentsFlag);
      }

InstrumentGateway::~InstrumentGateway() 
{
    cleanupInstruments();      
    delete sender;             
}

int InstrumentGateway::init() 
{
    sender->init();
    return 0;
}
//Function to create Instrument creaters
//This is the step that the file is being read and the empty market instruments are mapped.
void InstrumentGateway::createInstrumentCreaters() 
{
    std::condition_variable cv;
    std::mutex mtx;
    for (int i = 0; i < 10; ++i) {
        
        InstrumentCreater* creater = new InstrumentCreater(i + 1, &instrumentCount, &marketMapByOrderBookId,&orderBookIds,  &cv, &mtx);
        instrumentCreaters.push_back(creater);
        creater->start();
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&creater]() { return creater->isFinished(); });
        std::cout <<  "Total number of instruments: " << instrumentCount << std::endl;
        creater->stop();
        delete creater;
    }
}
//Function to open current instrument files and get current active instruments
//Next Id is being set by this function.
void InstrumentGateway::setFileInstruments(const char *filename)
{
	FILE *_instrumentFile;
	_instrumentFile = fopen(filename, "r+b");
    if (!_instrumentFile)
    {
        _instrumentFile = fopen(filename, "w+b");
        if (!_instrumentFile) {
            std::cerr << "Error: Unable to create or open the file: " << filename << std::endl;
            return; 
        }
    }
    nextId = 1;
	int tag = 0;
	fread(&tag, sizeof(int), 1, _instrumentFile);
	int version = 0;
	fread(&version, sizeof(int), 1, _instrumentFile);
	// Record count
	int recordCount = 0;
	fread(&recordCount, sizeof(int), 1, _instrumentFile);
	//Header Size
	int headerSize = 0;
	fread(&headerSize, sizeof(int), 1, _instrumentFile);
	for (int idx = 0; idx < recordCount; ++idx)
	{
		//Read the active Instruments
        //We have a write instrument and a regular instrument object Regular instrument object includes isActive field.
        WriteInstrument* fileInstrument = new WriteInstrument();
        int entrySize = 0;
        fread(&entrySize, sizeof(int), 1, _instrumentFile);  
        fread(fileInstrument, entrySize, 1, _instrumentFile);  
        // Print the fields from the WriteInstrument object
        std::cout <<  "Current active instrument with Id " << fileInstrument->id << "and OrderBookId "<<fileInstrument->orderBookId<< "and title " << fileInstrument->title << std::endl;
        // Create a new Instrument object and fill in the fields from file instrument
        Instrument* activeInstrument = new Instrument();
        // Copy fields from WriteInstrument to Instrument
        activeInstrument->id = fileInstrument->id;
        activeInstrument->orderBookId = fileInstrument->orderBookId;
        strncpy(activeInstrument->title, fileInstrument->title, sizeof(activeInstrument->title));
        strncpy(activeInstrument->internalName, fileInstrument->internalName, sizeof(activeInstrument->internalName));
        activeInstrument->instrumentType = fileInstrument->instrumentType;
        activeInstrument->orderBookType = fileInstrument->orderBookType;
        activeInstrument->marketGatewayType = fileInstrument->marketGatewayType;
        activeInstrument->marketGatewayNo = fileInstrument->marketGatewayNo;
        activeInstrument->orderGatewayType = fileInstrument->orderGatewayType;
        activeInstrument->originalOrderGatewayNo = fileInstrument->originalOrderGatewayNo;
        // Copy array fields (orderGroupGatewayIdxList)
        for (int i = 0; i < MAX_GATEWAY_COUNT; ++i) {
            activeInstrument->orderGroupGatewayIdxList[i] = fileInstrument->orderGroupGatewayIdxList[i];
        }
        // Copy remaining fields
        activeInstrument->decimalsInPrice = fileInstrument->decimalsInPrice;
        activeInstrument->contractSize = fileInstrument->contractSize;
        activeInstrument->quantityMultiplier = fileInstrument->quantityMultiplier;
        activeInstrument->allowShortSell = fileInstrument->allowShortSell;
        // Copy tickSizeMap array
        for (int i = 0; i < MAX_TICKSIZE_COUNT; ++i) {
            activeInstrument->tickSizeMap[i] = fileInstrument->tickSizeMap[i];
        }
        activeInstrument->leggedInstrument = fileInstrument->leggedInstrument;
        activeInstrument->farLegOrderBookId = fileInstrument->farLegOrderBookId;
        activeInstrument->nearLegOrderBookId = fileInstrument->nearLegOrderBookId;
        activeInstrument->fpgaSessionNo = fileInstrument->fpgaSessionNo;
        // Set the isActive field to 1
        activeInstrument->isActive = 1;
        // Add the active instrument to the map and order book list
        activeInsMapByOrderBookId.add(activeInstrument->orderBookId, activeInstrument);
        activeOrderBookIds.push_back(activeInstrument->orderBookId);
        //check this because instruments are not in order
        if (nextId <= fileInstrument->id )
        {
            // Update the nextId
            nextId = (fileInstrument->id) + 1;
        }
        // Clean up (free fileInstrument if no longer needed)
        delete fileInstrument;
	}
}
// Start the InstrumentGateway and the InstrumentCreaters
void InstrumentGateway::start() {
    setFileInstruments("instrument.dat");
    createInstrumentCreaters();  
    sendInstrumentsFlag = true;
    sender->start();
}
// Disconnect and stop all processes
void InstrumentGateway::disconnect() 
{
    _continue = 0;  
}

// Cleanup function to deallocate dynamically allocated Instruments
void InstrumentGateway::cleanupInstruments() {
    for (auto& instrument : instruments) {
        delete instrument;  // Deallocate each Instrument pointer
    }
    instruments.clear();    // Clear the vector of pointers
}

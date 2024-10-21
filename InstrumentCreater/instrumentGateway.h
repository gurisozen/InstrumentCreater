#ifndef INSTRUMENT_GATEWAY_H_
#define INSTRUMENT_GATEWAY_H_

#include <vector>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "instrumentCreater.h"  // Include the header for InstrumentCreater

#define MAX_ITCH_BUFFER_SIZE (1024 * 1024 * 768)

class InstrumentGateway {
public:
    // Constructor and Destructor
    InstrumentGateway();
    ~InstrumentGateway();

    // Public methods for initializing, starting, stopping the gateway
    int init();             // Initializes the gateway
    void start();           // Starts the gateway and instrument creators
    void stop();            // Stops the gateway and instrument creators
    void startConnect();    // Starts the connection process
    void disconnect();      // Disconnects and cleans up
    void printInstrumentCount();  // Prints the current count of instruments
    void setFileInstruments(const char *filename);
    void compareInstruments();
private:
    FILE* outputFile;         // Output file (if needed)
    volatile int _continue;   // Control flag to signal stop/start
    char* _bufferPtr;         // Pointer to the buffer
    char _buffer[MAX_ITCH_BUFFER_SIZE];  // The buffer for instrument data
    int nextId;
    volatile bool sendInstrumentsFlag = false;  // Flag to signal when to send instruments
    Libs::InstrumentSender* sender;              // Pointer to the sender object (to send instruments)

    std::vector<Instrument*> instruments;


    int instrumentCount;  

    std::unordered_map<unsigned int, Instrument*> marketMapByOrderBookId;  // Declare the marketMapByOrderBookId here
    Lib::Dictionary::CKhashInt<Instrument*> activeInsMapByOrderBookId;
    std::vector<unsigned int> orderBookIds ; 
    std::vector<unsigned int> activeOrderBookIds;     



    std::vector<InstrumentCreater*> instrumentCreaters;  // Vector holding pointers to InstrumentCreaters

    // Private methods
    void createInstrumentCreaters();  // Creates 10 InstrumentCreaters
    void startInstrumentCreaters();   // Starts all InstrumentCreaters
    void stopInstrumentCreaters();    // Stops all InstrumentCreaters
    void cleanupInstruments();        // Cleans up dynamically allocated instruments
};

#endif /* INSTRUMENT_GATEWAY_H_ */

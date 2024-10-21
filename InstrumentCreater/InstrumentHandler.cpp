#include "instrumentGateway.h"
#include <csignal>
#include <iostream>

// Global pointer to the InstrumentGateway
InstrumentGateway* gateway = nullptr;

// Signal handler to gracefully stop the gateway
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Stopping the gateway..." << std::endl;
    if (gateway) {
        gateway->disconnect();  // Gracefully disconnect the gateway
    }
    exit(signum);
}

int main() {
    // Register signal handler for Ctrl+C or termination signals
    signal(SIGINT, signalHandler);

    // Instantiate and initialize the InstrumentGateway
    gateway = new InstrumentGateway();
    
    // Initialize the gateway (this could initialize the sender, etc.)
    if (gateway->init() != 0) {
        std::cerr << "Failed to initialize the InstrumentGateway." << std::endl;
        delete gateway;  // Clean up if initialization fails
        return -1;
    }

    // Start the gateway (creates and starts the InstrumentCreaters and InstrumentSender)
    std::cout << "Starting the InstrumentGateway..." << std::endl;
    gateway->start();

    // Wait for all instruments to be processed and sent by the sender
    gateway->disconnect();

    // Clean up the gateway when done
    std::cout << "Stopping the InstrumentGateway..." << std::endl;
    delete gateway;

    return 0;
}

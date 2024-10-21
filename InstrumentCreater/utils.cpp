#include "instrumentDefinitions.h"

// Define the functions here
void writeInt(std::ofstream &file, int value) {
    file.write(reinterpret_cast<char*>(&value), sizeof(value));
}

void writeInstrument(std::ofstream &file, WriteInstrument &instrument) {
    file.write(reinterpret_cast<char*>(&instrument), sizeof(instrument));
}

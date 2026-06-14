#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <istream>

namespace serial3 {
typedef struct {
    std::string fileDestination;
    std::fstream fileStream;
    std::stringstream fileBuffer;
}Serial;

Serial* OpenSerial(std::string fileDestination, int* error);
int CloseSerial(Serial* serial);

int SerialWriteToFile(Serial* serial, std::string data);
int SerialRemoveLine(Serial* serial, int lineNumber, const char* lineEnding = "\r\n");
int SerialWriteLine(Serial* serial, int lineNumber, std::string data);
int SerialReadLine(Serial* serial, int lineNumber);
int SerialReadFromFile(Serial* serial);

int SerializeString(Serial* serial, const std::string& key, const std::string& value);
int SerializeInt(Serial* serial, const std::string& key, int value);
int SerializeBool(Serial* serial, const std::string& key, bool value);
int DeserializeString(Serial* serial, const std::string& key, std::string& outValue);
int DeserializeInt(Serial* serial, const std::string& key, int& outValue);
int DeserializeBool(Serial* serial, const std::string& key, bool& outValue);

int SerialFlush(Serial* serial);

}
#include <serial3/serial.hpp>
#include <serial3/serial.h>
#include <vector>
#include <iterator>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace serial3 {

Serial* OpenSerial(std::string fileDestination, int* error) {
    Serial* serial = new Serial();
    serial->fileDestination = std::move(fileDestination);
    serial->fileStream.open(serial->fileDestination, std::ios::in | std::ios::out | std::ios::app);
    if (!serial->fileStream.is_open()) {
        *error = 1;
        delete serial;
        return nullptr;
    }
    *error = 0;
    return serial;
}

int CloseSerial(Serial* serial) {
    if (serial == nullptr) {
        return 1;
    }
    if (serial->fileStream.is_open()) {
        serial->fileStream.close();
    }
    delete serial;
    return 0;
}

int SerialWriteToFile(Serial* serial, std::string data) {
    if (serial == nullptr || !serial->fileStream.is_open()) {
        return 1;
    }
    serial->fileStream.clear();
    serial->fileStream.seekp(0, std::ios::end);
    serial->fileStream << data;
    serial->fileStream.flush();
    return serial->fileStream.fail() ? 1 : 0;
}

static int ReadAllLines(Serial* serial, std::vector<std::string>& lines) {
    if (serial == nullptr || !serial->fileStream.is_open()) {
        return 1;
    }
    serial->fileStream.clear();
    serial->fileStream.seekg(0);
    std::string line;
    while (std::getline(serial->fileStream, line)) {
        lines.push_back(line);
    }
    return 0;
}

static int RewriteLines(Serial* serial, const std::vector<std::string>& lines, const char* lineEnding) {
    serial->fileStream.close();
    serial->fileStream.open(serial->fileDestination, std::ios::out | std::ios::trunc);
    if (!serial->fileStream.is_open()) {
        return 1;
    }
    for (const auto& l : lines) {
        serial->fileStream << l << lineEnding;
    }
    serial->fileStream.flush();
    return serial->fileStream.fail() ? 1 : 0;
}

int SerialRemoveLine(Serial* serial, int lineNumber, const char* lineEnding) {
    if (serial == nullptr) {
        return 1;
    }
    std::vector<std::string> lines;
    if (ReadAllLines(serial, lines) != 0) {
        return 1;
    }
    if (lineNumber < 0 || lineNumber >= static_cast<int>(lines.size())) {
        return 1;
    }
    lines.erase(lines.begin() + lineNumber);
    return RewriteLines(serial, lines, lineEnding);
}
    
int SerialWriteLine(Serial* serial, int lineNumber, std::string data) {
    if (serial == nullptr) {
        return 1;
    }
    std::vector<std::string> lines;
    if (ReadAllLines(serial, lines) != 0) {
        return 1;
    }
    if (lineNumber < 0 || lineNumber > static_cast<int>(lines.size())) {
        return 1;
    }
    lines.insert(lines.begin() + lineNumber, std::move(data));
    return RewriteLines(serial, lines, "\n");
}

int SerialReadLine(Serial* serial, int lineNumber) {
    if (serial == nullptr) {
        return 1;
    }
    std::vector<std::string> lines;
    if (ReadAllLines(serial, lines) != 0) {
        return 1;
    }
    if (lineNumber < 0 || lineNumber >= static_cast<int>(lines.size())) {
        return 1;
    }
    serial->fileBuffer.str(lines[lineNumber]);
    serial->fileBuffer.clear();
    return 0;
}

int SerialReadFromFile(Serial* serial) {
    if (serial == nullptr) {
        return 1;
    }
    if (!serial->fileStream.is_open()) {
        return 1;
    }
    serial->fileStream.clear();
    serial->fileStream.seekg(0);
    std::string content((std::istreambuf_iterator<char>(serial->fileStream)), std::istreambuf_iterator<char>());
    serial->fileBuffer.str(std::move(content));
    serial->fileBuffer.clear();
    return 0;
}

int SerialFlush(Serial* serial) {
    if (serial == nullptr || !serial->fileStream.is_open()) {
        return 1;
    }
    serial->fileStream.flush();
    return serial->fileStream.fail() ? 1 : 0;
}

static std::string Trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}

static std::string LoadBuffer(Serial* serial) {
    if (serial == nullptr) {
        return {};
    }
    if (!serial->fileBuffer.str().empty()) {
        return serial->fileBuffer.str();
    }
    if (serial->fileStream.is_open()) {
        SerialReadFromFile(serial);
        return serial->fileBuffer.str();
    }
    return {};
}

static std::string MakeRecord(const std::string& key, const std::string& value) {
    return key + "=" + value + "\n";
}

int SerializeString(Serial* serial, const std::string& key, const std::string& value) {
    if (serial == nullptr) {
        return 1;
    }
    const std::string record = MakeRecord(key, value);
    serial->fileBuffer << record;
    if (serial->fileStream.is_open()) {
        serial->fileStream << record;
        serial->fileStream.flush();
    }
    return serial->fileBuffer.fail() ? 1 : 0;
}

int SerializeInt(Serial* serial, const std::string& key, int value) {
    return SerializeString(serial, key, std::to_string(value));
}

int SerializeBool(Serial* serial, const std::string& key, bool value) {
    return SerializeString(serial, key, value ? "true" : "false");
}

static bool TryParseValue(const std::string& text, const std::string& key, std::string& outValue) {
    std::string data = text;
    size_t pos = 0;
    while (pos < data.size()) {
        size_t end = data.find('\n', pos);
        if (end == std::string::npos) {
            end = data.size();
        }
        std::string line = Trim(data.substr(pos, end - pos));
        if (!line.empty()) {
            size_t split = line.find('=');
            if (split != std::string::npos && Trim(line.substr(0, split)) == key) {
                outValue = line.substr(split + 1);
                return true;
            }
        }
        if (end == data.size()) {
            break;
        }
        pos = end + 1;
    }
    return false;
}

int DeserializeString(Serial* serial, const std::string& key, std::string& outValue) {
    std::string text = LoadBuffer(serial);
    if (text.empty()) {
        return 1;
    }
    return TryParseValue(text, key, outValue) ? 0 : 1;
}

int DeserializeInt(Serial* serial, const std::string& key, int& outValue) {
    std::string value;
    if (DeserializeString(serial, key, value) != 0) {
        return 1;
    }
    try {
        outValue = std::stoi(value);
        return 0;
    } catch (const std::exception&) {
        return 1;
    }
}

int DeserializeBool(Serial* serial, const std::string& key, bool& outValue) {
    std::string value;
    if (DeserializeString(serial, key, value) != 0) {
        return 1;
    }
    std::string normalized = Trim(value);
    if (normalized == "1" || normalized == "true" || normalized == "TRUE") {
        outValue = true;
        return 0;
    }
    if (normalized == "0" || normalized == "false" || normalized == "FALSE") {
        outValue = false;
        return 0;
    }
    return 1;
}

} // namespace serial3

extern "C" {

serial3_handle* s3Open(const char* fileDestination, int* error) {
    const char* path = fileDestination ? fileDestination : "";
    return reinterpret_cast<serial3_handle*>(serial3::OpenSerial(std::string(path), error));
}

int s3Close(serial3_handle* handle) {
    return serial3::CloseSerial(reinterpret_cast<serial3::Serial*>(handle));
}

int s3WriteToFile(serial3_handle* handle, const char* data) {
    if (!data) {
        return 1;
    }
    return serial3::SerialWriteToFile(reinterpret_cast<serial3::Serial*>(handle), std::string(data));
}

int s3RemoveLine(serial3_handle* handle, int lineNumber, const char* lineEnding) {
    const char* ending = lineEnding ? lineEnding : "\r\n";
    return serial3::SerialRemoveLine(reinterpret_cast<serial3::Serial*>(handle), lineNumber, ending);
}

int s3WriteLine(serial3_handle* handle, int lineNumber, const char* data) {
    if (!data) {
        return 1;
    }
    return serial3::SerialWriteLine(reinterpret_cast<serial3::Serial*>(handle), lineNumber, std::string(data));
}

int s3ReadLine(serial3_handle* handle, int lineNumber) {
    return serial3::SerialReadLine(reinterpret_cast<serial3::Serial*>(handle), lineNumber);
}

int s3ReadFromFile(serial3_handle* handle) {
    return serial3::SerialReadFromFile(reinterpret_cast<serial3::Serial*>(handle));
}

int s3Flush(serial3_handle* handle) {
    return serial3::SerialFlush(reinterpret_cast<serial3::Serial*>(handle));
}

int s3SerializeString(serial3_handle* handle, const char* key, const char* value) {
    if (!key || !value) {
        return 1;
    }
    return serial3::SerializeString(reinterpret_cast<serial3::Serial*>(handle), std::string(key), std::string(value));
}

int s3SerializeInt(serial3_handle* handle, const char* key, int value) {
    if (!key) {
        return 1;
    }
    return serial3::SerializeInt(reinterpret_cast<serial3::Serial*>(handle), std::string(key), value);
}

int s3SerializeBool(serial3_handle* handle, const char* key, int value) {
    if (!key) {
        return 1;
    }
    return serial3::SerializeBool(reinterpret_cast<serial3::Serial*>(handle), std::string(key), value != 0);
}

int s3DeserializeString(serial3_handle* handle, const char* key, char* out, size_t outSize) {
    if (!key || !out || outSize == 0) {
        return 1;
    }
    std::string value;
    if (serial3::DeserializeString(reinterpret_cast<serial3::Serial*>(handle), std::string(key), value) != 0) {
        return 1;
    }
    size_t copySize = std::min(outSize - 1, value.size());
    std::memcpy(out, value.data(), copySize);
    out[copySize] = '\0';
    return 0;
}

int s3DeserializeInt(serial3_handle* handle, const char* key, int* outValue) {
    if (!key || !outValue) {
        return 1;
    }
    return serial3::DeserializeInt(reinterpret_cast<serial3::Serial*>(handle), std::string(key), *outValue);
}

int s3DeserializeBool(serial3_handle* handle, const char* key, int* outValue) {
    if (!key || !outValue) {
        return 1;
    }
    bool value = false;
    if (serial3::DeserializeBool(reinterpret_cast<serial3::Serial*>(handle), std::string(key), value) != 0) {
        return 1;
    }
    *outValue = value ? 1 : 0;
    return 0;
}

size_t s3GetBufferSize(serial3_handle* handle) {
    serial3::Serial* serial = reinterpret_cast<serial3::Serial*>(handle);
    if (!serial) {
        return 0;
    }
    return serial->fileBuffer.str().size();
}

int s3GetBuffer(serial3_handle* handle, char* out, size_t outSize) {
    if (!out || outSize == 0) {
        return 1;
    }
    serial3::Serial* serial = reinterpret_cast<serial3::Serial*>(handle);
    if (!serial) {
        return 1;
    }
    std::string contents = serial->fileBuffer.str();
    size_t copySize = std::min(outSize - 1, contents.size());
    if (copySize > 0) {
        std::memcpy(out, contents.data(), copySize);
    }
    out[copySize] = '\0';
    return 0;
}

}
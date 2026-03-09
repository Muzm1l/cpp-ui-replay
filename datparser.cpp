#include "datparser.h"
#include "parser_format_config.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace {

std::string familyToString(MessageFamily family) {
    switch (family) {
        case MessageFamily::Own: return "own";
        case MessageFamily::Target: return "target";
        case MessageFamily::Torpedo: return "torpedo";
        case MessageFamily::Tube: return "tube";
        case MessageFamily::Event: return "event";
        case MessageFamily::Sonar: return "sonar";
        case MessageFamily::Unknown: return "unknown";
    }
    return "unknown";
}

template <size_t N>
std::string joinColumns(const std::array<const char*, N>& columns) {
    std::ostringstream stream;
    for (size_t index = 0; index < columns.size(); ++index) {
        if (index > 0) {
            stream << ",";
        }
        stream << columns[index];
    }
    return stream.str();
}

std::string jsonEscape(const std::string& value) {
    std::ostringstream escaped;
    for (char c : value) {
        switch (c) {
            case '\\': escaped << "\\\\"; break;
            case '"': escaped << "\\\""; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default: escaped << c; break;
        }
    }
    return escaped.str();
}

} // namespace

DatParser::DatParser()
    : targetType0dSource(-1),
      torpedoType0dSource(-1),
      tubeType0dSource(-1),
      unknownPacketCount(0),
      droppedRowCount(0) {}

DatParser::~DatParser() {}

bool DatParser::parseFile(const std::string& filepath) {
    packets.clear();
    ownRecords.clear();
    targetRecords.clear();
    torpedoRecords.clear();
    tubeRecords.clear();
    eventRecords.clear();
    sonarRecords.clear();
    badLineLogs.clear();
    messageCounts.clear();
    fieldMinMax.clear();
    partialPayloadByKey.clear();

    targetType0dSource = -1;
    torpedoType0dSource = -1;
    tubeType0dSource = -1;
    unknownPacketCount = 0;
    droppedRowCount = 0;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return false;
    }

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        if (line.empty()) {
            continue;
        }
        parsePacketLine(line, lineNum);
    }

    file.close();
    validateDecodedData();

    std::cout << "Parsed " << packets.size() << " packets from " << lineNum << " lines" << std::endl;
    if (!badLineLogs.empty()) {
        std::cout << "Bad lines: " << badLineLogs.size() << std::endl;
    }
    return true;
}

bool DatParser::parsePacketLine(const std::string& line, int lineNum) {
    size_t commaPos = line.find(',');
    if (commaPos == std::string::npos) {
        badLineLogs.push_back("line " + std::to_string(lineNum) + ": missing timestamp delimiter");
        return false;
    }

    std::string timestamp = line.substr(0, commaPos);
    std::string hexData = line.substr(commaPos + 1);

    hexData.erase(std::remove_if(hexData.begin(), hexData.end(),
                                 [](unsigned char c) { return std::isspace(c) != 0; }),
                  hexData.end());

    if (hexData.empty()) {
        badLineLogs.push_back("line " + std::to_string(lineNum) + ": empty hex payload");
        return false;
    }

    if ((hexData.length() % 2) != 0) {
        badLineLogs.push_back("line " + std::to_string(lineNum) + ": odd hex length");
        return false;
    }

    if (hexData.length() < ParserFormat::HEADER_HEX_CHARS) {
        badLineLogs.push_back("line " + std::to_string(lineNum) + ": too short for header");
        return false;
    }

    std::vector<uint8_t> allBytes;
    if (!hexToBytes(hexData, allBytes)) {
        badLineLogs.push_back("line " + std::to_string(lineNum) + ": invalid hex characters");
        return false;
    }

    if (allBytes.size() < ParserFormat::HEADER_HEX_BYTES) {
        badLineLogs.push_back("line " + std::to_string(lineNum) + ": incomplete header bytes");
        return false;
    }

    DatPacket packet;
    packet.timestamp = timestamp;
    packet.source = allBytes[0];
    packet.type = allBytes[20];
    packet.payload.assign(allBytes.begin() + static_cast<long>(ParserFormat::HEADER_HEX_BYTES), allBytes.end());

    packets.push_back(packet);

    MessageFamily family = classifyPacket(packet);
    messageCounts[familyToString(family)]++;

    switch (family) {
        case MessageFamily::Own:
            decodeOwnTrajectory(packet);
            break;
        case MessageFamily::Target:
            decodeTargetTrajectory(packet);
            break;
        case MessageFamily::Torpedo:
            decodeTorpedoTrajectory(packet);
            break;
        case MessageFamily::Tube:
            decodeTubeTrajectory(packet);
            break;
        case MessageFamily::Event:
            decodeEvent(packet);
            break;
        case MessageFamily::Sonar:
            decodeSonar(packet);
            break;
        case MessageFamily::Unknown:
            ++unknownPacketCount;
            break;
    }

    return true;
}

std::string DatParser::hexToString(const std::string& hexStr) {
    std::string result;
    for (size_t index = 0; index + 1 < hexStr.size(); index += 2) {
        uint8_t value = (hexCharToUint8(hexStr[index]) << 4) | hexCharToUint8(hexStr[index + 1]);
        result.push_back(static_cast<char>(value));
    }
    return result;
}

bool DatParser::hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes) {
    outBytes.clear();
    outBytes.reserve(hexStr.size() / 2);

    for (size_t index = 0; index + 1 < hexStr.size(); index += 2) {
        char left = hexStr[index];
        char right = hexStr[index + 1];
        if (!std::isxdigit(static_cast<unsigned char>(left)) || !std::isxdigit(static_cast<unsigned char>(right))) {
            outBytes.clear();
            return false;
        }

        uint8_t value = static_cast<uint8_t>((hexCharToUint8(left) << 4) | hexCharToUint8(right));
        outBytes.push_back(value);
    }

    return true;
}

uint8_t DatParser::hexCharToUint8(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
    return 0;
}

uint32_t DatParser::bytesToUint32LE(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + 3 >= data.size()) return 0;
    return static_cast<uint32_t>(data[offset]) |
           (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) |
           (static_cast<uint32_t>(data[offset + 3]) << 24);
}

int32_t DatParser::bytesToInt32LE(const std::vector<uint8_t>& data, size_t offset) {
    return static_cast<int32_t>(bytesToUint32LE(data, offset));
}

float DatParser::bytesToFloatLE(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + 3 >= data.size()) return 0.0f;
    uint32_t intVal = bytesToUint32LE(data, offset);
    float floatVal = 0.0f;
    std::memcpy(&floatVal, &intVal, sizeof(float));
    return floatVal;
}

double DatParser::bytesToDoubleLE(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + 7 >= data.size()) return 0.0;
    uint64_t low = static_cast<uint64_t>(bytesToUint32LE(data, offset));
    uint64_t high = static_cast<uint64_t>(bytesToUint32LE(data, offset + 4));
    uint64_t intVal = low | (high << 32);
    double value = 0.0;
    std::memcpy(&value, &intVal, sizeof(double));
    return value;
}

std::string DatParser::bytesToAsciiZ(const std::vector<uint8_t>& data, size_t offset, size_t maxLen) {
    std::string output;
    if (offset >= data.size() || maxLen == 0) {
        return output;
    }

    size_t limit = std::min(offset + maxLen, data.size());
    for (size_t index = offset; index < limit; ++index) {
        if (data[index] == 0) {
            break;
        }
        output.push_back(static_cast<char>(data[index]));
    }
    return output;
}

bool DatParser::readU32(const std::vector<uint8_t>& data, size_t& offset, uint32_t& outValue) {
    if (offset + 4 > data.size()) return false;
    outValue = bytesToUint32LE(data, offset);
    offset += 4;
    return true;
}

bool DatParser::readI32(const std::vector<uint8_t>& data, size_t& offset, int32_t& outValue) {
    if (offset + 4 > data.size()) return false;
    outValue = bytesToInt32LE(data, offset);
    offset += 4;
    return true;
}

bool DatParser::readF32(const std::vector<uint8_t>& data, size_t& offset, float& outValue) {
    if (offset + 4 > data.size()) return false;
    outValue = bytesToFloatLE(data, offset);
    offset += 4;
    return true;
}

bool DatParser::readF64(const std::vector<uint8_t>& data, size_t& offset, double& outValue) {
    if (offset + 8 > data.size()) return false;
    outValue = bytesToDoubleLE(data, offset);
    offset += 8;
    return true;
}

bool DatParser::readAsciiZ(const std::vector<uint8_t>& data, size_t& offset, size_t maxLen, std::string& outValue) {
    if (offset >= data.size()) {
        outValue.clear();
        return false;
    }

    outValue = bytesToAsciiZ(data, offset, maxLen);
    size_t consumed = outValue.size();
    if (offset + consumed < data.size() && data[offset + consumed] == 0) {
        ++consumed;
    }
    offset += consumed;
    return true;
}

MessageFamily DatParser::classifyPacket(const DatPacket& packet) {
    switch (packet.type) {
        case 0x07:
            return MessageFamily::Own;
        case 0x0d:
            if (targetType0dSource == -1 || packet.source == static_cast<uint8_t>(targetType0dSource)) {
                targetType0dSource = packet.source;
                return MessageFamily::Target;
            }
            if (torpedoType0dSource == -1 || packet.source == static_cast<uint8_t>(torpedoType0dSource)) {
                torpedoType0dSource = packet.source;
                return MessageFamily::Torpedo;
            }
            if (tubeType0dSource == -1 || packet.source == static_cast<uint8_t>(tubeType0dSource)) {
                tubeType0dSource = packet.source;
            }
            return MessageFamily::Tube;
        case 0x0e:
        case 0x0f:
        case 0x10:
        case 0x11:
            return MessageFamily::Event;
        case 0x26:
        case 0x27:
            return MessageFamily::Sonar;
        default:
            return MessageFamily::Unknown;
    }
}

std::vector<uint8_t> DatParser::assemblePayload(const DatPacket& packet, size_t requiredBytes, const std::string& key) {
    std::vector<uint8_t> combined;
    auto existingIt = partialPayloadByKey.find(key);
    if (existingIt != partialPayloadByKey.end()) {
        combined = existingIt->second;
        partialPayloadByKey.erase(existingIt);
    }

    combined.insert(combined.end(), packet.payload.begin(), packet.payload.end());
    if (combined.size() < requiredBytes) {
        partialPayloadByKey[key] = combined;
        return {};
    }

    return combined;
}

void DatParser::updateFieldMinMax(const std::string& fieldName, double value) {
    auto it = fieldMinMax.find(fieldName);
    if (it == fieldMinMax.end()) {
        fieldMinMax[fieldName] = std::make_pair(value, value);
        return;
    }

    it->second.first = std::min(it->second.first, value);
    it->second.second = std::max(it->second.second, value);
}

bool DatParser::isFiniteFloat(float value) const {
    return std::isfinite(static_cast<double>(value));
}

bool DatParser::isWithinRange(float value, float minValue, float maxValue) const {
    return value >= minValue && value <= maxValue;
}

void DatParser::decodeOwnTrajectory(const DatPacket& packet) {
    uint32_t trackId = bytesToUint32LE(packet.payload, 0);
    std::string assemblyKey = "own:" + std::to_string(packet.source) + ":" + std::to_string(trackId);
    std::vector<uint8_t> payload = assemblePayload(packet, ParserFormat::OWN_MIN_BYTES, assemblyKey);
    if (payload.empty()) {
        return;
    }

    OwnRecord record;
    size_t offset = ParserFormat::OWN_START_OFFSET;
    readF32(payload, offset, record.time);
    readF32(payload, offset, record.course);
    readF32(payload, offset, record.speed);
    readF32(payload, offset, record.depth);
    readF32(payload, offset, record.x);
    readF32(payload, offset, record.y);

    if (!isFiniteFloat(record.time) || !isFiniteFloat(record.course) || !isFiniteFloat(record.speed) || !isFiniteFloat(record.depth)) {
        ++droppedRowCount;
        return;
    }

    if (!isWithinRange(record.course, ParserFormat::COURSE_MIN, ParserFormat::COURSE_MAX) ||
        !isWithinRange(record.speed, ParserFormat::SPEED_MIN, ParserFormat::SPEED_MAX) ||
        !isWithinRange(record.depth, ParserFormat::DEPTH_MIN, ParserFormat::DEPTH_MAX)) {
        ++droppedRowCount;
        return;
    }

    updateFieldMinMax("own.time", record.time);
    updateFieldMinMax("own.course", record.course);
    updateFieldMinMax("own.speed", record.speed);
    updateFieldMinMax("own.depth", record.depth);
    ownRecords.push_back(record);
}

void DatParser::decodeTargetTrajectory(const DatPacket& packet) {
    uint32_t trackId = bytesToUint32LE(packet.payload, 0);
    std::string assemblyKey = "target:" + std::to_string(packet.source) + ":" + std::to_string(trackId);
    std::vector<uint8_t> payload = assemblePayload(packet, ParserFormat::TARGET_MIN_BYTES, assemblyKey);
    if (payload.empty()) {
        return;
    }

    TargetRecord record;
    size_t offset = ParserFormat::TARGET_START_OFFSET;
    readF32(payload, offset, record.time);
    readF32(payload, offset, record.bearing);
    readF32(payload, offset, record.range);
    readF32(payload, offset, record.course);
    readF32(payload, offset, record.speed);
    readF32(payload, offset, record.depth);
    readF32(payload, offset, record.f1);
    readF32(payload, offset, record.f2);
    readF32(payload, offset, record.f3);
    readF32(payload, offset, record.f4);
    readF32(payload, offset, record.f5);
    readF32(payload, offset, record.f6);
    readF32(payload, offset, record.f7);
    readF32(payload, offset, record.f8);
    readF32(payload, offset, record.x);
    readF32(payload, offset, record.y);

    if (!isFiniteFloat(record.time) || !isFiniteFloat(record.course) || !isFiniteFloat(record.speed) || !isFiniteFloat(record.depth) || !isFiniteFloat(record.range)) {
        ++droppedRowCount;
        return;
    }

    if (!isWithinRange(record.course, ParserFormat::COURSE_MIN, ParserFormat::COURSE_MAX) ||
        !isWithinRange(record.speed, ParserFormat::SPEED_MIN, ParserFormat::SPEED_MAX) ||
        !isWithinRange(record.depth, ParserFormat::DEPTH_MIN, ParserFormat::DEPTH_MAX) ||
        !isWithinRange(record.range, ParserFormat::RANGE_MIN, ParserFormat::RANGE_MAX)) {
        ++droppedRowCount;
        return;
    }

    updateFieldMinMax("target.time", record.time);
    updateFieldMinMax("target.bearing", record.bearing);
    updateFieldMinMax("target.range", record.range);
    updateFieldMinMax("target.course", record.course);
    updateFieldMinMax("target.speed", record.speed);
    updateFieldMinMax("target.depth", record.depth);
    targetRecords.push_back(record);
}

void DatParser::decodeTorpedoTrajectory(const DatPacket& packet) {
    uint32_t trackId = bytesToUint32LE(packet.payload, 0);
    std::string assemblyKey = "torpedo:" + std::to_string(packet.source) + ":" + std::to_string(trackId);
    std::vector<uint8_t> payload = assemblePayload(packet, ParserFormat::TUBE_MIN_BYTES, assemblyKey);
    if (payload.empty()) {
        return;
    }

    TorpedoRecord record;
    size_t offset = ParserFormat::TUBE_START_OFFSET;
    readF32(payload, offset, record.time);
    readF32(payload, offset, record.bearing);
    readF32(payload, offset, record.range);
    readF32(payload, offset, record.course);
    readF32(payload, offset, record.speed);
    readF32(payload, offset, record.depth);
    readF32(payload, offset, record.sonar);
    readF32(payload, offset, record.wake);
    readF32(payload, offset, record.x);
    readF32(payload, offset, record.y);

    if (!isFiniteFloat(record.time) || !isFiniteFloat(record.course) || !isFiniteFloat(record.speed) || !isFiniteFloat(record.depth) || !isFiniteFloat(record.range)) {
        ++droppedRowCount;
        return;
    }

    if (!isWithinRange(record.course, ParserFormat::COURSE_MIN, ParserFormat::COURSE_MAX) ||
        !isWithinRange(record.speed, ParserFormat::SPEED_MIN, ParserFormat::SPEED_MAX) ||
        !isWithinRange(record.depth, ParserFormat::DEPTH_MIN, ParserFormat::DEPTH_MAX) ||
        !isWithinRange(record.range, ParserFormat::RANGE_MIN, ParserFormat::RANGE_MAX)) {
        ++droppedRowCount;
        return;
    }

    updateFieldMinMax("torpedo.time", record.time);
    updateFieldMinMax("torpedo.range", record.range);
    updateFieldMinMax("torpedo.speed", record.speed);
    torpedoRecords.push_back(record);
}

void DatParser::decodeTubeTrajectory(const DatPacket& packet) {
    uint32_t trackId = bytesToUint32LE(packet.payload, 0);
    std::string assemblyKey = "tube:" + std::to_string(packet.source) + ":" + std::to_string(trackId);
    std::vector<uint8_t> payload = assemblePayload(packet, ParserFormat::TUBE_MIN_BYTES, assemblyKey);
    if (payload.empty()) {
        return;
    }

    TubeRecord record;
    size_t offset = ParserFormat::TUBE_START_OFFSET;
    readF32(payload, offset, record.time);
    readF32(payload, offset, record.bearing);
    readF32(payload, offset, record.range);
    readF32(payload, offset, record.course);
    readF32(payload, offset, record.speed);
    readF32(payload, offset, record.depth);
    readF32(payload, offset, record.sonar);
    readF32(payload, offset, record.wake);
    readF32(payload, offset, record.x);
    readF32(payload, offset, record.y);

    if (!isFiniteFloat(record.time) || !isFiniteFloat(record.course) || !isFiniteFloat(record.speed) || !isFiniteFloat(record.depth) || !isFiniteFloat(record.range)) {
        ++droppedRowCount;
        return;
    }

    if (!isWithinRange(record.course, ParserFormat::COURSE_MIN, ParserFormat::COURSE_MAX) ||
        !isWithinRange(record.speed, ParserFormat::SPEED_MIN, ParserFormat::SPEED_MAX) ||
        !isWithinRange(record.depth, ParserFormat::DEPTH_MIN, ParserFormat::DEPTH_MAX) ||
        !isWithinRange(record.range, ParserFormat::RANGE_MIN, ParserFormat::RANGE_MAX)) {
        ++droppedRowCount;
        return;
    }

    updateFieldMinMax("tube.time", record.time);
    updateFieldMinMax("tube.range", record.range);
    updateFieldMinMax("tube.speed", record.speed);
    tubeRecords.push_back(record);

    TorpedoRecord torpedoRecord;
    torpedoRecord.time = record.time;
    torpedoRecord.bearing = record.bearing;
    torpedoRecord.range = record.range;
    torpedoRecord.course = record.course;
    torpedoRecord.speed = record.speed;
    torpedoRecord.depth = record.depth;
    torpedoRecord.sonar = record.sonar;
    torpedoRecord.wake = record.wake;
    torpedoRecord.x = record.x;
    torpedoRecord.y = record.y;
    torpedoRecords.push_back(torpedoRecord);
}

void DatParser::decodeEvent(const DatPacket& packet) {
    std::string assemblyKey = "event:" + std::to_string(packet.source) + ":" + std::to_string(packet.type);
    std::vector<uint8_t> payload = assemblePayload(packet, ParserFormat::EVENT_MIN_BYTES, assemblyKey);
    if (payload.empty()) {
        return;
    }

    EventRecord record;
    size_t offset = ParserFormat::EVENT_START_OFFSET;
    readF32(payload, offset, record.time);
    readU32(payload, offset, record.sender);
    readU32(payload, offset, record.type);

    if (offset < payload.size()) {
        size_t textLen = payload.size() - offset;
        record.name = bytesToAsciiZ(payload, offset, textLen);
    }
    if (record.name.empty()) {
        record.name = "UNKNOWN";
    }

    if (!isFiniteFloat(record.time)) {
        ++droppedRowCount;
        return;
    }

    updateFieldMinMax("event.time", record.time);
    eventRecords.push_back(record);
}

void DatParser::decodeSonar(const DatPacket& packet) {
    std::string assemblyKey = "sonar:" + std::to_string(packet.source) + ":" + std::to_string(packet.type);
    std::vector<uint8_t> payload = assemblePayload(packet, ParserFormat::SONAR_RECORD_BYTES, assemblyKey);
    if (payload.empty()) {
        return;
    }

    size_t offset = ParserFormat::SONAR_START_OFFSET;
    while (offset + ParserFormat::SONAR_RECORD_BYTES <= payload.size()) {
        SonarRecord record;
        readF32(payload, offset, record.time);
        readU32(payload, offset, record.weapon);
        readF32(payload, offset, record.beamX);
        readF32(payload, offset, record.beamY);
        readF32(payload, offset, record.intensity);

        if (!isFiniteFloat(record.time) || !isFiniteFloat(record.beamX) || !isFiniteFloat(record.beamY) || !isFiniteFloat(record.intensity)) {
            ++droppedRowCount;
            continue;
        }

        updateFieldMinMax("sonar.time", record.time);
        updateFieldMinMax("sonar.intensity", record.intensity);
        sonarRecords.push_back(record);
    }

    if (offset < payload.size()) {
        std::vector<uint8_t> remainder(payload.begin() + static_cast<long>(offset), payload.end());
        partialPayloadByKey[assemblyKey] = remainder;
    }
}

void DatParser::decodeMetadata(const DatPacket& packet) {
    std::cout << "  (Metadata packet, " << packet.payload.size() << " bytes)" << std::endl;
}

bool DatParser::writeOwnCsv(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << joinColumns(ParserFormat::OWN_COLUMNS) << "\n";
    for (const auto& record : ownRecords) {
        file << record.time << ","
             << record.course << ","
             << record.speed << ","
             << record.depth << ","
             << record.x << ","
             << record.y << "\n";
    }
    return true;
}

bool DatParser::writeTargetCsv(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << joinColumns(ParserFormat::TARGET_COLUMNS) << "\n";
    for (const auto& record : targetRecords) {
        file << record.time << ","
             << record.bearing << ","
             << record.range << ","
             << record.course << ","
             << record.speed << ","
             << record.depth << ","
             << record.f1 << ","
             << record.f2 << ","
             << record.f3 << ","
             << record.f4 << ","
             << record.f5 << ","
             << record.f6 << ","
             << record.f7 << ","
             << record.f8 << ","
             << record.x << ","
             << record.y << "\n";
    }
    return true;
}

bool DatParser::writeTorpedoCsv(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << joinColumns(ParserFormat::TUBE_COLUMNS) << "\n";
    for (const auto& record : torpedoRecords) {
        file << record.time << ","
             << record.bearing << ","
             << record.range << ","
             << record.course << ","
             << record.speed << ","
             << record.depth << ","
             << record.sonar << ","
             << record.wake << ","
             << record.x << ","
             << record.y << "\n";
    }
    return true;
}

bool DatParser::writeTubeCsv(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << joinColumns(ParserFormat::TUBE_COLUMNS) << "\n";
    for (const auto& record : tubeRecords) {
        file << record.time << ","
             << record.bearing << ","
             << record.range << ","
             << record.course << ","
             << record.speed << ","
             << record.depth << ","
             << record.sonar << ","
             << record.wake << ","
             << record.x << ","
             << record.y << "\n";
    }
    return true;
}

bool DatParser::writeEventCsv(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << joinColumns(ParserFormat::EVENT_COLUMNS) << "\n";
    for (const auto& record : eventRecords) {
        file << record.time << ","
             << record.sender << ","
             << record.type << ","
             << record.name << "\n";
    }
    return true;
}

bool DatParser::writeSonarCsv(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << joinColumns(ParserFormat::SONAR_COLUMNS) << "\n";
    for (const auto& record : sonarRecords) {
        file << record.time << ","
             << record.weapon << ","
             << record.beamX << ","
             << record.beamY << ","
             << record.intensity << "\n";
    }
    return true;
}

bool DatParser::writeSummaryJson(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing" << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"message_counts\": {\n";

    size_t countIndex = 0;
    for (const auto& pair : messageCounts) {
        file << "    \"" << jsonEscape(pair.first) << "\": " << pair.second;
        ++countIndex;
        file << (countIndex < messageCounts.size() ? ",\n" : "\n");
    }

    file << "  },\n";
    file << "  \"unknown_types\": " << unknownPacketCount << ",\n";
    file << "  \"dropped_rows\": " << droppedRowCount << ",\n";
    file << "  \"bad_lines\": " << badLineLogs.size() << ",\n";

    file << "  \"row_counts\": {\n";
    file << "    \"own\": " << ownRecords.size() << ",\n";
    file << "    \"target\": " << targetRecords.size() << ",\n";
    file << "    \"torpedo\": " << torpedoRecords.size() << ",\n";
    file << "    \"tube\": " << tubeRecords.size() << ",\n";
    file << "    \"event\": " << eventRecords.size() << ",\n";
    file << "    \"sonar\": " << sonarRecords.size() << "\n";
    file << "  },\n";

    file << "  \"field_min_max\": {\n";
    size_t rangeIndex = 0;
    for (const auto& pair : fieldMinMax) {
        file << "    \"" << jsonEscape(pair.first) << "\": { \"min\": " << pair.second.first
             << ", \"max\": " << pair.second.second << " }";
        ++rangeIndex;
        file << (rangeIndex < fieldMinMax.size() ? ",\n" : "\n");
    }
    file << "  }\n";

    file << "}\n";
    return true;
}

bool DatParser::validateDecodedData() {
    bool allValid = true;

    auto checkMonotonic = [&](const std::string& name, const std::vector<float>& times) {
        for (size_t index = 1; index < times.size(); ++index) {
            if (times[index] < times[index - 1]) {
                std::cerr << "Warning: non-monotonic time in " << name << " at index " << index << std::endl;
                allValid = false;
                break;
            }
        }
    };

    std::vector<float> ownTimes;
    ownTimes.reserve(ownRecords.size());
    for (const auto& row : ownRecords) {
        ownTimes.push_back(row.time);
        if (!isWithinRange(row.course, ParserFormat::COURSE_MIN, ParserFormat::COURSE_MAX) ||
            !isWithinRange(row.speed, ParserFormat::SPEED_MIN, ParserFormat::SPEED_MAX) ||
            !isWithinRange(row.depth, ParserFormat::DEPTH_MIN, ParserFormat::DEPTH_MAX)) {
            allValid = false;
        }
    }

    std::vector<float> targetTimes;
    targetTimes.reserve(targetRecords.size());
    for (const auto& row : targetRecords) {
        targetTimes.push_back(row.time);
        if (!isWithinRange(row.course, ParserFormat::COURSE_MIN, ParserFormat::COURSE_MAX) ||
            !isWithinRange(row.speed, ParserFormat::SPEED_MIN, ParserFormat::SPEED_MAX) ||
            !isWithinRange(row.depth, ParserFormat::DEPTH_MIN, ParserFormat::DEPTH_MAX) ||
            !isWithinRange(row.range, ParserFormat::RANGE_MIN, ParserFormat::RANGE_MAX)) {
            allValid = false;
        }
    }

    std::vector<float> tubeTimes;
    tubeTimes.reserve(tubeRecords.size());
    for (const auto& row : tubeRecords) {
        tubeTimes.push_back(row.time);
    }

    std::vector<float> eventTimes;
    eventTimes.reserve(eventRecords.size());
    for (const auto& row : eventRecords) {
        eventTimes.push_back(row.time);
    }

    std::vector<float> sonarTimes;
    sonarTimes.reserve(sonarRecords.size());
    for (const auto& row : sonarRecords) {
        sonarTimes.push_back(row.time);
    }

    checkMonotonic("Own", ownTimes);
    checkMonotonic("Target", targetTimes);
    checkMonotonic("Tube", tubeTimes);
    checkMonotonic("Event", eventTimes);
    checkMonotonic("Sonar", sonarTimes);

    return allValid;
}

void DatParser::printStatistics() {
    std::cout << "\n=== Packet Statistics ===" << std::endl;
    std::cout << "Total packets: " << packets.size() << std::endl;
    std::cout << "Own trajectory records: " << ownRecords.size() << std::endl;
    std::cout << "Target trajectory records: " << targetRecords.size() << std::endl;
    std::cout << "Torpedo trajectory records: " << torpedoRecords.size() << std::endl;
    std::cout << "Tube trajectory records: " << tubeRecords.size() << std::endl;
    std::cout << "Event records: " << eventRecords.size() << std::endl;
    std::cout << "Sonar records: " << sonarRecords.size() << std::endl;
    std::cout << "Unknown packet types: " << unknownPacketCount << std::endl;
    std::cout << "Dropped rows: " << droppedRowCount << std::endl;

    std::map<uint8_t, int> typeCount;
    std::map<uint8_t, int> sourceCount;
    for (const auto& packet : packets) {
        typeCount[packet.type]++;
        sourceCount[packet.source]++;
    }

    std::cout << "\nPacket types:" << std::endl;
    for (const auto& pair : typeCount) {
        std::cout << "  0x" << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<int>(pair.first) << ": " << std::dec << pair.second << " records" << std::endl;
    }

    std::cout << "\nSources:" << std::endl;
    for (const auto& pair : sourceCount) {
        std::cout << "  0x" << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<int>(pair.first) << ": " << std::dec << pair.second << " packets" << std::endl;
    }
}

std::string DatParser::getTimestampForOutput(const std::string& datTimestamp) {
    return datTimestamp;
}

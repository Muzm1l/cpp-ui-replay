#ifndef DATPARSER_H
#define DATPARSER_H

#include "datpacket.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <memory>

enum class MessageFamily {
    Own,
    Target,
    Torpedo,
    Tube,
    Event,
    Sonar,
    Unknown
};

class DatParser {
public:
    DatParser();
    ~DatParser();
    
    // Main parsing function
    bool parseFile(const std::string& filepath);
    
    // CSV output generation
    bool writeOwnCsv(const std::string& filepath);
    bool writeTargetCsv(const std::string& filepath);
    bool writeTorpedoCsv(const std::string& filepath);
    bool writeTubeCsv(const std::string& filepath);
    bool writeEventCsv(const std::string& filepath);
    bool writeSonarCsv(const std::string& filepath);
    bool writeSummaryJson(const std::string& filepath);
    
    // Statistics
    void printStatistics();
    bool validateDecodedData();
    
    // Packet access
    const std::vector<DatPacket>& getPackets() const { return packets; }
    
    // Data access by type
    const std::vector<OwnRecord>& getOwnRecords() const { return ownRecords; }
    const std::vector<TargetRecord>& getTargetRecords() const { return targetRecords; }
    const std::vector<TorpedoRecord>& getTorpedoRecords() const { return torpedoRecords; }
    const std::vector<TubeRecord>& getTubeRecords() const { return tubeRecords; }
    const std::vector<EventRecord>& getEventRecords() const { return eventRecords; }
    const std::vector<SonarRecord>& getSonarRecords() const { return sonarRecords; }
    const std::vector<std::string>& getBadLineLogs() const { return badLineLogs; }

private:
    std::vector<DatPacket> packets;
    std::vector<OwnRecord> ownRecords;
    std::vector<TargetRecord> targetRecords;
    std::vector<TorpedoRecord> torpedoRecords;
    std::vector<TubeRecord> tubeRecords;
    std::vector<EventRecord> eventRecords;
    std::vector<SonarRecord> sonarRecords;
    std::vector<std::string> badLineLogs;
    int targetType0dSource;
    int torpedoType0dSource;
    int tubeType0dSource;
    size_t unknownPacketCount;
    size_t droppedRowCount;
    std::map<std::string, size_t> messageCounts;
    std::map<std::string, std::pair<double, double>> fieldMinMax;
    std::map<std::string, std::vector<uint8_t>> partialPayloadByKey;
    
    // Parsing helpers
    bool parsePacketLine(const std::string& line, int lineNum);
    std::string hexToString(const std::string& hexStr);
    bool hexToBytes(const std::string& hexStr, std::vector<uint8_t>& outBytes);
    uint8_t hexCharToUint8(char c);
    uint32_t bytesToUint32LE(const std::vector<uint8_t>& data, size_t offset);
    int32_t bytesToInt32LE(const std::vector<uint8_t>& data, size_t offset);
    float bytesToFloatLE(const std::vector<uint8_t>& data, size_t offset);
    double bytesToDoubleLE(const std::vector<uint8_t>& data, size_t offset);
    std::string bytesToAsciiZ(const std::vector<uint8_t>& data, size_t offset, size_t maxLen);
    bool readU32(const std::vector<uint8_t>& data, size_t& offset, uint32_t& outValue);
    bool readI32(const std::vector<uint8_t>& data, size_t& offset, int32_t& outValue);
    bool readF32(const std::vector<uint8_t>& data, size_t& offset, float& outValue);
    bool readF64(const std::vector<uint8_t>& data, size_t& offset, double& outValue);
    bool readAsciiZ(const std::vector<uint8_t>& data, size_t& offset, size_t maxLen, std::string& outValue);
    
    // Packet type decoders
    void decodeOwnTrajectory(const DatPacket& packet);
    void decodeTargetTrajectory(const DatPacket& packet);
    void decodeTorpedoTrajectory(const DatPacket& packet);
    void decodeTubeTrajectory(const DatPacket& packet);
    void decodeEvent(const DatPacket& packet);
    void decodeSonar(const DatPacket& packet);
    void decodeMetadata(const DatPacket& packet);  // Type 0x05
    MessageFamily classifyPacket(const DatPacket& packet);
    std::vector<uint8_t> assemblePayload(const DatPacket& packet, size_t requiredBytes, const std::string& key);
    void updateFieldMinMax(const std::string& fieldName, double value);
    bool isFiniteFloat(float value) const;
    bool isWithinRange(float value, float minValue, float maxValue) const;
    
    // Utilities
    std::string getTimestampForOutput(const std::string& datTimestamp);
};

#endif // DATPARSER_H

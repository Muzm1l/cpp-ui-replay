#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <memory>
#include <cstdint>
#include <cstddef>

struct DatPacket {
    std::string timestamp;
    uint8_t source;
    uint8_t type;
    std::vector<uint8_t> payload;

    DatPacket() : source(0), type(0) {}
};

struct OwnRecord {
    float time;
    float course;
    float speed;
    float depth;
    float x;
    float y;

    OwnRecord() : time(0), course(0), speed(0), depth(0), x(0), y(0) {}
};

struct TargetRecord {
    float time;
    float bearing;
    float range;
    float course;
    float speed;
    float depth;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    float x;
    float y;

    TargetRecord() : time(0), bearing(0), range(0), course(0), speed(0),
                     depth(0), f1(0), f2(0), f3(0), f4(0), f5(0), f6(0),
                     f7(0), f8(0), x(0), y(0) {}
};

struct TorpedoRecord {
    float time;
    float bearing;
    float range;
    float course;
    float speed;
    float depth;
    float sonar;
    float wake;
    float x;
    float y;

    TorpedoRecord() : time(0), bearing(0), range(0), course(0), speed(0),
                      depth(0), sonar(0), wake(0), x(0), y(0) {}
};

struct TubeRecord {
    float time;
    float bearing;
    float range;
    float course;
    float speed;
    float depth;
    float sonar;
    float wake;
    float x;
    float y;

    TubeRecord() : time(0), bearing(0), range(0), course(0), speed(0),
                   depth(0), sonar(0), wake(0), x(0), y(0) {}
};

struct EventRecord {
    uint32_t sender;
    float systemTime;
    float engagementTime;
    float torpedoTime;

    EventRecord() : sender(0), systemTime(0), engagementTime(0), torpedoTime(0) {}
};

struct SonarRecord {
    float time;
    uint32_t weapon;
    float beamX;
    float beamY;
    float intensity;

    SonarRecord() : time(0), weapon(0), beamX(0), beamY(0), intensity(0) {}
};

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

#endif // PARSER_H

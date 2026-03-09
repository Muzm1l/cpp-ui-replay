#ifndef DATPACKET_H
#define DATPACKET_H

#include <string>
#include <vector>
#include <cstdint>

// Packet structure for binary .dat file
struct DatPacket {
    std::string timestamp;      // e.g., "17_02_2026_14_03_54_020"
    uint8_t  source;            // Source ID from header
    uint8_t  type;              // Packet type (0x07, 0x0d, 0x05, etc.)
    std::vector<uint8_t> payload;  // Raw binary payload
    
    DatPacket() : source(0), type(0) {}
};

// Own trajectory record (6 fields)
struct OwnRecord {
    float time;
    float course;
    float speed;
    float depth;
    float x;
    float y;
    
    OwnRecord() : time(0), course(0), speed(0), depth(0), x(0), y(0) {}
};

// Target trajectory record (16 fields)
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

// Torpedo trajectory record (10 fields)
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

// Tube trajectory record (10 fields)
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

// Event record (4 fields per record)
struct EventRecord {
    float time;
    uint32_t sender;
    uint32_t type;
    std::string name;
    
    EventRecord() : time(0), sender(0), type(0) {}
};

// Sonar record (5 fields per record)
struct SonarRecord {
    float time;       // Sonar time (seconds from start)
    uint32_t weapon;  // Weapon ID
    float beamX;      // Beam X coordinate
    float beamY;      // Beam Y coordinate
    float intensity;  // Intensity/Power level
    
    SonarRecord() : time(0), weapon(0), beamX(0), beamY(0), intensity(0) {}
};

#endif // DATPACKET_H

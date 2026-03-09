#ifndef PARSER_FORMAT_CONFIG_H
#define PARSER_FORMAT_CONFIG_H

#include <array>
#include <cstddef>

namespace ParserFormat {

inline constexpr std::array<const char*, 6> OWN_COLUMNS = {
    "time", "course", "speed", "depth", "x", "y"
};

inline constexpr std::array<const char*, 16> TARGET_COLUMNS = {
    "time", "bearing", "range", "course", "speed", "depth",
    "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "x", "y"
};

inline constexpr std::array<const char*, 10> TUBE_COLUMNS = {
    "time", "bearing", "range", "course", "speed", "depth", "sonar", "wake", "x", "y"
};

inline constexpr std::array<const char*, 4> EVENT_COLUMNS = {
    "sender", "systemTime", "engagementTime", "torpedoTime"
};

inline constexpr std::array<const char*, 5> SONAR_COLUMNS = {
    "time", "weapon", "beamX", "beamY", "intensity"
};

inline constexpr size_t HEADER_HEX_BYTES = 2;
inline constexpr size_t HEADER_HEX_CHARS = HEADER_HEX_BYTES * 2;

inline constexpr size_t OWN_START_OFFSET = 32;
inline constexpr size_t OWN_FIELD_COUNT = 6;
inline constexpr size_t TARGET_START_OFFSET = 32;
inline constexpr size_t TARGET_FIELD_COUNT = 16;
inline constexpr size_t TUBE_START_OFFSET = 32;
inline constexpr size_t TUBE_FIELD_COUNT = 10;
inline constexpr size_t EVENT_START_OFFSET = 0;
inline constexpr size_t SONAR_START_OFFSET = 0;

inline constexpr size_t F32_SIZE = 4;
inline constexpr size_t OWN_MIN_BYTES = OWN_FIELD_COUNT * F32_SIZE;
inline constexpr size_t TARGET_MIN_BYTES = 64;   // 16 floats need 64 bytes minimum
inline constexpr size_t TUBE_MIN_BYTES = 40;    // 10 floats need 40 bytes minimum
inline constexpr size_t EVENT_MIN_BYTES = 16;    // 1 uint32 + 3 floats need 16 bytes
inline constexpr size_t SONAR_RECORD_BYTES = 20;

inline constexpr float COURSE_MIN = 0.0f;
inline constexpr float COURSE_MAX = 360.0f;
inline constexpr float SPEED_MIN = 0.0f;
inline constexpr float SPEED_MAX = 200.0f;
inline constexpr float DEPTH_MIN = -12000.0f;
inline constexpr float DEPTH_MAX = 12000.0f;
inline constexpr float RANGE_MIN = 0.0f;
inline constexpr float RANGE_MAX = 2000000.0f;

} // namespace ParserFormat

#endif // PARSER_FORMAT_CONFIG_H

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
    "time", "sender", "type", "name"
};

inline constexpr std::array<const char*, 5> SONAR_COLUMNS = {
    "time", "weapon", "beamX", "beamY", "intensity"
};

inline constexpr size_t HEADER_HEX_BYTES = 28;
inline constexpr size_t HEADER_HEX_CHARS = HEADER_HEX_BYTES * 2;

inline constexpr size_t OWN_START_OFFSET = 8;
inline constexpr size_t OWN_FIELD_COUNT = 6;
inline constexpr size_t TARGET_START_OFFSET = 0;
inline constexpr size_t TARGET_FIELD_COUNT = 16;
inline constexpr size_t TUBE_START_OFFSET = 0;
inline constexpr size_t TUBE_FIELD_COUNT = 10;
inline constexpr size_t EVENT_START_OFFSET = 0;
inline constexpr size_t SONAR_START_OFFSET = 0;

inline constexpr size_t F32_SIZE = 4;
inline constexpr size_t OWN_MIN_BYTES = OWN_START_OFFSET + OWN_FIELD_COUNT * F32_SIZE;
inline constexpr size_t TARGET_MIN_BYTES = TARGET_START_OFFSET + TARGET_FIELD_COUNT * F32_SIZE;
inline constexpr size_t TUBE_MIN_BYTES = TUBE_START_OFFSET + TUBE_FIELD_COUNT * F32_SIZE;
inline constexpr size_t EVENT_MIN_BYTES = 12;
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

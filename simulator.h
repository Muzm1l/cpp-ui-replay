#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstdint>
#include <string>
#include <vector>

namespace Simulator {

struct Sample {
    std::string timestamp;
    double ownSpeedKnots;
    double ownHeadingDeg;
    double targetSpeedKnots;
    double targetHeadingDeg;
    double torpedoSpeedKnots;
    double torpedoHeadingDeg;
    double targetX;
    double targetY;
    double torpedoX;
    double torpedoY;
};

std::vector<Sample> generateSimulatorSamples(int durationSeconds = 180, int stepSeconds = 1, std::uint32_t seed = 0);
bool generateSimulatorCsv(const std::string& filePath, int durationSeconds = 180, int stepSeconds = 1);

} // namespace Simulator

#endif // SIMULATOR_H

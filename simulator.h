#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Simulator {

struct TargetState {
    double x;
    double y;
    double speedKnots;
    double headingDeg;
};

struct Sample {
    std::string timestamp;
    double ownSpeedKnots;
    double ownHeadingDeg;
    double ownX;
    double ownY;
    double targetSpeedKnots;
    double targetHeadingDeg;
    double torpedoSpeedKnots;
    double torpedoHeadingDeg;
    double targetX;
    double targetY;
    double torpedoX;
    double torpedoY;
    std::vector<TargetState> targets;
    std::size_t selectedTargetIndex;
};

std::vector<Sample> generateSimulatorSamples(int durationSeconds = 180,
                                             int stepSeconds = 1,
                                             std::uint32_t seed = 0,
                                             std::size_t targetCount = 4,
                                             std::size_t selectedTargetIndex = 0,
                                             std::size_t launchTubeIndex = 0);
bool generateSimulatorCsv(const std::string& filePath,
                          int durationSeconds = 180,
                          int stepSeconds = 1,
                          std::size_t targetCount = 4,
                          std::size_t selectedTargetIndex = 0,
                          std::size_t launchTubeIndex = 0);

// New aliases for clearer naming
std::vector<Sample> generateScenarioSamples(int durationSeconds = 180,
                                            int stepSeconds = 1,
                                            std::uint32_t seed = 0,
                                            std::size_t targetCount = 4,
                                            std::size_t selectedTargetIndex = 0,
                                            std::size_t launchTubeIndex = 0);
bool generateScenarioCsv(const std::string& filePath,
                         int durationSeconds = 180,
                         int stepSeconds = 1,
                         std::size_t targetCount = 4,
                         std::size_t selectedTargetIndex = 0,
                         std::size_t launchTubeIndex = 0);

} // namespace Simulator

#endif // SIMULATOR_H

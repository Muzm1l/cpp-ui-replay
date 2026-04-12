#include "simulator.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kKnotsToMetersPerSecond = 0.514444;
constexpr std::uint32_t kStableScenarioSeed = 20260412u;
constexpr int kTubeCount = 3;

bool isForwardTube(int tubeIndex) {
    // Tube 1 and Tube 2 are forward-facing, Tube 3 is backward-facing.
    return tubeIndex < 2;
}

double toRadians(double degrees) {
    return degrees * (kPi / 180.0);
}

double toDegrees(double radians) {
    double degrees = radians * (180.0 / kPi);
    if (degrees < 0.0) {
        degrees += 360.0;
    }
    return degrees;
}

std::string formatTimestamp(const std::chrono::system_clock::time_point& timePoint) {
    const std::time_t timestamp = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tmSnapshot{};
#if defined(_WIN32)
    localtime_s(&tmSnapshot, &timestamp);
#else
    localtime_r(&timestamp, &tmSnapshot);
#endif

    std::ostringstream stream;
    stream << std::put_time(&tmSnapshot, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

} // namespace

namespace Simulator {

std::vector<Sample> generateSimulatorSamples(int durationSeconds,
                                             int stepSeconds,
                                             std::uint32_t seed,
                                             std::size_t targetCount,
                                             std::size_t selectedTargetIndex,
                                             std::size_t launchTubeIndex) {
    std::vector<Sample> samples;
    if (durationSeconds <= 0 || stepSeconds <= 0) {
        return samples;
    }

    std::mt19937 generator(seed == 0 ? kStableScenarioSeed : seed);
    std::uniform_real_distribution<double> ownSpeedKnotsDist(10.0, 16.0);
    std::uniform_real_distribution<double> ownHeadingDist(20.0, 55.0);
    std::uniform_real_distribution<double> targetSpeedKnotsDist(8.0, 22.0);
    std::uniform_real_distribution<double> targetHeadingDist(0.0, 359.0);
    std::uniform_real_distribution<double> targetRangeDist(2200.0, 5600.0);
    std::uniform_real_distribution<double> targetBearingDist(5.0, 355.0);

    const double ownSpeedKnots = ownSpeedKnotsDist(generator);
    const double ownHeadingDeg = ownHeadingDist(generator);
    double ownX = 0.0;
    double ownY = 0.0;

    const std::size_t safeTargetCount = targetCount == 0 ? 1 : targetCount;
    std::vector<TargetState> targets;
    std::vector<double> targetBaseHeadings;
    targets.reserve(safeTargetCount);
    targetBaseHeadings.reserve(safeTargetCount);
    for (std::size_t i = 0; i < safeTargetCount; ++i) {
        const double bearingDeg = targetBearingDist(generator);
        const double rangeMeters = targetRangeDist(generator);
        const double bearingRad = toRadians(bearingDeg);
        const double baseHeadingDeg = targetHeadingDist(generator);
        targets.push_back(TargetState{
            rangeMeters * std::sin(bearingRad),
            rangeMeters * std::cos(bearingRad),
            targetSpeedKnotsDist(generator),
            baseHeadingDeg
        });
        targetBaseHeadings.push_back(baseHeadingDeg);
    }

    const int torpedoLaunchTimeSec = 22;
    const double torpedoSpeedKnots = 42.0;
    const int safeTubeIndex = static_cast<int>(launchTubeIndex % static_cast<std::size_t>(kTubeCount));
    const bool launchFromForwardTube = isForwardTube(safeTubeIndex);
    const double kForwardAftOffsetMeters = 12.0;
    const double kPortStarboardOffsetMeters = 6.0;

    double longitudinalOffsetMeters = launchFromForwardTube ? kForwardAftOffsetMeters : -kForwardAftOffsetMeters;
    double lateralOffsetMeters = 0.0;
    if (safeTubeIndex == 0) {
        lateralOffsetMeters = -kPortStarboardOffsetMeters;
    } else if (safeTubeIndex == 1) {
        lateralOffsetMeters = kPortStarboardOffsetMeters;
    }

    bool torpedoLaunched = false;
    bool torpedoHitTarget = false;
    double torpedoX = 0.0;
    double torpedoY = 0.0;
    double torpedoHeadingDeg = 0.0;

    const auto startTime = std::chrono::system_clock::now();
    samples.reserve(static_cast<size_t>(durationSeconds / stepSeconds) + 1U);

    for (int elapsed = 0; elapsed <= durationSeconds; elapsed += stepSeconds) {
        const auto currentTime = startTime + std::chrono::seconds(elapsed);
        const std::string timestamp = formatTimestamp(currentTime);

        const double ownStepMeters = ownSpeedKnots * kKnotsToMetersPerSecond * static_cast<double>(stepSeconds);
        const double ownHeadingRad = toRadians(ownHeadingDeg);
        ownX += ownStepMeters * std::sin(ownHeadingRad);
        ownY += ownStepMeters * std::cos(ownHeadingRad);

        for (std::size_t i = 0; i < targets.size(); ++i) {
            TargetState& target = targets[i];
            const double headingAmplitudeDeg = 1.2 + (0.5 * static_cast<double>(i % 3));
            const double headingPhase = 0.7 * static_cast<double>(i + 1);
            const double headingPeriod = 55.0 + (8.0 * static_cast<double>(i));
            double headingDeg = targetBaseHeadings[i] +
                                (headingAmplitudeDeg * std::sin((static_cast<double>(elapsed) / headingPeriod) + headingPhase));
            headingDeg = std::fmod(headingDeg, 360.0);
            if (headingDeg < 0.0) {
                headingDeg += 360.0;
            }
            target.headingDeg = headingDeg;

            const double targetStepMeters = target.speedKnots * kKnotsToMetersPerSecond * static_cast<double>(stepSeconds);
            const double targetHeadingRad = toRadians(target.headingDeg);
            target.x += targetStepMeters * std::sin(targetHeadingRad);
            target.y += targetStepMeters * std::cos(targetHeadingRad);
        }

        const std::size_t selectedIndex = std::min(selectedTargetIndex, targets.size() - 1);
        const TargetState& selectedTarget = targets[selectedIndex];

        const double ownForwardX = std::sin(ownHeadingRad);
        const double ownForwardY = std::cos(ownHeadingRad);
        const double ownStarboardX = std::cos(ownHeadingRad);
        const double ownStarboardY = -std::sin(ownHeadingRad);

        const double targetRelX = selectedTarget.x - ownX;
        const double targetRelY = selectedTarget.y - ownY;
        const double forwardProjection = (targetRelX * ownForwardX) + (targetRelY * ownForwardY);
        const bool targetInForwardSector = forwardProjection >= 0.0;
        const bool tubeCanEngageTarget = launchFromForwardTube ? targetInForwardSector : !targetInForwardSector;

        double torpedoSpeedOut = 0.0;
        if (!torpedoLaunched && elapsed >= torpedoLaunchTimeSec && tubeCanEngageTarget) {
            torpedoLaunched = true;
            // Launch from forward or aft section, plus port/starboard tube spacing.
            torpedoX = ownX + (longitudinalOffsetMeters * ownForwardX) + (lateralOffsetMeters * ownStarboardX);
            torpedoY = ownY + (longitudinalOffsetMeters * ownForwardY) + (lateralOffsetMeters * ownStarboardY);
        }

        if (torpedoLaunched && !torpedoHitTarget) {
            const double dx = selectedTarget.x - torpedoX;
            const double dy = selectedTarget.y - torpedoY;
            const double targetRange = std::hypot(dx, dy);

            if (targetRange <= 35.0) {
                torpedoHitTarget = true;
                torpedoX = selectedTarget.x;
                torpedoY = selectedTarget.y;
            }

            torpedoHeadingDeg = toDegrees(std::atan2(dx, dy));

            const double torpedoStepMeters = torpedoSpeedKnots * kKnotsToMetersPerSecond * static_cast<double>(stepSeconds);
            const double torpedoHeadingRad = toRadians(torpedoHeadingDeg);
            if (!torpedoHitTarget) {
                torpedoX += torpedoStepMeters * std::sin(torpedoHeadingRad);
                torpedoY += torpedoStepMeters * std::cos(torpedoHeadingRad);
            }
            torpedoSpeedOut = torpedoSpeedKnots;
        }

        samples.push_back(Sample{
            timestamp,
            ownSpeedKnots,
            ownHeadingDeg,
            ownX,
            ownY,
            selectedTarget.speedKnots,
            selectedTarget.headingDeg,
            torpedoSpeedOut,
            torpedoHeadingDeg,
            selectedTarget.x,
            selectedTarget.y,
            torpedoX,
            torpedoY,
            targets,
            selectedIndex
        });
    }

    return samples;
}

bool generateSimulatorCsv(const std::string& filePath,
                          int durationSeconds,
                          int stepSeconds,
                          std::size_t targetCount,
                          std::size_t selectedTargetIndex,
                          std::size_t launchTubeIndex) {
    if (filePath.empty()) {
        return false;
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    const std::vector<Sample> samples = generateSimulatorSamples(durationSeconds,
                                                                 stepSeconds,
                                                                 0,
                                                                 targetCount,
                                                                 selectedTargetIndex,
                                                                 launchTubeIndex);
    if (samples.empty()) {
        return false;
    }

    file << "timestamp,own_speed_knots,own_heading_deg,own_x,own_y,target_speed_knots,target_heading_deg,target_x,target_y,torpedo_speed_knots,torpedo_heading_deg,torpedo_x,torpedo_y\n";
    for (const Sample& sample : samples) {
        file << sample.timestamp << ","
             << std::fixed << std::setprecision(3)
             << sample.ownSpeedKnots << ","
             << sample.ownHeadingDeg << ","
             << sample.ownX << ","
             << sample.ownY << ","
             << sample.targetSpeedKnots << ","
             << sample.targetHeadingDeg << ","
             << sample.targetX << ","
             << sample.targetY << ","
             << sample.torpedoSpeedKnots << ","
             << sample.torpedoHeadingDeg << ","
             << sample.torpedoX << ","
             << sample.torpedoY << "\n";
    }

    return true;
}

std::vector<Sample> generateScenarioSamples(int durationSeconds,
                                            int stepSeconds,
                                            std::uint32_t seed,
                                            std::size_t targetCount,
                                            std::size_t selectedTargetIndex,
                                            std::size_t launchTubeIndex) {
    return generateSimulatorSamples(durationSeconds,
                                    stepSeconds,
                                    seed,
                                    targetCount,
                                    selectedTargetIndex,
                                    launchTubeIndex);
}

bool generateScenarioCsv(const std::string& filePath,
                         int durationSeconds,
                         int stepSeconds,
                         std::size_t targetCount,
                         std::size_t selectedTargetIndex,
                         std::size_t launchTubeIndex) {
    return generateSimulatorCsv(filePath,
                                durationSeconds,
                                stepSeconds,
                                targetCount,
                                selectedTargetIndex,
                                launchTubeIndex);
}

} // namespace Simulator

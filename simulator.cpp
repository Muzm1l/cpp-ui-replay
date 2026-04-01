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

std::vector<Sample> generateSimulatorSamples(int durationSeconds, int stepSeconds, std::uint32_t seed) {
    std::vector<Sample> samples;
    if (durationSeconds <= 0 || stepSeconds <= 0) {
        return samples;
    }

    std::mt19937 generator(seed == 0 ? std::random_device{}() : seed);
    std::uniform_real_distribution<double> targetSpeedKnotsDist(8.0, 20.0);
    std::uniform_real_distribution<double> targetHeadingDist(0.0, 359.0);
    std::uniform_real_distribution<double> targetXDist(2500.0, 5000.0);
    std::uniform_real_distribution<double> targetYDist(1000.0, 3000.0);

    const double ownSpeedKnots = 0.0;
    const double ownHeadingDeg = 0.0;

    double targetSpeedKnots = targetSpeedKnotsDist(generator);
    double targetHeadingDeg = targetHeadingDist(generator);
    double targetX = targetXDist(generator);
    double targetY = targetYDist(generator);

    const int torpedoLaunchTimeSec = 10;
    const double torpedoSpeedKnots = 42.0;
    bool torpedoLaunched = false;
    double torpedoX = 0.0;
    double torpedoY = 0.0;
    double torpedoHeadingDeg = 0.0;

    const auto startTime = std::chrono::system_clock::now();
    samples.reserve(static_cast<size_t>(durationSeconds / stepSeconds) + 1U);

    for (int elapsed = 0; elapsed <= durationSeconds; elapsed += stepSeconds) {
        const auto currentTime = startTime + std::chrono::seconds(elapsed);
        const std::string timestamp = formatTimestamp(currentTime);

        const double targetStepMeters = targetSpeedKnots * kKnotsToMetersPerSecond * static_cast<double>(stepSeconds);
        const double targetHeadingRad = toRadians(targetHeadingDeg);
        targetX += targetStepMeters * std::sin(targetHeadingRad);
        targetY += targetStepMeters * std::cos(targetHeadingRad);

        double torpedoSpeedOut = 0.0;
        if (elapsed >= torpedoLaunchTimeSec) {
            torpedoLaunched = true;
        }

        if (torpedoLaunched) {
            const double dx = targetX - torpedoX;
            const double dy = targetY - torpedoY;
            torpedoHeadingDeg = toDegrees(std::atan2(dx, dy));

            const double torpedoStepMeters = torpedoSpeedKnots * kKnotsToMetersPerSecond * static_cast<double>(stepSeconds);
            const double torpedoHeadingRad = toRadians(torpedoHeadingDeg);
            torpedoX += torpedoStepMeters * std::sin(torpedoHeadingRad);
            torpedoY += torpedoStepMeters * std::cos(torpedoHeadingRad);
            torpedoSpeedOut = torpedoSpeedKnots;
        }

        samples.push_back(Sample{
            timestamp,
            ownSpeedKnots,
            ownHeadingDeg,
            targetSpeedKnots,
            targetHeadingDeg,
            torpedoSpeedOut,
            torpedoHeadingDeg,
            targetX,
            targetY,
            torpedoX,
            torpedoY
        });
    }

    return samples;
}

bool generateSimulatorCsv(const std::string& filePath, int durationSeconds, int stepSeconds) {
    if (filePath.empty()) {
        return false;
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    const std::vector<Sample> samples = generateSimulatorSamples(durationSeconds, stepSeconds);
    if (samples.empty()) {
        return false;
    }

    file << "timestamp,own_speed_knots,own_heading_deg,target_speed_knots,target_heading_deg,torpedo_speed_knots,torpedo_heading_deg\n";
    for (const Sample& sample : samples) {
        file << sample.timestamp << ","
             << std::fixed << std::setprecision(3)
             << sample.ownSpeedKnots << ","
             << sample.ownHeadingDeg << ","
             << sample.targetSpeedKnots << ","
             << sample.targetHeadingDeg << ","
             << sample.torpedoSpeedKnots << ","
             << sample.torpedoHeadingDeg << "\n";
    }

    return true;
}

} // namespace Simulator

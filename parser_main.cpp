#include "parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>

// Forward declaration from parser.cpp
bool convertBinaryToText(const std::string& inputFile, const std::string& outputFile);

static bool isLikelyTextHexDat(const std::string& inputFile) {
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::string firstLine;
    if (!std::getline(file, firstLine)) {
        return false;
    }

    size_t commaPos = firstLine.find(',');
    if (commaPos == std::string::npos || commaPos + 1 >= firstLine.size()) {
        return false;
    }

    std::string hexPart = firstLine.substr(commaPos + 1);
    hexPart.erase(std::remove_if(hexPart.begin(), hexPart.end(),
                                 [](unsigned char c) { return std::isspace(c) != 0; }),
                  hexPart.end());

    if (hexPart.size() < 4) {
        return false;
    }

    return std::all_of(hexPart.begin(), hexPart.end(),
                       [](unsigned char c) { return std::isxdigit(c) != 0; });
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.dat|.txt> [output_directory]" << std::endl;
        std::cerr << "  If input is .dat (binary), it will be converted to text first" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputDir = argc > 2 ? argv[2] : "output";
    std::filesystem::create_directories(outputDir);

    // .dat can be either binary or text-hex format; convert only when binary
    std::string parseFile = inputFile;
    if (inputFile.size() > 4 && inputFile.substr(inputFile.size() - 4) == ".dat") {
        if (isLikelyTextHexDat(inputFile)) {
            std::cout << ".dat text-hex format detected. Parsing directly..." << std::endl;
        } else {
            std::string convertedFile = outputDir + "/converted.txt";
            std::cout << "Binary .dat detected. Converting..." << std::endl;
            if (!convertBinaryToText(inputFile, convertedFile)) {
                std::cerr << "Error: Failed to convert binary file" << std::endl;
                return 1;
            }
            parseFile = convertedFile;
        }
    }

    DatParser parser;
    if (!parser.parseFile(parseFile)) {
        std::cerr << "Error: Failed to parse input file" << std::endl;
        return 1;
    }

    parser.printStatistics();
    parser.writeOwnCsv(outputDir + "/Own.csv");
    parser.writeTargetCsv(outputDir + "/Target.csv");
    parser.writeTorpedoCsv(outputDir + "/Torpedo.csv");
    parser.writeTubeCsv(outputDir + "/Tube.csv");
    parser.writeEventCsv(outputDir + "/Event.csv");
    parser.writeSonarCsv(outputDir + "/Sonar.csv");
    parser.writeSummaryJson(outputDir + "/summary.json");

    std::cout << "\nParsing completed successfully!" << std::endl;
    std::cout << "Output files written to: " << outputDir << std::endl;

    return 0;
}

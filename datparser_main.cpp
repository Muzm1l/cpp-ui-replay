#include "datparser.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.dat> [output_dir]" << std::endl;
        std::cerr << "\n  input.dat:   Path to the .dat file to parse" << std::endl;
        std::cerr << "  output_dir:  Directory for output CSV files (default: current dir)" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputDir = argc > 2 ? argv[2] : ".";
    
    // Ensure output directory exists
    std::filesystem::create_directories(outputDir);
    
    std::cout << "DatParser v1.0" << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output directory: " << outputDir << std::endl << std::endl;
    
    DatParser parser;
    
    // Parse the .dat file
    std::cout << "Parsing .dat file..." << std::endl;
    if (!parser.parseFile(inputFile)) {
        std::cerr << "Error: Failed to parse input file" << std::endl;
        return 1;
    }
    
    // Print statistics
    parser.printStatistics();
    
    // Write output CSV files
    std::cout << "\nWriting output CSV files..." << std::endl;
    
    std::string ownCsv = outputDir + "/Own.csv";
    std::string targetCsv = outputDir + "/Target1.csv";
    std::string torpedoCsv = outputDir + "/Torpedo.csv";
    std::string tubeCsv = outputDir + "/Tube1.csv";
    std::string eventCsv = outputDir + "/Event.txt";
    std::string sonarCsv = outputDir + "/Sonar.txt";
    std::string summaryJson = outputDir + "/summary.json";
    
    parser.writeOwnCsv(ownCsv);
    parser.writeTargetCsv(targetCsv);
    parser.writeTorpedoCsv(torpedoCsv);
    parser.writeTubeCsv(tubeCsv);
    parser.writeEventCsv(eventCsv);
    parser.writeSonarCsv(sonarCsv);
    parser.writeSummaryJson(summaryJson);
    
    std::cout << "\nParsing complete!" << std::endl;
    
    return 0;
}

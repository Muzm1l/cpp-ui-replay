#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstdint>

struct Sample {
    std::string timestamp;
    std::string payload_hex;
    std::vector<uint8_t> payload;
};

uint8_t hexCharToUint8(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

float bytesToFloatLE(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + 3 >= data.size()) return 0.0f;
    uint32_t intVal = (uint32_t)data[offset] | 
                     ((uint32_t)data[offset+1] << 8) | 
                     ((uint32_t)data[offset+2] << 16) | 
                     ((uint32_t)data[offset+3] << 24);
    float floatVal;
    std::memcpy(&floatVal, &intVal, sizeof(float));
    return floatVal;
}

uint32_t bytesToUint32LE(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + 3 >= data.size()) return 0;
    return (uint32_t)data[offset] | 
           ((uint32_t)data[offset+1] << 8) | 
           ((uint32_t)data[offset+2] << 16) | 
           ((uint32_t)data[offset+3] << 24);
}

std::vector<Sample> readSamplePackets(const std::string& filepath, uint8_t packet_type, int count) {
    std::vector<Sample> samples;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << std::endl;
        return samples;
    }
    
    std::string line;
    char type_str[3];
    snprintf(type_str, sizeof(type_str), "%02x", packet_type);
    std::string type_search(type_str);
    
    while (std::getline(file, line) && samples.size() < count) {
        size_t comma_pos = line.find(',');
        if (comma_pos == std::string::npos) continue;
        
        std::string hex_data = line.substr(comma_pos + 1);
        
        // Check if type matches (at offset 40-41 in hex)
        if (hex_data.length() >= 42) {
            std::string hex_type = hex_data.substr(40, 2);
            if (hex_type != type_search) continue;
        }
        
        std::string timestamp = line.substr(0, comma_pos);
        std::string payload_hex = hex_data.substr(56);
        
        std::vector<uint8_t> payload;
        for (size_t i = 0; i < payload_hex.length(); i += 2) {
            if (i + 1 < payload_hex.length()) {
                uint8_t byte = (hexCharToUint8(payload_hex[i]) << 4) | 
                              hexCharToUint8(payload_hex[i+1]);
                payload.push_back(byte);
            }
        }
        
        Sample sample;
        sample.timestamp = timestamp;
        sample.payload_hex = payload_hex;
        sample.payload = payload;
        samples.push_back(sample);
    }
    
    file.close();
    return samples;
}

void analyzeType07(const std::vector<Sample>& samples) {
    std::cout << "=== TYPE 0x07: Own Trajectory ===" << std::endl;
    std::cout << "Expected fields: time, course, speed, depth, x, y (6 fields)\n" << std::endl;
    
    for (size_t idx = 0; idx < samples.size(); idx++) {
        const auto& sample = samples[idx];
        std::cout << "Sample " << (idx + 1) << ":" << std::endl;
        std::cout << "  Timestamp: " << sample.timestamp << std::endl;
        std::cout << "  Payload size: " << sample.payload.size() << " bytes" << std::endl;
        
        std::cout << "\n  Layout A (skip 12, then 6 f32s):" << std::endl;
        for (int i = 0; i < 6; i++) {
            size_t offset = 12 + i * 4;
            if (offset + 4 <= sample.payload.size()) {
                float val = bytesToFloatLE(sample.payload, offset);
                std::cout << "    Field " << i << ": " << std::fixed << std::setprecision(4) 
                         << std::setw(12) << val << std::endl;
            }
        }
        
        std::cout << "  Layout B (skip 16, then 6 f32s):" << std::endl;
        for (int i = 0; i < 6; i++) {
            size_t offset = 16 + i * 4;
            if (offset + 4 <= sample.payload.size()) {
                float val = bytesToFloatLE(sample.payload, offset);
                std::cout << "    Field " << i << ": " << std::fixed << std::setprecision(4) 
                         << std::setw(12) << val << std::endl;
            }
        }
        
        std::cout << "  Layout C (skip 8, then 4 f32s):" << std::endl;
        for (int i = 0; i < 4; i++) {
            size_t offset = 8 + i * 4;
            if (offset + 4 <= sample.payload.size()) {
                float val = bytesToFloatLE(sample.payload, offset);
                std::cout << "    Field " << i << ": " << std::fixed << std::setprecision(4) 
                         << std::setw(12) << val << std::endl;
            }
        }
        
        std::cout << std::endl;
    }
}

void analyzeType0d(const std::vector<Sample>& samples) {
    std::cout << "=== TYPE 0x0d: Target/Torpedo ===" << std::endl;
    std::cout << "Expected: 16 fields (target) or 10 fields (torpedo)\n" << std::endl;
    
    for (size_t idx = 0; idx < samples.size(); idx++) {
        const auto& sample = samples[idx];
        std::cout << "Sample " << (idx + 1) << ":" << std::endl;
        std::cout << "  Timestamp: " << sample.timestamp << std::endl;
        std::cout << "  Payload size: " << sample.payload.size() << " bytes" << std::endl;
        
        std::cout << "  Trying 16 f32 fields starting at offset 0:" << std::endl;
        for (int i = 0; i < 16 && i * 4 + 4 <= (int)sample.payload.size(); i++) {
            size_t offset = i * 4;
            float val = bytesToFloatLE(sample.payload, offset);
            std::cout << "    Field " << std::setw(2) << i << ": " << std::fixed 
                     << std::setprecision(4) << std::setw(12) << val << std::endl;
        }
        
        std::cout << std::endl;
    }
}

void analyzeEvent(const std::vector<Sample>& samples) {
    std::cout << "=== Event Packets ===" << std::endl;
    std::cout << "Expected: time, sender, type, name\n" << std::endl;
    
    for (size_t idx = 0; idx < samples.size(); idx++) {
        const auto& sample = samples[idx];
        std::cout << "Sample " << (idx + 1) << ":" << std::endl;
        std::cout << "  Timestamp: " << sample.timestamp << std::endl;
        std::cout << "  Payload size: " << sample.payload.size() << " bytes" << std::endl;
        
        std::cout << "  As u32/f32 values:" << std::endl;
        std::cout << "  Offset | u32 value  | f32 value" << std::endl;
        std::cout << "  -------|------------|----------" << std::endl;
        
        for (int i = 0; i < 8 && i * 4 + 4 <= (int)sample.payload.size(); i++) {
            size_t offset = i * 4;
            uint32_t u32_val = bytesToUint32LE(sample.payload, offset);
            float f32_val = bytesToFloatLE(sample.payload, offset);
            
            std::cout << "    " << std::setw(2) << offset << "  | " 
                     << std::setw(10) << u32_val << " | " 
                     << std::fixed << std::setprecision(4) << std::setw(10) << f32_val << std::endl;
        }
        
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <type> [count]" << std::endl;
        std::cerr << "  type:  packet type in hex (07, 0d, 0e, etc.)" << std::endl;
        std::cerr << "  count: number of samples to analyze (default: 5)" << std::endl;
        return 1;
    }
    
    uint8_t ptype = (uint8_t)std::stoi(argv[1], nullptr, 16);
    int count = argc > 2 ? std::stoi(argv[2]) : 5;
    
    std::string filepath = "/home/siddartha/Music/FiringAnalysis/saveData.dat";
    
    std::cout << "Analyzing type 0x" << std::hex << (int)ptype << std::dec << " packets..." << std::endl;
    
    auto samples = readSamplePackets(filepath, ptype, count);
    
    if (samples.empty()) {
        std::cerr << "No packets of type 0x" << std::hex << (int)ptype << std::dec << " found" << std::endl;
        return 1;
    }
    
    if (ptype == 0x07) {
        analyzeType07(samples);
    } else if (ptype == 0x0d) {
        analyzeType0d(samples);
    } else if (ptype == 0x0e || ptype == 0x0f || ptype == 0x10 || ptype == 0x11) {
        analyzeEvent(samples);
    } else {
        std::cout << "Analysis for type 0x" << std::hex << (int)ptype << std::dec 
                 << " not implemented yet" << std::endl;
    }
    
    return 0;
}

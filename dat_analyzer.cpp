#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring>
#include <iomanip>
#include <cstdint>

struct PacketInfo {
    uint8_t type;
    uint8_t source;
    size_t payload_size;
};

uint8_t hexCharToUint8(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

void hexDump(const std::vector<uint8_t>& data, const std::string& label, int max_lines = 3) {
    if (!label.empty()) {
        std::cout << "\n" << label << std::endl;
    }
    
    for (size_t i = 0; i < data.size(); i += 16) {
        std::cout << std::hex << std::setfill('0') << std::setw(4) << i << ": ";
        
        // Hex bytes
        for (size_t j = 0; j < 16 && i + j < data.size(); j++) {
            std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)data[i+j] << " ";
        }
        
        // ASCII
        std::cout << "  ";
        for (size_t j = 0; j < 16 && i + j < data.size(); j++) {
            char c = data[i+j];
            std::cout << (c >= 32 && c < 127 ? c : '.');
        }
        std::cout << std::endl;
        
        if (i / 16 >= (size_t)(max_lines - 1)) {
            std::cout << "  ..." << std::endl;
            break;
        }
    }
    std::cout << std::dec;
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

bool analyzePackets(const std::string& filepath, int num_packets) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filepath << std::endl;
        return false;
    }
    
    std::vector<PacketInfo> packets;
    std::map<uint8_t, int> typeCount;
    std::map<uint8_t, int> sourceCount;
    
    std::string line;
    int line_num = 0;
    
    while (std::getline(file, line) && line_num < num_packets) {
        line_num++;
        
        size_t comma_pos = line.find(',');
        if (comma_pos == std::string::npos) continue;
        
        std::string hex_data = line.substr(comma_pos + 1);
        
        if (hex_data.length() < 56) continue;
        
        // Extract source (first byte, chars 0-1)
        uint8_t source = (hexCharToUint8(hex_data[0]) << 4) | hexCharToUint8(hex_data[1]);
        
        // Extract type (byte 20, chars 40-41)
        uint8_t type = (hexCharToUint8(hex_data[40]) << 4) | hexCharToUint8(hex_data[41]);
        
        // Extract payload size
        std::string payload_hex = hex_data.substr(56);
        size_t payload_size = payload_hex.length() / 2;
        
        PacketInfo info;
        info.type = type;
        info.source = source;
        info.payload_size = payload_size;
        
        packets.push_back(info);
        typeCount[type]++;
        sourceCount[source]++;
    }
    
    file.close();
    
    std::cout << "Analyzed " << packets.size() << " packets" << std::endl;
    std::cout << "\nPacket types found:" << std::endl;
    
    for (const auto& pair : typeCount) {
        std::cout << "  Type 0x" << std::hex << std::setfill('0') << std::setw(2) 
                 << (int)pair.first << ": " << std::dec << pair.second << " packets" << std::endl;
    }
    
    std::cout << "\nPacket sources found:" << std::endl;
    for (const auto& pair : sourceCount) {
        std::cout << "  Source 0x" << std::hex << std::setfill('0') << std::setw(2) 
                 << (int)pair.first << ": " << std::dec << pair.second << " packets" << std::endl;
    }
    
    // Detailed analysis by type
    std::cout << "\n\n=== DETAILED PACKET ANALYSIS ===" << std::endl;
    
    // Type 0x07
    if (typeCount.count(0x07)) {
        std::cout << "\n--- TYPE 0x07 (Own Trajectory) ---" << std::endl;
        file.open(filepath);
        int count = 0;
        while (std::getline(file, line) && count < 2) {
            size_t comma_pos = line.find(',');
            if (comma_pos == std::string::npos) continue;
            
            std::string hex_data = line.substr(comma_pos + 1);
            if (hex_data.length() < 42) continue;
            
            if (hex_data.substr(40, 2) != "07") continue;
            
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
            
            std::cout << "\nPacket #" << (count + 1) << ": " << timestamp << std::endl;
            std::cout << "Payload size: " << payload.size() << " bytes" << std::endl;
            
            hexDump(payload, "Payload (hex):");
            
            std::cout << "\nAs little-endian u32/f32:" << std::endl;
            for (size_t i = 0; i < 8 && i * 4 + 4 <= payload.size(); i++) {
                uint32_t u32_val = (uint32_t)payload[i*4] | 
                                  ((uint32_t)payload[i*4+1] << 8) | 
                                  ((uint32_t)payload[i*4+2] << 16) | 
                                  ((uint32_t)payload[i*4+3] << 24);
                float f32_val = bytesToFloatLE(payload, i*4);
                
                std::cout << "  Offset " << std::setw(2) << (i*4) << ": u32=" 
                         << std::setw(10) << u32_val << "  f32=" 
                         << std::fixed << std::setprecision(2) << std::setw(10) << f32_val << std::endl;
            }
            
            count++;
        }
        file.close();
    }
    
    // Type 0x0d
    if (typeCount.count(0x0d)) {
        std::cout << "\n--- TYPE 0x0d (Target/Torpedo) ---" << std::endl;
        file.open(filepath);
        int count = 0;
        while (std::getline(file, line) && count < 1) {
            size_t comma_pos = line.find(',');
            if (comma_pos == std::string::npos) continue;
            
            std::string hex_data = line.substr(comma_pos + 1);
            if (hex_data.length() < 42) continue;
            
            if (hex_data.substr(40, 2) != "0d") continue;
            
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
            
            std::cout << "\nPacket #" << (count + 1) << ": " << timestamp << std::endl;
            std::cout << "Payload size: " << payload.size() << " bytes" << std::endl;
            
            hexDump(payload, "Payload (hex):", 5);
            
            std::cout << "\nAs little-endian u32/f32 (first 16 fields):" << std::endl;
            for (size_t i = 0; i < 16 && i * 4 + 4 <= payload.size(); i++) {
                uint32_t u32_val = (uint32_t)payload[i*4] | 
                                  ((uint32_t)payload[i*4+1] << 8) | 
                                  ((uint32_t)payload[i*4+2] << 16) | 
                                  ((uint32_t)payload[i*4+3] << 24);
                float f32_val = bytesToFloatLE(payload, i*4);
                
                std::cout << "  Offset " << std::setw(2) << (i*4) << ": u32=" 
                         << std::setw(10) << u32_val << "  f32=" 
                         << std::fixed << std::setprecision(2) << std::setw(10) << f32_val << std::endl;
            }
            
            count++;
        }
        file.close();
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    std::string filepath = "/home/siddartha/Music/FiringAnalysis/saveData.dat";
    int num_packets = 30;
    
    if (argc > 1) {
        filepath = argv[1];
    }
    if (argc > 2) {
        num_packets = std::stoi(argv[2]);
    }
    
    std::cout << "DatAnalyzer - Binary .dat file analyzer" << std::endl;
    std::cout << "File: " << filepath << std::endl;
    std::cout << "Analyzing first " << num_packets << " packets..." << std::endl;
    
    if (!analyzePackets(filepath, num_packets)) {
        return 1;
    }
    
    return 0;
}

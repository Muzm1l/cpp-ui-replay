#!/usr/bin/env python3
"""
Utilities to analyze and decode .dat file packets
"""
import struct
import sys
import re
from collections import defaultdict

def hex_dump(data, label=""):
    """Print a formatted hex dump of binary data"""
    if label:
        print(f"\n{label}")
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ' '.join(f'{b:02x}' for b in chunk)
        ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
        print(f"{i:04x}: {hex_str:<48} {ascii_str}")

def parse_dat_file(filepath, num_packets=10):
    """Parse first N packets from .dat file"""
    packets = []
    
    with open(filepath, 'r') as f:
        for line_num, line in enumerate(f):
            if line_num >= num_packets:
                break
            
            line = line.strip()
            if not line:
                continue
                
            # Parse: timestamp,hexdata
            parts = line.split(',')
            if len(parts) < 2:
                continue
            
            timestamp = parts[0]
            hex_data = parts[1]
            
            #Extract type from fixed offset (byte 20)
            if len(hex_data) >= 42:
                type_str = hex_data[40:42]
                packet_type = int(type_str, 16)
            else:
                packet_type = 0
            
            payload_hex = hex_data[56:]  # Everything after 28-byte header
            payload_bytes = bytes.fromhex(payload_hex)
            
            packets.append({
                'timestamp': timestamp,
                'hex_header': hex_data[:56],
                'hex_payload': payload_hex,
                'type': packet_type,
                'payload': payload_bytes,
                'line': line_num
            })
    
    return packets

def analyze_packets(filepath, num_packets=30):
    """Analyze multiple packets to find patterns"""
    packets = parse_dat_file(filepath, num_packets)
    
    # Group by type
    by_type = defaultdict(list)
    for pkt in packets:
        by_type[pkt['type']].append(pkt)
    
    print(f"Analyzed {len(packets)} packets")
    print(f"\nPacket types found:")
    for ptype in sorted(by_type.keys()):
        print(f"  Type 0x{ptype:02x}: {len(by_type[ptype])} packets")
    
    # Analyze type 0x07 (Own trajectory)
    if 0x07 in by_type:
        print(f"\n\n=== TYPE 0x07 (Own Trajectory) ===")
        for pkt in by_type[0x07][:3]:  # First 3 examples
            print(f"\nPacket #{pkt['line']}: {pkt['timestamp']}")
            print(f"Payload size: {len(pkt['payload'])} bytes")
            
            hex_dump(pkt['payload'][:60], "Payload (hex):")
            
            # Try various interpretations
            print("\nAs little-endian u32/f32:")
            for i in range(0, min(len(pkt['payload']), 32), 4):
                if i + 4 <= len(pkt['payload']):
                    u32 = struct.unpack('<I', pkt['payload'][i:i+4])[0]
                    f32 = struct.unpack('<f', pkt['payload'][i:i+4])[0]
                    print(f"  Offset {i:2d}: u32={u32:10d}  f32={f32:10.2f}")
    
    # Analyze type 0x0d (Target/Torpedo)  
    if 0x0d in by_type:
        print(f"\n\n=== TYPE 0x0d (Target/Torpedo) ===")
        for pkt in by_type[0x0d][:1]:  # First example
            print(f"\nPacket #{pkt['line']}: {pkt['timestamp']}")
            print(f"Payload size: {len(pkt['payload'])} bytes")
            
            hex_dump(pkt['payload'][:80], "Payload (hex):")
            
            print("\nAs little-endian u32/f32:")
            for i in range(0, min(len(pkt['payload']), 64), 4):
                if i + 4 <= len(pkt['payload']):
                    u32 = struct.unpack('<I', pkt['payload'][i:i+4])[0]
                    f32 = struct.unpack('<f', pkt['payload'][i:i+4])[0]
                    print(f"  Offset {i:2d}: u32={u32:10d}  f32={f32:10.2f}")

if __name__ == '__main__':
    analyze_packets('/home/siddartha/Music/FiringAnalysis/saveData.dat', 30)

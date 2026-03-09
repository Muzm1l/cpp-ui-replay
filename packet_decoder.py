#!/usr/bin/env python3
"""
Binary packet decoder helper - helps figure out correct field offsets
Usage: python3 packet_decoder.py <packet_type>
"""
import struct
import sys
import re

def read_sample_packets(filepath, packet_type, count=5):
    """Extract sample packets of a given type"""
    samples = []
    type_hex = f"{packet_type:02x}"
    
    with open(filepath, 'r') as f:
        for line in f:
            if len(samples) >= count:
                break
            
            parts = line.strip().split(',')
            if len(parts) < 2:
                continue
            
            hex_data = parts[1]
            if len(hex_data) >= 42 and hex_data[40:42].lower() == type_hex:
                payload_hex = hex_data[56:]
                payload = bytes.fromhex(payload_hex)
                samples.append({
                    'timestamp': parts[0],
                    'payload_hex': payload_hex,
                    'payload': payload
                })
    
    return samples

def analyze_type_07(samples):
    """Analyze Own trajectory packets (type 0x07)"""
    print("=== TYPE 0x07: Own Trajectory ===")
    print("Expected fields: time, course, speed, depth, x, y (6 fields)\n")
    
    for idx, sample in enumerate(samples):
        print(f"Sample {idx + 1}:")
        print(f"  Timestamp: {sample['timestamp']}")
        print(f"  Payload size: {len(sample['payload'])} bytes")
        
        # Try different interpretations
        print("\n  Trying various field layouts...")
        
        # Layout 1: Skip header, then 6 floats
        print("  Layout A (skip 12, then 6 f32s):")
        for i in range(6):
            offset = 12 + i * 4
            if offset + 4 <= len(sample['payload']):
                val = struct.unpack('<f', sample['payload'][offset:offset+4])[0]
                print(f"    Field {i}: {val:12.4f}")
        
        # Layout 2: u32 header, then 6 floats
        print("  Layout B (4 u32 header, then 6 f32s):")
        for i in range(6):
            offset = 16 + i * 4
            if offset + 4 <= len(sample['payload']):
                val = struct.unpack('<f', sample['payload'][offset:offset+4])[0]
                print(f"    Field {i}: {val:12.4f}")
        
        # Layout 3: Try as doubles
        print("  Layout C (skip 8, then floats):")
        for i in range(4):
            offset = 8 + i * 4
            if offset + 4 <= len(sample['payload']):
                val = struct.unpack('<f', sample['payload'][offset:offset+4])[0]
                print(f"    Field {i}: {val:12.4f}")
        
        print()

def analyze_type_0d(samples):
    """Analyze Target/Torpedo packets (type 0x0d)"""
    print("=== TYPE 0x0d: Target/Torpedo ===")
    print("Expected: 16 fields (target) or 10 fields (torpedo)\n")
    
    for idx, sample in enumerate(samples):
        print(f"Sample {idx + 1}:")
        print(f"  Timestamp: {sample['timestamp']}")
        print(f"  Payload size: {len(sample['payload'])} bytes")
        
        print("  Trying 16 f32 fields starting at offset 0:")
        for i in range(min(16, len(sample['payload']) // 4)):
            offset = i * 4
            if offset + 4 <= len(sample['payload']):
                val = struct.unpack('<f', sample['payload'][offset:offset+4])[0]
                print(f"    Field {i:2d}: {val:12.4f}")
        
        print()

def analyze_event(samples):
    """Analyze Event packets (types 0x0e, 0x0f, 0x10, 0x11)"""
    print("=== Event Packets ===")
    print("Expected: time, sender, type, name\n")
    
    for idx, sample in enumerate(samples):
        print(f"Sample {idx + 1}:")
        print(f"  Timestamp: {sample['timestamp']}")
        print(f"  Payload size: {len(sample['payload'])} bytes")
        
        print("  As u32/f32 values:")
        for i in range(min(8, len(sample['payload']) // 4)):
            offset = i * 4
            if offset + 4 <= len(sample['payload']):
                u32_val = struct.unpack('<I', sample['payload'][offset:offset+4])[0]
                f32_val = struct.unpack('<f', sample['payload'][offset:offset+4])[0]
                print(f"    Offset {offset:2d}: u32={u32_val:10d}  f32={f32_val:10.4f}")
        
        print()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 packet_decoder.py <type> [count]")
        print("  type: packet type in hex (07, 0d, 0e, etc.)")
        print("  count: number of samples to analyze (default: 5)")
        sys.exit(1)
    
    ptype = int(sys.argv[1], 16)
    count = int(sys.argv[2]) if len(sys.argv) > 2 else 5
    
    filepath = '/home/siddartha/Music/FiringAnalysis/saveData.dat'
    samples = read_sample_packets(filepath, ptype, count)
    
    if not samples:
        print(f"No packets of type 0x{ptype:02x} found")
        sys.exit(1)
    
    if ptype == 0x07:
        analyze_type_07(samples)
    elif ptype == 0x0d:
        analyze_type_0d(samples)
    elif ptype in [0x0e, 0x0f, 0x10, 0x11]:
        analyze_event(samples)
    else:
        print(f"Type 0x{ptype:02x} analysis not implemented yet")

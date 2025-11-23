#!/usr/bin/env python3
"""
PID Data Logger - Save telemetry to CSV for offline analysis
"""

import socket
import struct
import csv
import time
import argparse
from datetime import datetime

TELEMETRY_PORT = 10001
MULTICAST_IP = "239.42.42.42"

def parse_telemetry(data):
    """Parse telemetry packet from robot"""
    if len(data) < 32:
        return None
    
    if data[0] != 0xFE or data[1] != 0xED:
        return None
    
    timestamp_ms = struct.unpack('>I', data[2:6])[0]
    
    wheel_data = []
    for i in range(4):
        idx = 6 + (i * 6)
        target = struct.unpack('>h', data[idx:idx+2])[0]
        actual = struct.unpack('>h', data[idx+2:idx+4])[0]
        output = struct.unpack('>h', data[idx+4:idx+6])[0]
        wheel_data.append({
            'target': target,
            'actual': actual,
            'output': output,
            'error': target - actual
        })
    
    return {
        'timestamp': timestamp_ms / 1000.0,
        'wheels': wheel_data
    }

def main():
    parser = argparse.ArgumentParser(description="Log PID telemetry to CSV")
    parser.add_argument('--output', '-o', type=str, 
                        default=f"pid_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv",
                        help='Output CSV filename')
    parser.add_argument('--duration', '-d', type=int, default=0,
                        help='Duration to log in seconds (0 = until Ctrl+C)')
    args = parser.parse_args()
    
    # Setup socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', TELEMETRY_PORT))
    
    mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    sock.settimeout(1.0)
    
    print(f"📝 PID Data Logger")
    print(f"   Output file: {args.output}")
    print(f"   Listening on {MULTICAST_IP}:{TELEMETRY_PORT}")
    print("\n   Press Ctrl+C to stop\n")
    
    # Open CSV file
    with open(args.output, 'w', newline='') as csvfile:
        fieldnames = ['timestamp', 
                      'w0_target', 'w0_actual', 'w0_output', 'w0_error',
                      'w1_target', 'w1_actual', 'w1_output', 'w1_error',
                      'w2_target', 'w2_actual', 'w2_output', 'w2_error',
                      'w3_target', 'w3_actual', 'w3_output', 'w3_error']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        
        start_time = time.time()
        packet_count = 0
        
        try:
            while True:
                if args.duration > 0 and (time.time() - start_time) > args.duration:
                    print(f"\n⏱️  Duration reached ({args.duration}s)")
                    break
                
                try:
                    data, _ = sock.recvfrom(1024)
                    telemetry = parse_telemetry(data)
                    
                    if telemetry:
                        row = {'timestamp': telemetry['timestamp']}
                        for i, wheel in enumerate(telemetry['wheels']):
                            row[f'w{i}_target'] = wheel['target']
                            row[f'w{i}_actual'] = wheel['actual']
                            row[f'w{i}_output'] = wheel['output']
                            row[f'w{i}_error'] = wheel['error']
                        
                        writer.writerow(row)
                        packet_count += 1
                        
                        if packet_count % 10 == 0:
                            print(f"Packets logged: {packet_count}", end='\r')
                
                except socket.timeout:
                    continue
        
        except KeyboardInterrupt:
            print(f"\n\n✅ Logged {packet_count} packets to {args.output}")
    
    print("\n📊 To plot this data:")
    print(f"   python plot_csv.py {args.output}")

if __name__ == "__main__":
    main()

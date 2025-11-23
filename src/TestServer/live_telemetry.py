#!/usr/bin/env python3
"""
Live telemetry viewer - see what values robot is sending in real-time
"""

import socket
import struct
import time

TELEMETRY_PORT = 10001
COMMAND_PORT = 10000
MULTICAST_IP = "239.42.42.42"
ROBOT_ID = 1

print("📡 Live Telemetry Viewer")
print("="*70)
print("This shows real-time data from the robot")
print("Press Ctrl+C to stop\n")

# Setup receive socket
recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
recv_sock.bind(('', TELEMETRY_PORT))
mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
recv_sock.settimeout(0.5)

# Setup send socket
send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def parse_telemetry(data):
    if len(data) < 32 or data[0] != 0xFE or data[1] != 0xED:
        return None
    
    timestamp_ms = struct.unpack('>I', data[2:6])[0]
    wheels = []
    for i in range(4):
        idx = 6 + (i * 6)
        target = struct.unpack('>h', data[idx:idx+2])[0]
        actual = struct.unpack('>h', data[idx+2:idx+4])[0]
        output = struct.unpack('>h', data[idx+4:idx+6])[0]
        wheels.append({'target': target, 'actual': actual, 'output': output})
    
    return {'timestamp': timestamp_ms / 1000.0, 'wheels': wheels}

def send_command(cmd):
    send_sock.sendto(cmd.encode(), (MULTICAST_IP, COMMAND_PORT))

try:
    print("Waiting for telemetry...\n")
    
    last_print = time.time()
    while True:
        try:
            data, _ = recv_sock.recvfrom(1024)
            telemetry = parse_telemetry(data)
            
            if telemetry and (time.time() - last_print) > 0.2:
                wheels = telemetry['wheels']
                print(f"t={telemetry['timestamp']:7.2f}s | ", end='')
                for i, w in enumerate(wheels):
                    print(f"W{i}: T={w['target']:5d} A={w['actual']:5d} O={w['output']:5d} | ", end='')
                print(end='\r')
                last_print = time.time()
        
        except socket.timeout:
            continue

except KeyboardInterrupt:
    print("\n\n" + "="*70)
    print("Stopped. Try these commands manually:")
    print(f'  echo "{ROBOT_ID} move 1.0 0 0" | nc -u {MULTICAST_IP} {COMMAND_PORT}')
    print(f'  echo "{ROBOT_ID} move 0 0 0" | nc -u {MULTICAST_IP} {COMMAND_PORT}')

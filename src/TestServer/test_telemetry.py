#!/usr/bin/env python3
"""
Quick test to see if telemetry is being received
"""

import socket
import struct
import time

TELEMETRY_PORT = 10001
MULTICAST_IP = "239.42.42.42"

print("🔍 Testing for telemetry data...")
print(f"   Listening on {MULTICAST_IP}:{TELEMETRY_PORT}")
print("   Waiting 10 seconds for data...\n")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', TELEMETRY_PORT))

mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
sock.settimeout(1.0)

packets_received = 0
start_time = time.time()

try:
    while (time.time() - start_time) < 10:
        try:
            data, addr = sock.recvfrom(1024)
            if len(data) >= 32 and data[0] == 0xFE and data[1] == 0xED:
                packets_received += 1
                if packets_received == 1:
                    print("✅ TELEMETRY DETECTED!")
                    print(f"   Packet size: {len(data)} bytes")
                    print(f"   From: {addr}")
                print(f"   Packets received: {packets_received}", end='\r')
        except socket.timeout:
            continue
except KeyboardInterrupt:
    pass

print(f"\n\n{'='*50}")
if packets_received > 0:
    print(f"✅ SUCCESS! Received {packets_received} telemetry packets")
    print("   Robot is sending data correctly!")
    print("\n🚀 Ready to run auto-tuner:")
    print("   python auto_tune_pid.py --wheel 0")
else:
    print("❌ NO TELEMETRY DATA RECEIVED")
    print("\n📋 Troubleshooting:")
    print("   1. Is robot powered on?")
    print("   2. Is robot connected to WiFi?")
    print("   3. Have you flashed updated firmware?")
    print("      - STM32 (src/drivetrain/)")
    print("      - ESP32 (parserChange/robocup/)")
    print("   4. Try sending a move command:")
    print('      echo "1 move 1.0 0 0" | nc -u 239.42.42.42 10000')
print('='*50)

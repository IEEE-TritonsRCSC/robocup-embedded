#!/usr/bin/env python3
"""
Emergency robot reset - sends stop commands
"""

import socket
import struct
import time

MULTICAST_IP = "239.42.42.42"
COMMAND_PORT = 10000
ROBOT_ID = 1

print("🛑 Sending STOP commands to robot...")

send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Send multiple stop commands to make sure it gets through
for i in range(10):
    send_sock.sendto(f"{ROBOT_ID} move 0 0 0".encode(), (MULTICAST_IP, COMMAND_PORT))
    print(f"   Stop {i+1}/10", end='\r')
    time.sleep(0.1)

print("\n✅ Stop commands sent!")
print("\nIf robot still moving, try:")
print("  1. Power cycle the robot")
print("  2. Check if ESP32 is processing UART commands properly")

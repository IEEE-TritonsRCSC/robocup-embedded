#!/usr/bin/env python3
"""
Demo plotter with simulated data
Use this to see how the plotter works before flashing firmware
"""

import socket
import struct
import time
import math
import argparse

MULTICAST_IP = "239.42.42.42"
TELEMETRY_PORT = 10001

def send_simulated_telemetry(kp=0.5, ki=0.05):
    """Send simulated PID response data"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    print(f"📡 Sending simulated telemetry data")
    print(f"   Using Kp={kp}, Ki={ki}")
    print(f"   Target: Step from 0 → 1000")
    print("\n   Start pid_plotter.py in another terminal to see this!")
    print("   Press Ctrl+C to stop\n")
    
    timestamp_ms = 0
    target = 0
    actual = 0
    integral = 0
    
    try:
        while True:
            # Simulate step input at t=2s
            if timestamp_ms > 2000:
                target = 1000
            else:
                target = 0
            
            # Simulate PID response
            error = target - actual
            integral += error * 0.01  # dt = 10ms
            
            # Simple simulation: actual tries to follow target with some dynamics
            pid_output = kp * error + ki * integral
            actual += pid_output * 0.01  # Simple integration
            
            # Add some realistic behavior
            if actual < 0:
                actual = 0
            if actual > 1200:
                actual = 1200
            
            # Build telemetry packet
            packet = bytearray(32)
            packet[0] = 0xFE  # Header
            packet[1] = 0xED
            
            # Timestamp
            packet[2:6] = struct.pack('>I', timestamp_ms)
            
            # All 4 wheels (same data for demo)
            for i in range(4):
                idx = 6 + (i * 6)
                packet[idx:idx+2] = struct.pack('>h', int(target))
                packet[idx+2:idx+4] = struct.pack('>h', int(actual))
                packet[idx+4:idx+6] = struct.pack('>h', int(pid_output))
            
            # Checksum
            checksum = sum(packet[2:30]) & 0xFF
            packet[30] = checksum
            packet[31] = ord('\n')
            
            # Send
            sock.sendto(packet, (MULTICAST_IP, TELEMETRY_PORT))
            
            # Update for next iteration
            timestamp_ms += 100
            time.sleep(0.1)
            
            if timestamp_ms % 1000 == 0:
                print(f"t={timestamp_ms/1000:.1f}s: target={target:.0f}, actual={actual:.0f}, error={error:.0f}")
    
    except KeyboardInterrupt:
        print("\n\n👋 Stopped simulation")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send simulated PID telemetry")
    parser.add_argument('--kp', type=float, default=0.5, help='Proportional gain')
    parser.add_argument('--ki', type=float, default=0.05, help='Integral gain')
    args = parser.parse_args()
    
    send_simulated_telemetry(args.kp, args.ki)

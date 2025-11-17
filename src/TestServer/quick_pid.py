#!/usr/bin/env python3
"""
Quick PID setter - Use this BEFORE reflashing firmware to test values
"""

import socket
import sys

MULTICAST_IP = "239.42.42.42"
MULTICAST_PORT = 10000
ROBOT_ID = 1

def send_pid(kp, ki, kd):
    """Send PID values to all wheels"""
    kp_q = int(kp * 1000)
    ki_q = int(ki * 1000)
    kd_q = int(kd * 1000)
    
    cmd = f"{ROBOT_ID} pidu 4 {kp_q} {ki_q} {kd_q}"
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd.encode(), (MULTICAST_IP, MULTICAST_PORT))
    sock.close()
    
    print(f"✅ Sent: {cmd}")
    print(f"   Kp={kp}, Ki={ki}, Kd={kd}")
    print("\nNow test with wasd_teleop.py or send move commands!")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python quick_pid.py <Kp> <Ki> [Kd]")
        print("\nExamples:")
        print("  python quick_pid.py 10 0.5      # Kp=10, Ki=0.5, Kd=0")
        print("  python quick_pid.py 20 1.0 0    # Kp=20, Ki=1.0, Kd=0")
        print("\nRecommended starting values:")
        print("  python quick_pid.py 10 0.5      # Balanced")
        print("  python quick_pid.py 15 1.0      # More aggressive")
        print("  python quick_pid.py 5 0.3       # Conservative")
        sys.exit(1)
    
    kp = float(sys.argv[1])
    ki = float(sys.argv[2])
    kd = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0
    
    send_pid(kp, ki, kd)

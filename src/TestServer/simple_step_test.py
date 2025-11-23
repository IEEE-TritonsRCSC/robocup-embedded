#!/usr/bin/env python3
"""
Simple step response test - just collect and display data
"""

import socket
import struct
import time
import json

TELEMETRY_PORT = 10001
MULTICAST_IP = "239.42.42.42"
COMMAND_PORT = 10000
ROBOT_ID = 1
WHEEL_IDX = 0

# Setup sockets
recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
recv_sock.bind(('', TELEMETRY_PORT))
mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
recv_sock.settimeout(0.5)

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

print("🧪 Simple Step Response Test")
print("="*60)
print("This will:")
print("  1. Stop the robot")
print("  2. Wait 2 seconds")
print("  3. Send move command")
print("  4. Collect data for 3 seconds")
print("  5. Save to JSON file")
print("="*60)

input("Press ENTER to start...")

# Stop
print("\n1️⃣ Stopping robot...")
send_command(f"{ROBOT_ID} move 0 0 0")
time.sleep(2.0)

# Clear buffer
print("2️⃣ Clearing buffer...")
try:
    while True:
        recv_sock.recvfrom(1024)
except socket.timeout:
    pass

# Start test
print("3️⃣ Sending move command and collecting data...")
data = []

send_command(f"{ROBOT_ID} move 1.0 0 0")

start_time = time.time()
while (time.time() - start_time) < 3.0:
    try:
        packet, _ = recv_sock.recvfrom(1024)
        telemetry = parse_telemetry(packet)
        if telemetry:
            wheel = telemetry['wheels'][WHEEL_IDX]
            data.append({
                'time': telemetry['timestamp'],
                'target': wheel['target'],
                'actual': wheel['actual'],
                'output': wheel['output']
            })
            if len(data) % 10 == 0:
                print(f"   Collected {len(data)} packets... T={wheel['target']}, A={wheel['actual']}", end='\r')
    except socket.timeout:
        continue

print(f"\n   ✓ Collected {len(data)} data points")

# Stop
print("4️⃣ Stopping robot...")
send_command(f"{ROBOT_ID} move 0 0 0")

# Save
filename = f"step_test_wheel{WHEEL_IDX}_{int(time.time())}.json"
with open(filename, 'w') as f:
    json.dump(data, f, indent=2)

print(f"5️⃣ Saved to {filename}")

# Display summary
if len(data) > 0:
    targets = [d['target'] for d in data]
    actuals = [d['actual'] for d in data]
    print(f"\n📊 Summary:")
    print(f"   Samples: {len(data)}")
    print(f"   Duration: {data[-1]['time'] - data[0]['time']:.2f}s")
    print(f"   Target: min={min(targets)}, max={max(targets)}")
    print(f"   Actual: min={min(actuals)}, max={max(actuals)}")
    print(f"\n   First 5 samples:")
    for i in range(min(5, len(data))):
        d = data[i]
        print(f"     [{i}] t={d['time']:.3f}s  target={d['target']:5d}  actual={d['actual']:5d}")

#!/usr/bin/env python3
"""
Verification script for jittering fix
Run this after flashing updated STM32 code
"""

import socket
import time
import sys

MULTICAST_IP = "239.42.42.42"
COMMAND_PORT = 10000
TELEMETRY_PORT = 10001
ROBOT_ID = 1

def send_command(cmd):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd.encode(), (MULTICAST_IP, COMMAND_PORT))
    sock.close()

def check_telemetry():
    """Check if telemetry is still being sent (should NOT be)"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', TELEMETRY_PORT))
    
    import struct
    mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    sock.settimeout(2.0)
    
    try:
        data, _ = sock.recvfrom(1024)
        return True  # Telemetry received (unexpected!)
    except socket.timeout:
        return False  # No telemetry (expected!)

print("="*70)
print("🔧 JITTERING FIX VERIFICATION")
print("="*70)
print("\nThis script will verify that the jittering fix is working.")
print("Make sure:")
print("  1. You've flashed the updated STM32 code")
print("  2. Robot is powered on and on the ground")
print("  3. You're on the same WiFi network")
print("\n")

input("Press ENTER when ready to begin...")

# Test 1: Check telemetry disabled
print("\n" + "="*70)
print("TEST 1: Verify Telemetry is Disabled")
print("="*70)
print("Checking for telemetry packets (should be NONE)...")

has_telemetry = check_telemetry()
if has_telemetry:
    print("❌ FAILED: Telemetry is still being sent!")
    print("   Did you flash the updated code?")
    sys.exit(1)
else:
    print("✅ PASSED: No telemetry detected (correct!)")

# Test 2: Send stop command
print("\n" + "="*70)
print("TEST 2: Verify Robot Responds to Commands")
print("="*70)
print("Sending STOP command...")

for i in range(5):
    send_command(f"{ROBOT_ID} move 0 0 0")
    time.sleep(0.1)

print("✅ Stop commands sent")
print("\n⚠️  OBSERVE THE ROBOT:")
print("   - Are wheels completely still?")
print("   - Any slow spinning?")
print("   - Any jittering or twitching?")
print()

response = input("Are wheels COMPLETELY STILL? (y/n): ").strip().lower()

if response != 'y':
    print("\n❌ ISSUE DETECTED: Wheels still moving when target=0")
    print("\n📋 Troubleshooting:")
    print("   1. Power cycle the robot completely")
    print("   2. Verify you flashed the CORRECT STM32 binary")
    print("   3. Check for mechanical binding (wheels should spin freely)")
    print("   4. Check for electrical issues (bad CAN bus?)")
    print()
    print("Advanced debug:")
    print("   - Add Serial.print() in ESP32 to see what commands it receives")
    print("   - Add LED toggle in STM32 main loop to verify code is running")
    sys.exit(1)

print("✅ PASSED: Wheels are stationary")

# Test 3: Gentle movement test
print("\n" + "="*70)
print("TEST 3: Verify Smooth Movement")
print("="*70)
print("Sending gentle forward command (speed=0.5)...")
print("Robot will move forward for 2 seconds")
print()

input("Press ENTER to start movement test...")

send_command(f"{ROBOT_ID} move 0.5 0 0")
print("Moving forward...")
time.sleep(2.0)

send_command(f"{ROBOT_ID} move 0 0 0")
print("Stopped!")
print()
print("⚠️  OBSERVE THE ROBOT:")
print("   - Was motion smooth (no jerking)?")
print("   - Did it stop cleanly?")
print("   - Any oscillation around target speed?")
print()

response = input("Was movement SMOOTH? (y/n): ").strip().lower()

if response != 'y':
    print("\n⚠️  Movement issues detected")
    print("\n📋 This might be normal with Kp=0.3, Ki=0")
    print("   You may see:")
    print("   - Slightly sluggish response (OK)")
    print("   - Doesn't reach full commanded speed (OK)")
    print("   - Takes time to accelerate (OK)")
    print()
    print("   NOT OK:")
    print("   - Violent oscillation or vibration")
    print("   - Erratic speed changes")
    print("   - Jittering")
    print()
    
    response2 = input("Was it violent/erratic? (y/n): ").strip().lower()
    if response2 == 'y':
        print("\n❌ FAILED: Violent motion detected")
        print("Try even lower Kp:")
        print('  echo "1 pidu 4 200 0 0" | nc -u 239.42.42.42 10000  # Kp=0.2')
        sys.exit(1)
    else:
        print("✅ OK: Sluggish but stable (expected with conservative PID)")
else:
    print("✅ PASSED: Movement is smooth")

# Test 4: Return to zero
print("\n" + "="*70)
print("TEST 4: Verify Clean Stop")
print("="*70)
print("Ensuring robot is stopped...")

for i in range(5):
    send_command(f"{ROBOT_ID} move 0 0 0")
    time.sleep(0.1)

print("\nWait 3 seconds for robot to settle...")
time.sleep(3)

print("\n⚠️  OBSERVE THE ROBOT:")
print("   - Are wheels completely still now?")
print("   - Any residual jittering?")
print()

response = input("Wheels completely still after stop? (y/n): ").strip().lower()

if response != 'y':
    print("\n❌ ISSUE: Wheels not stopping cleanly")
    print("\n📋 Possible causes:")
    print("   1. Integral windup (but Ki=0, so unlikely)")
    print("   2. Target speeds not actually zero (ESP32 issue?)")
    print("   3. CAN feedback giving wrong values")
    print()
    print("Next steps:")
    print("   - Check ESP32 serial output")
    print("   - Power cycle robot")
    print("   - Consider setting even lower Kp (0.2 or 0.1)")
    sys.exit(1)

print("✅ PASSED: Clean stop")

# Summary
print("\n" + "="*70)
print("🎉 VERIFICATION COMPLETE!")
print("="*70)
print("\n✅ All tests passed! The jittering fix is working.")
print()
print("📊 Current Configuration:")
print("   Kp = 0.3 (all wheels)")
print("   Ki = 0.0 (all wheels)")
print("   Kd = 0.0 (all wheels)")
print("   Telemetry = DISABLED")
print()
print("🚀 Next Steps:")
print("   1. You can now tune for better performance:")
print("      - Increase Kp for faster response")
print("      - Add Ki carefully for accuracy")
print("   2. Use quick_pid.py to test values without reflashing")
print("   3. Read ROBOT_PID_CALIBRATION.md for tuning guide")
print()
print("📝 Document your final PID values in:")
print("   ROBOT_PID_CALIBRATION.md")
print()

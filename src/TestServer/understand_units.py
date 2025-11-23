#!/usr/bin/env python3
"""
Unit Understanding Tool
Helps you figure out what your speed commands actually mean
"""

import socket
import time

MULTICAST_IP = "239.42.42.42"
MULTICAST_PORT = 10000
ROBOT_ID = 1

# From velocityConversions.h
WHEEL_RADIUS = 0.02425  # meters
GEAR_RATIO = 36
MAX_RPM = 15000
MAX_VELOCITY = 175
RESCALE_FACTOR = MAX_RPM / MAX_VELOCITY  # ≈ 85.7

def send_command(cmd):
    """Send UDP command to robot"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd.encode(), (MULTICAST_IP, MULTICAST_PORT))
    sock.close()

def explain_units():
    """Explain what the speed units mean"""
    print("="*70)
    print("🔍 UNDERSTANDING YOUR SPEED UNITS")
    print("="*70)
    
    print("\n📊 Your Robot's Constants:")
    print(f"  Wheel Radius: {WHEEL_RADIUS} m")
    print(f"  Gear Ratio: {GEAR_RATIO}")
    print(f"  Max Motor RPM: {MAX_RPM}")
    print(f"  Max Velocity: {MAX_VELOCITY} (units unknown)")
    print(f"  Rescale Factor: {RESCALE_FACTOR:.2f}")
    
    print("\n🔄 Unit Conversion Chain:")
    print("  1. Your command: vx=1.0 m/s (linear velocity)")
    print("  2. Kinematics converts to wheel speeds (rad/s)")
    print("  3. Divided by wheel radius → still rad/s or m/s")
    print("  4. Multiplied by RESCALE_FACTOR (85.7)")
    print("  5. Sent to STM32 as int16")
    print("  6. STM32 PID uses this as 'target speed'")
    print("  7. CAN feedback reports motor speed in same units")
    
    print("\n📏 What Does 'speed = 1' Mean?")
    print("  Looking at your code:")
    print("    wheel_speed = (vx * cos(angle) + ...) / WHEEL_RADIUS")
    print("    wheel_speed *= RESCALE_FACTOR")
    print()
    print("  If you command vx=1.0 m/s:")
    
    # Example calculation
    vx = 1.0
    # For forward motion, approximate wheel speed
    wheel_speed_rads = vx / WHEEL_RADIUS  # rad/s
    wheel_speed_rescaled = wheel_speed_rads * RESCALE_FACTOR
    
    print(f"    → wheel_speed ≈ {vx}/{WHEEL_RADIUS} = {wheel_speed_rads:.2f} rad/s")
    print(f"    → rescaled = {wheel_speed_rads:.2f} × {RESCALE_FACTOR:.2f} = {wheel_speed_rescaled:.2f}")
    
    print(f"\n  So 'speed = 1' in your system ≈ {1/RESCALE_FACTOR:.4f} rad/s")
    print(f"  Or in RPM: {(1/RESCALE_FACTOR * 60 / (2*3.14159)):.2f} RPM")
    
    print("\n⚠️  ISSUE: Your speed units are NOT standard!")
    print("     They're a mix of rad/s and an arbitrary rescale factor.")
    print("     This makes PID tuning confusing.")

def measure_real_speed():
    """Help user measure actual speed vs commanded speed"""
    print("\n" + "="*70)
    print("📐 LET'S MEASURE YOUR ACTUAL SPEEDS")
    print("="*70)
    
    print("\nYou'll need:")
    print("  1. A ruler or measuring tape")
    print("  2. A stopwatch/phone timer")
    print("  3. Space for robot to move ~2 meters")
    
    input("\nPress Enter when ready...")
    
    test_speeds = [0.5, 1.0, 2.0]
    
    for speed in test_speeds:
        print(f"\n--- Test: vx = {speed} ---")
        print(f"I'll command the robot to move forward at speed {speed}")
        print("Measure how far it travels in 2 seconds!")
        
        input("Press Enter to start (robot will move in 2 seconds)...")
        
        print("Get ready...")
        time.sleep(2)
        
        print(f"GO! (Moving at speed {speed})")
        send_command(f"{ROBOT_ID} move {speed} 0 0")
        
        time.sleep(2)
        
        send_command(f"{ROBOT_ID} move 0 0 0")
        print("STOPPED!")
        
        distance = input(f"\nHow far did it travel? (meters): ")
        
        try:
            dist = float(distance)
            actual_speed = dist / 2.0  # m/s
            print(f"  Commanded: {speed}")
            print(f"  Actual: {actual_speed:.3f} m/s")
            print(f"  Ratio: {actual_speed/speed:.3f}")
        except:
            print("  Invalid input, skipping calculation")
        
        time.sleep(1)

def test_pid_response():
    """Test PID response at different speeds"""
    print("\n" + "="*70)
    print("🎯 PID RESPONSE TEST")
    print("="*70)
    
    print("\nThis test shows how well your PID tracks different speeds.")
    print("Watch if the robot:")
    print("  - Reaches target quickly")
    print("  - Oscillates or vibrates")
    print("  - Has smooth motion")
    
    kp = float(input("\nEnter Kp to test: "))
    ki = float(input("Enter Ki to test: "))
    
    # Set PID
    kp_q = int(kp * 1000)
    ki_q = int(ki * 1000)
    cmd = f"{ROBOT_ID} pidu 4 {kp_q} {ki_q} 0"
    send_command(cmd)
    time.sleep(0.2)
    
    speeds = [0.5, 1.0, 1.5, 2.0]
    
    for speed in speeds:
        print(f"\n--- Testing speed = {speed} ---")
        send_command(f"{ROBOT_ID} move {speed} 0 0")
        
        print(f"Running for 3 seconds...")
        time.sleep(3)
        
        send_command(f"{ROBOT_ID} move 0 0 0")
        
        response = input(f"  At speed {speed}: (g=good, s=slow, o=oscillate): ")
        
        if response.lower() == 'o':
            print("  💡 Oscillation → Decrease Kp")
        elif response.lower() == 's':
            print("  💡 Slow response → Increase Kp")
        
        time.sleep(0.5)

def systematic_tuning_guide():
    """Step-by-step PID tuning procedure"""
    print("\n" + "="*70)
    print("📚 SYSTEMATIC PID TUNING GUIDE")
    print("="*70)
    
    print("""
The BEST way to PID tune:

STEP 1: UNDERSTAND YOUR SYSTEM
  - Know what 'speed=1' means (run measurement test)
  - Pick a representative test speed (e.g., 1.0 m/s)
  - Use consistent test conditions

STEP 2: TUNE P FIRST (Set Ki=0, Kd=0)
  a) Start with P = 0.1
  b) Send step command (e.g., move 1.0 0 0)
  c) Observe:
     - Too slow? → Increase P by 2x
     - Oscillates? → Decrease P by 0.7x
     - Good but not exact? → Go to Step 3
  d) Repeat until you get fast response with ~10-20% overshoot
  e) Note this P value

STEP 3: ADD I FOR ACCURACY (Keep Kd=0)
  a) Start with Ki = P/10
  b) Send step command
  c) Observe:
     - Reaches exact target? → Good! ✅
     - Oscillates slowly? → Decrease Ki by 0.5x
     - Still has error? → Increase Ki by 1.5x
  d) Repeat until steady-state error = 0

STEP 4: (Optional) ADD D FOR DAMPING
  - Usually NOT needed for velocity control
  - Only if you have overshoot and can't fix with P/I
  - Start with Kd = P/100

STEP 5: TEST ACROSS RANGE
  - Test at different speeds (0.5, 1.0, 2.0)
  - Test different directions (forward, strafe, rotate)
  - Verify consistent performance

STEP 6: TUNE INDIVIDUAL WHEELS (if needed)
  - If one wheel behaves differently
  - Adjust its P/I separately
  - Usually within ±20% of baseline
""")

def main():
    print("🤖 Unit Understanding & PID Tuning Guide")
    print("="*70)
    
    while True:
        print("\n📋 MENU:")
        print("  1. Explain what 'speed' units mean")
        print("  2. Measure actual speeds (need ruler)")
        print("  3. Test PID response at different speeds")
        print("  4. Show systematic tuning guide")
        print("  0. Exit")
        
        choice = input("\nSelect option: ").strip()
        
        if choice == '1':
            explain_units()
        elif choice == '2':
            measure_real_speed()
        elif choice == '3':
            test_pid_response()
        elif choice == '4':
            systematic_tuning_guide()
        elif choice == '0':
            print("Exiting...")
            send_command(f"{ROBOT_ID} move 0 0 0")
            break
        else:
            print("Invalid option!")

if __name__ == "__main__":
    main()

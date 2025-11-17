#!/usr/bin/env python3
"""
PID Tuning Helper for Omni-Wheel Robot
Helps diagnose deadband issues and find optimal PID values
"""

import socket
import time
import argparse

MULTICAST_IP = "239.42.42.42"
MULTICAST_PORT = 10000
ROBOT_ID = 1

def send_command(cmd):
    """Send UDP command to robot"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd.encode(), (MULTICAST_IP, MULTICAST_PORT))
    sock.close()
    print(f"Sent: {cmd}")

def stop_robot():
    """Emergency stop"""
    send_command(f"{ROBOT_ID} move 0 0 0")
    print("🛑 Robot stopped")

def set_pid(wheel_idx, kp, ki, kd):
    """
    Set PID values for specific wheel or all wheels
    wheel_idx: 0-3 for individual wheels, 4+ for all wheels
    Values are in real units (will be converted to milli-units)
    """
    kp_q = int(kp * 1000)
    ki_q = int(ki * 1000)
    kd_q = int(kd * 1000)
    cmd = f"{ROBOT_ID} pidu {wheel_idx} {kp_q} {ki_q} {kd_q}"
    send_command(cmd)
    time.sleep(0.1)

def test_deadband():
    """Test to find minimum motor activation threshold"""
    print("\n" + "="*60)
    print("🔍 DEADBAND TEST - Finding minimum motor activation")
    print("="*60)
    print("\nThis test will gradually increase P until motors respond")
    print("Watch which wheels start moving first!\n")
    
    stop_robot()
    time.sleep(0.5)
    
    # Start with very low P values
    p_values = [0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 20.0, 30.0, 50.0]
    
    for kp in p_values:
        print(f"\n--- Testing Kp = {kp}, Ki = 0, Kd = 0 ---")
        set_pid(4, kp, 0, 0)  # All wheels
        time.sleep(0.2)
        
        # Send forward command
        send_command(f"{ROBOT_ID} move 1.0 0 0")
        
        response = input(f"  At Kp={kp}: Which wheels are moving? (type 'all', 'some', 'none', or 'q' to quit): ")
        
        stop_robot()
        time.sleep(0.5)
        
        if response.lower() == 'q':
            break
        elif response.lower() == 'all':
            print(f"\n✅ SUCCESS! All wheels moving at Kp = {kp}")
            print(f"Recommended starting Kp: {kp}")
            print(f"Try adding Ki = {kp * 0.05} for steady-state accuracy")
            break
        elif response.lower() == 'some':
            print(f"  ⚠️  Only some wheels moving - increase P more")
        else:
            print(f"  ❌ No movement - increase P significantly")
    
    stop_robot()

def test_individual_wheels():
    """Test each wheel individually to find asymmetric issues"""
    print("\n" + "="*60)
    print("🔧 INDIVIDUAL WHEEL TEST")
    print("="*60)
    print("\nTesting each wheel separately to find mechanical differences\n")
    
    kp = float(input("Enter Kp value to test (e.g., 10.0): "))
    ki = float(input("Enter Ki value to test (e.g., 0.5): "))
    
    wheel_names = ["Front-Right (0)", "Back-Right (1)", "Back-Left (2)", "Front-Left (3)"]
    
    for idx, name in enumerate(wheel_names):
        print(f"\n--- Testing {name} ---")
        
        # Set individual wheel PID
        set_pid(idx, kp, ki, 0)
        time.sleep(0.2)
        
        # Test with a simple forward command (all wheels should get same target)
        send_command(f"{ROBOT_ID} move 0.5 0 0")
        
        input(f"  Observe wheel {idx} ({name}). Press Enter when ready...")
        
        stop_robot()
        time.sleep(0.5)
        
        adjust = input(f"  Does wheel {idx} need adjustment? (h=higher P, l=lower P, enter=OK): ")
        
        if adjust.lower() == 'h':
            new_kp = kp * 1.2
            print(f"  Suggested Kp for wheel {idx}: {new_kp:.1f}")
        elif adjust.lower() == 'l':
            new_kp = kp * 0.8
            print(f"  Suggested Kp for wheel {idx}: {new_kp:.1f}")

def test_movement_patterns():
    """Test various movement patterns"""
    print("\n" + "="*60)
    print("🎯 MOVEMENT PATTERN TEST")
    print("="*60)
    
    kp = float(input("\nEnter Kp value: "))
    ki = float(input("Enter Ki value: "))
    kd = float(input("Enter Kd value (usually 0): "))
    
    set_pid(4, kp, ki, kd)
    time.sleep(0.2)
    
    patterns = [
        ("Forward", f"{ROBOT_ID} move 0.5 0 0", 2.0),
        ("Backward", f"{ROBOT_ID} move -0.5 0 0", 2.0),
        ("Strafe Right", f"{ROBOT_ID} move 0 0.5 0", 2.0),
        ("Strafe Left", f"{ROBOT_ID} move 0 -0.5 0", 2.0),
        ("Rotate CW", f"{ROBOT_ID} move 0 0 1.0", 3.0),
        ("Rotate CCW", f"{ROBOT_ID} move 0 0 -1.0", 3.0),
        ("Diagonal", f"{ROBOT_ID} move 0.5 0.5 0", 2.0),
    ]
    
    for name, cmd, duration in patterns:
        print(f"\n--- Testing: {name} ---")
        send_command(cmd)
        
        print(f"Running for {duration}s...")
        time.sleep(duration)
        
        stop_robot()
        
        response = input(f"  {name}: (g=good, o=oscillation, w=weak, enter=continue): ")
        
        if response.lower() == 'o':
            print("  💡 Decrease Kp by 20-30%")
        elif response.lower() == 'w':
            print("  💡 Increase Kp by 20-50%")
        
        time.sleep(0.5)
    
    stop_robot()

def quick_test():
    """Quick test with custom values"""
    print("\n" + "="*60)
    print("⚡ QUICK PID TEST")
    print("="*60)
    
    kp = float(input("\nEnter Kp: "))
    ki = float(input("Enter Ki: "))
    kd = float(input("Enter Kd: "))
    
    set_pid(4, kp, ki, kd)
    print("\nPID values set! Use WASD teleop to test.")
    print("Press Ctrl+C when done testing.")
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopping...")
        stop_robot()

def main():
    parser = argparse.ArgumentParser(description="PID Tuning Helper for Omni-Wheel Robot")
    parser.add_argument('--robot', type=int, default=1, help='Robot ID (default: 1)')
    args = parser.parse_args()
    
    global ROBOT_ID
    ROBOT_ID = args.robot
    
    print("🤖 PID Tuning Helper for Omni-Wheel Robot")
    print(f"   Robot ID: {ROBOT_ID}")
    print(f"   Multicast: {MULTICAST_IP}:{MULTICAST_PORT}")
    print("\n" + "="*60)
    
    try:
        while True:
            print("\n📋 MENU:")
            print("  1. Deadband Test (find minimum P)")
            print("  2. Individual Wheel Test")
            print("  3. Movement Pattern Test")
            print("  4. Quick Test (set PID and manual control)")
            print("  5. Emergency Stop")
            print("  0. Exit")
            
            choice = input("\nSelect option: ").strip()
            
            if choice == '1':
                test_deadband()
            elif choice == '2':
                test_individual_wheels()
            elif choice == '3':
                test_movement_patterns()
            elif choice == '4':
                quick_test()
            elif choice == '5':
                stop_robot()
            elif choice == '0':
                print("Exiting...")
                stop_robot()
                break
            else:
                print("Invalid option!")
    
    except KeyboardInterrupt:
        print("\n\nInterrupted! Stopping robot...")
        stop_robot()

if __name__ == "__main__":
    main()

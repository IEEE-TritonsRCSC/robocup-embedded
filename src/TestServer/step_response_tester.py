#!/usr/bin/env python3
"""
Automated step response test for PID tuning
Sends step commands and measures response characteristics
"""
import socket
import time
import sys

MCAST_GRP = "239.42.42.42"
MCAST_PORT = 10000
ROBOT_ID = 1

def send_command(sock, cmd):
    """Send UDP command to robot"""
    msg = f"{ROBOT_ID} {cmd}\n"
    sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))
    print(f"→ Sent: {cmd}")

def step_test_sequence(sock, step_size=500, duration=5):
    """
    Run a step response test
    
    Args:
        step_size: Target speed in RPM
        duration: How long to hold the command (seconds)
    """
    print("\n" + "="*60)
    print(f"📊 STEP RESPONSE TEST")
    print(f"Step size: {step_size} RPM")
    print(f"Duration: {duration} seconds")
    print("="*60)
    
    # Stop first
    print("\n1️⃣  Setting initial state (stopped)...")
    send_command(sock, "move 0 0 0")
    time.sleep(2)
    
    # Apply step
    print(f"\n2️⃣  Applying step command ({step_size} RPM forward)...")
    send_command(sock, f"move {step_size} 0 0")
    
    print(f"\n⏱️  Holding for {duration} seconds...")
    print("   Watch the real-time plotter for response curve!")
    time.sleep(duration)
    
    # Stop
    print("\n3️⃣  Stopping...")
    send_command(sock, "move 0 0 0")
    time.sleep(2)
    
    print("\n✅ Step test complete!")
    print("   Analyze the plot to evaluate PID performance:")
    print("   • Rise time: How fast does it reach ~90% of target?")
    print("   • Overshoot: Does it exceed the target?")
    print("   • Settling time: How long to reach steady state?")
    print("   • Steady-state error: Final error at equilibrium?")

def rotation_test(sock, rot_speed=500, duration=5):
    """Test rotation (useful for IMU feedback later)"""
    print("\n" + "="*60)
    print(f"🔄 ROTATION TEST")
    print(f"Rotation speed: {rot_speed} RPM")
    print(f"Duration: {duration} seconds")
    print("="*60)
    
    print("\n1️⃣  Rotating CCW...")
    send_command(sock, f"move 0 0 {rot_speed}")
    time.sleep(duration)
    
    print("\n2️⃣  Stopping...")
    send_command(sock, "move 0 0 0")
    time.sleep(2)
    
    print("\n✅ Rotation test complete!")

def strafe_test(sock, strafe_speed=500, duration=5):
    """Test strafing"""
    print("\n" + "="*60)
    print(f"↔️  STRAFE TEST")
    print(f"Strafe speed: {strafe_speed} RPM")
    print(f"Duration: {duration} seconds")
    print("="*60)
    
    print("\n1️⃣  Strafing right...")
    send_command(sock, f"move {strafe_speed} 0 0")
    time.sleep(duration)
    
    print("\n2️⃣  Stopping...")
    send_command(sock, "move 0 0 0")
    time.sleep(2)
    
    print("\n✅ Strafe test complete!")

def pid_adjustment_guide():
    """Print PID tuning guide"""
    print("\n" + "="*60)
    print("📖 PID TUNING GUIDE")
    print("="*60)
    print("""
Based on the plot, adjust PID values:

🐌 SLOW RESPONSE (takes >2 seconds to reach target):
   → Increase Kp by 0.1-0.2
   → Command: python quick_pid.py [NEW_KP] [CURRENT_KI]

📳 OSCILLATION (vibrates around target):
   → Decrease Kp by 0.1
   → Command: python quick_pid.py [NEW_KP] [CURRENT_KI]

🎯 STEADY-STATE ERROR (reaches 90% but not 100%):
   → Add Ki = 0.05 - 0.1
   → Command: python quick_pid.py [CURRENT_KP] [NEW_KI]

🚀 OVERSHOOT (exceeds target then settles):
   → Decrease Kp slightly OR add small Kd
   → Command: python quick_pid.py [NEW_KP] [CURRENT_KI] [KD]

Current defaults:
   Kp = 0.3, Ki = 0.0, Kd = 0.0

Example commands:
   python quick_pid.py 0.5 0.0      # Increase responsiveness
   python quick_pid.py 0.3 0.05     # Add integral for accuracy
   python quick_pid.py 0.4 0.05 0.01  # Balanced tuning
""")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="PID Step Response Tester")
    parser.add_argument("--speed", type=int, default=500, help="Step size in RPM (default: 500)")
    parser.add_argument("--duration", type=float, default=5.0, help="Test duration in seconds (default: 5)")
    parser.add_argument("--test", choices=['forward', 'rotation', 'strafe', 'all'], 
                       default='forward', help="Type of test to run")
    parser.add_argument("--guide", action="store_true", help="Show PID tuning guide")
    args = parser.parse_args()
    
    if args.guide:
        pid_adjustment_guide()
        return
    
    # Setup socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)
    
    print("🤖 PID Step Response Tester")
    print(f"Target: {MCAST_GRP}:{MCAST_PORT}, Robot ID: {ROBOT_ID}")
    print("\n💡 Make sure realtime_pid_plotter.py is running in another window!")
    print("   Command: python realtime_pid_plotter.py")
    
    try:
        if args.test == 'forward' or args.test == 'all':
            step_test_sequence(sock, args.speed, args.duration)
        
        if args.test == 'rotation' or args.test == 'all':
            time.sleep(3)
            rotation_test(sock, args.speed, args.duration)
        
        if args.test == 'strafe' or args.test == 'all':
            time.sleep(3)
            strafe_test(sock, args.speed, args.duration)
        
        pid_adjustment_guide()
        
    except KeyboardInterrupt:
        print("\n\n⚠️  Test interrupted by user")
    finally:
        # Emergency stop
        print("\n🛑 Sending final stop command...")
        send_command(sock, "move 0 0 0")
        sock.close()

if __name__ == "__main__":
    main()

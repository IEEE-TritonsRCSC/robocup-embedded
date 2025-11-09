#!/usr/bin/env python3
"""
UART loopback test for ESP32 → STM32 communication debugging.
Sends a single test frame and waits for acknowledgment.
"""
import socket
import struct
import time
import argparse

MCAST_GRP = "239.42.42.42"
MCAST_PORT = 10000


def main():
    p = argparse.ArgumentParser(description="Send a single test command to robot")
    p.add_argument("--robot", type=int, default=1, help="Robot number")
    p.add_argument("--iface", default=None, help="Network interface IP")
    args = p.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack('b', 1))
    if args.iface:
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(args.iface))

    print(f"Sending test command to robot {args.robot} at {MCAST_GRP}:{MCAST_PORT}")
    print("Expected frame: CA FE 00 00 00 00 00 00 00 00 01")
    print("\nWatch ESP32 serial monitor for:")
    print("  ✓ 'TX to STM32: CA FE ...'")
    print("  ✓ 'STM32 ACK received: 01'")
    print("  ✗ 'WARNING: No ACK from STM32!'")
    print("\nWatch STM32 LEDs:")
    print("  - Red blinks 3x on startup = STM32 running")
    print("  - Green toggles = valid frame received")
    print("  - Red blinks fast = UART error")
    print("  - Red slow blink = timeout (no commands)\n")

    # Send a simple stationary command (all wheels 0, dribbler on)
    cmd = f"{args.robot} dash 0 0\n"
    
    for i in range(5):
        print(f"[{i+1}/5] Sending: {cmd.strip()}")
        sock.sendto(cmd.encode("utf-8"), (MCAST_GRP, MCAST_PORT))
        time.sleep(1)
    
    print("\n--- Test complete ---")
    print("\nTroubleshooting:")
    print("1. If ESP32 shows nothing:")
    print("   - Check WiFi connection")
    print("   - Verify robot number matches (ROBOT_NO in ESP32 code)")
    print("   - Check multicast route (same subnet)")
    print("\n2. If ESP32 shows TX but 'WARNING: No ACK':")
    print("   - Check wiring: ESP32 GPIO17 (TX) → STM32 PA3 (USART2 RX)")
    print("   - Check common GND between ESP32 and STM32")
    print("   - Verify STM32 red LED blinked 3x on power-up")
    print("   - Flash latest STM32 firmware with ACK support")
    print("\n3. If STM32 green LED toggles but no ACK:")
    print("   - Check STM32 TX pin (PA2) connected to ESP32 RX (GPIO16)")
    print("   - Verify UART initialized correctly (115200 8N1)")
    print("\n4. If red LED blinks slowly:")
    print("   - STM32 in timeout mode (normal if no commands)")
    print("   - Send more commands or increase timeout value")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
Velocity calibration tool for compensating motor speed differences.
Usage: python velocity_calibrate.py <FR> <FL> <BR> <BL>
Example: python velocity_calibrate.py 1.02 1.02 1.06 1.05
"""
import socket
import struct
import sys
import argparse

MCAST_GRP = "239.42.42.42"
MCAST_PORT = 10000

def get_local_ip():
    """Auto-detect the local IP on the robot's network (192.168.8.x)"""
    try:
        hostname = socket.gethostname()
        addrs = socket.getaddrinfo(hostname, None)
        for addr in addrs:
            ip = addr[4][0]
            if ip.startswith("192.168.8."):
                return ip
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except:
        return None

def main():
    parser = argparse.ArgumentParser(description="Velocity calibration for motor compensation")
    parser.add_argument("FR", type=float, help="Front Right multiplier (e.g., 1.02)")
    parser.add_argument("FL", type=float, help="Front Left multiplier (e.g., 1.02)")
    parser.add_argument("BR", type=float, help="Back Right multiplier (e.g., 1.06)")
    parser.add_argument("BL", type=float, help="Back Left multiplier (e.g., 1.05)")
    parser.add_argument("--robot", type=int, default=1, help="Robot number (default: 1)")
    parser.add_argument("--iface", default=None, help="Network interface IP")
    args = parser.parse_args()
    
    # Auto-detect interface
    if not args.iface:
        args.iface = get_local_ip()
        if args.iface:
            print(f"🔍 Using network interface: {args.iface}")
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack('b', 1))
    
    if args.iface:
        try:
            sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(args.iface))
        except OSError as e:
            print(f"⚠️  Failed to set interface: {e}")
    
    # Send velocity calibration command
    msg = f"{args.robot} vcal {args.FR:.3f} {args.FL:.3f} {args.BR:.3f} {args.BL:.3f}\n"
    sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))
    
    print(f"✅ Velocity calibration sent:")
    print(f"   FR (Front Right): {args.FR:.3f}")
    print(f"   FL (Front Left):  {args.FL:.3f}")
    print(f"   BR (Back Right):  {args.BR:.3f}")
    print(f"   BL (Back Left):   {args.BL:.3f}")
    print()
    print("💡 Test the robot's straight-line motion to verify calibration")
    print("   Adjust values incrementally (±0.01) until robot drives straight")

if __name__ == "__main__":
    main()

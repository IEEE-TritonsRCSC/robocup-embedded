#!/usr/bin/env python3
"""
Quick diagnostic tool to check if telemetry packets are being received
"""
import socket
import struct
import time

MCAST_GRP = "239.42.42.42"
TELEMETRY_PORT = 10001

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
    # Detect WiFi interface
    local_ip = get_local_ip()
    if local_ip:
        print(f"🔍 Using network interface: {local_ip}")
    else:
        print("⚠️  Could not detect interface, using system default")
        local_ip = socket.INADDR_ANY
    
    # Create UDP socket with multicast support
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', TELEMETRY_PORT))
    
    # Join multicast group on specific interface
    mcast_group = socket.inet_aton(MCAST_GRP)
    if isinstance(local_ip, str):
        local_iface = socket.inet_aton(local_ip)
    else:
        local_iface = struct.pack('I', socket.INADDR_ANY)
    mreq = struct.pack('4s4s', mcast_group, local_iface)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    
    sock.settimeout(1.0)
    
    print("=" * 60)
    print(f"🔍 Telemetry Diagnostic Tool")
    print(f"📡 Listening on UDP port {TELEMETRY_PORT}")
    print("=" * 60)
    print()
    print("Waiting for telemetry packets...")
    print("(Make sure robot is powered and receiving commands)")
    print()
    
    packet_count = 0
    last_print = time.time()
    
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            packet_count += 1
            
            # Print every second or on first packet
            if packet_count == 1 or (time.time() - last_print) >= 1.0:
                print(f"✅ Packet #{packet_count} received from {addr}")
                print(f"   Length: {len(data)} bytes")
                print(f"   Raw (hex): {' '.join(f'{b:02X}' for b in data[:16])}{'...' if len(data) > 16 else ''}")
                
                # Check header
                if len(data) >= 2:
                    if data[0] == 0xFE and data[1] == 0xED:
                        print(f"   ✅ Valid header (0xFE 0xED)")
                        
                        if len(data) >= 32:
                            # Parse timestamp
                            timestamp = struct.unpack('>I', data[2:6])[0]
                            print(f"   Timestamp: {timestamp} ms")
                            
                            # Parse first wheel as example
                            target = struct.unpack('>h', data[6:8])[0]
                            actual = struct.unpack('>h', data[8:10])[0]
                            output = struct.unpack('>h', data[10:12])[0]
                            print(f"   Wheel 0: Target={target}, Actual={actual}, Output={output}")
                        else:
                            print(f"   ⚠️  Packet too short (expected 32 bytes)")
                    else:
                        print(f"   ❌ Invalid header (got 0x{data[0]:02X} 0x{data[1]:02X})")
                print()
                last_print = time.time()
                
        except socket.timeout:
            print("⏳ No packets received (timeout)...")
            print("   Check: 1) Is STM32 flashed with telemetry enabled?")
            print("          2) Is ESP32 running and forwarding telemetry?")
            print("          3) Is robot receiving commands to generate motion?")
            print()
            time.sleep(1)
        except KeyboardInterrupt:
            print("\n👋 Exiting...")
            break

if __name__ == "__main__":
    main()

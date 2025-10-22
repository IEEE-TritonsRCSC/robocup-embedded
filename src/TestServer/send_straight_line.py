#!/usr/bin/env python3
"""
Send straight-line motion commands to the ESP32 robot over UDP multicast.
Matches robocup.ino parser: "<ROBOT_NO> dash <power> <angle>\n".
- Uses multicast group 239.42.42.42 and port 10000 (from robocup.ino)
- power in [-100,100] mapped by firmware
- angle is rotational velocity command; use 0 for straight line
"""
import argparse
import socket
import struct
import time

MCAST_GRP = "239.42.42.42"
MCAST_PORT = 10000


def main():
    p = argparse.ArgumentParser(description="Send straight-line UDP commands to RoboCup robot")
    p.add_argument("--robot", type=int, default=1, help="Robot number expected by firmware (ROBOT_NO)")
    p.add_argument("--power", type=int, default=-1, help="Forward command in [-100,100]")
    # duration <= 0 means run forever
    p.add_argument("--duration", type=float, default=5.0, help="Seconds to send; <=0 to run forever")
    p.add_argument("--rate", type=float, default=20.0, help="Send rate (Hz)")
    p.add_argument("--yaw-trim", type=int, default=0, help="Additive rotation trim to counter drift (e.g., 1 or -1)")
    p.add_argument("--iface", default=None, help="Optional local interface IP for multicast (e.g., 192.168.x.x)")
    args = p.parse_args()

    # UDP socket to multicast group
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    # TTL=1 (stay on local network)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack('b', 1))
    if args.iface:
        try:
            sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(args.iface))
        except OSError as e:
            print(f"Warning: failed to set multicast interface {args.iface}: {e}")

    def clamp(x: int, lo: int, hi: int) -> int:
        return max(lo, min(hi, x))

    def send(power: int, rot: int = 0):
        # Exact format expected by robocup.ino processCommand()
        rot_cmd = clamp(int(rot + args.yaw_trim), -100, 100)
        msg = f"{args.robot} dash {int(power)} {rot_cmd}\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    period = 1.0 / max(args.rate, 1e-3)

    try:
        if args.duration and args.duration > 0:
            t_end = time.time() + args.duration
            print(f"Sending to {MCAST_GRP}:{MCAST_PORT} as robot {args.robot} -> power={args.power}, rot=0 trim={args.yaw_trim} for {args.duration}s @ {args.rate}Hz")
            while time.time() < t_end:
                send(args.power, 0)
                time.sleep(period)
        else:
            print(f"Sending to {MCAST_GRP}:{MCAST_PORT} as robot {args.robot} -> power={args.power}, rot=0 trim={args.yaw_trim} continuously @ {args.rate}Hz (Ctrl+C to stop)")
            while True:
                send(args.power, 0)
                time.sleep(period)
    except KeyboardInterrupt:
        pass
    finally:
        # stop command to ensure the robot halts
        for _ in range(3):
            send(0, 0)
            time.sleep(0.05)
        print("Stopped.")


if __name__ == "__main__":
    main()

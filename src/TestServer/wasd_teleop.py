#!/usr/bin/env python3
"""
WASD controller for the RoboCup robot over UDP multicast.
Matches robocup.ino parser:
    - "<ROBOT_NO> dash <power> <angle>\n"  (we use angle as rotation command)
  - "<ROBOT_NO> kick\n"
Controls (Windows-friendly, no extra deps):
    W/S: increase/decrease forward power (range -10..10)
    A/D: rotate CCW/CW (range -10..10)
  SPACE: stop (power=0, rot=0)
  K: kick
  Q or ESC: quit
"""
import argparse
import socket
import struct
import sys
import time

MCAST_GRP = "239.42.42.42"
MCAST_PORT = 10000


def main():
    p = argparse.ArgumentParser(description="WASD teleop for RoboCup robot (UDP multicast)")
    p.add_argument("--robot", type=int, default=1, help="Robot number expected by firmware (ROBOT_NO)")
    p.add_argument("--rate", type=float, default=20.0, help="Send rate (Hz)")
    p.add_argument("--step-power", type=int, default=1, help="Increment per W/S keypress ([-10,10])")
    p.add_argument("--step-rot", type=int, default=1, help="Increment per A/D keypress ([-10,10])")
    p.add_argument("--step-drib", type= int, default=1, help="Increment per C/B keypress ([0, 20])") #dont really know acutal value, do some testing
    p.add_argument("--iface", default=None, help="Optional local interface IP for multicast (e.g., 192.168.x.x)")
    args = p.parse_args()

    # UDP socket to multicast group
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack('b', 1))
    if args.iface:
        try:
            sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(args.iface))
        except OSError as e:
            print(f"Warning: failed to set multicast interface {args.iface}: {e}")

    def send_dash(power: int, rot: int = 0):
        msg = f"{args.robot} dash {int(power)} {int(rot)}\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    def send_kick():
        msg = f"{args.robot} kick\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    def send_drib(drib_speed: int):
        msg = f"{args.robot} drib {int(drib_speed)}\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    # Current setpoints
    power = 0
    rot = 0
    drib_speed = 0
    P_MIN, P_MAX = -10, 10
    R_MIN, R_MAX = -10, 10
    D_MIN, D_MAX = 0, 20

    # Keyboard handling
    using_msvcrt = sys.platform.startswith("win")

    print("WASD teleop active -> sending to %s:%d for robot %d" % (MCAST_GRP, MCAST_PORT, args.robot))
    print("Controls: W/S forward/back, A/D rotate CCW/CW, SPACE stop, K kick, Q/ESC quit")

    period = 1.0 / max(args.rate, 1e-3)
    last_print = 0.0

    try:
        if using_msvcrt:
            import msvcrt
            while True:
                # non-blocking key read(s)
                while msvcrt.kbhit():
                    ch = msvcrt.getwch()
                    if ch in ('q', 'Q'):
                        return
                    if ord(ch) == 27:  # ESC
                        return
                    if ch in ('w', 'W'):
                        power = min(P_MAX, power + args.step_power)
                    elif ch in ('s', 'S'):
                        power = max(P_MIN, power - args.step_power)
                    elif ch in ('a', 'A'):
                        rot = min(R_MAX, rot + args.step_rot)
                    elif ch in ('d', 'D'):
                        rot = max(R_MIN, rot - args.step_rot)
                    elif ch == ' ':
                        power = 0; rot = 0
                    elif ch in ('k', 'K'):
                        send_kick()
                    elif ch in ('b', 'B'):
                        drib_speed = min(D_MAX, drib_speed + args.step_drib)
                    elif ch in ('c', 'C'):
                        drib_speed = max(D_MIN, drib_speed - args.step_drib)

                #send_dash(power, rot)
                send_drib(drib_speed)
                now = time.time()
                if now - last_print > 0.5:
                    print(f"power={power:>4}, rot={rot:>4}, drib_speed={drib_speed}", end="\r", flush=True)
                    last_print = now
                time.sleep(period)
        else:
            # POSIX fallback (simple, blocking getch using termios)
            import tty, termios, select
            fd = sys.stdin.fileno()
            old_settings = termios.tcgetattr(fd)
            tty.setcbreak(fd)
            try:
                while True:
                    r, _, _ = select.select([sys.stdin], [], [], 0)
                    if r:
                        ch = sys.stdin.read(1)
                        if ch in ('q', 'Q', '\x1b'):
                            return
                        if ch in ('w', 'W'):
                            power = min(P_MAX, power + args.step_power)
                        elif ch in ('s', 'S'):
                            power = max(P_MIN, power - args.step_power)
                        elif ch in ('a', 'A'):
                            rot = min(R_MAX, rot + args.step_rot)
                        elif ch in ('d', 'D'):
                            rot = max(R_MIN, rot - args.step_rot)
                        elif ch == ' ':
                            power = 0; rot = 0
                        elif ch in ('k', 'K'):
                            send_kick()
                        elif ch in ('b', 'B'):
                            drib_speed = min(D_MAX, drib_speed + args.step_drib)
                        elif ch in ('c', 'C'):
                            drib_speed = max(D_MIN, drib_speed - args.step_drib)
                    send_dash(power, rot, drib_speed)
                    now = time.time()
                    if now - last_print > 0.5:
                        print(f"power={power:>4}, rot={rot:>4}, drib_speed={drib_speed:>4}", end="\r", flush=True)
                        last_print = now
                    time.sleep(period)
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    finally:
        # Stop
        for _ in range(3):
            send_dash(0, 0)
            time.sleep(0.05)
        print("\nStopped.")


if __name__ == "__main__":
    main()

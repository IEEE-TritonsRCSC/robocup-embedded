import math
import socket
import time

ROBOT_ID = 1
COMMAND_IP = "239.42.42.42"
COMMAND_PORT = 10000
SEND_DELAY_S = 0.05


def send_message(sock, message):
    sock.sendto(message.encode(), (COMMAND_IP, COMMAND_PORT))


def send_stop(sock):
    send_message(sock, "stop\0")


def format_command(command, params):
    if params:
        return f"{ROBOT_ID} {command} {' '.join(params)}\0"
    return f"{ROBOT_ID} {command}\0"


def get_float(token):
    return float(token)


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dribbler_on = False

    print("Commands:")
    print("  d <speed> <direction_deg>  -> dash")
    print("  t <degrees_per_sec>        -> turn (positive CCW, negative CW)")
    print("  c                          -> toggle dribbler on/off")
    print("  k                          -> kick")
    print("  stop                       -> stop all motors")
    print("  q                          -> stop and quit")

    try:
        while True:
            line = input("Enter command: ").strip()
            if not line:
                continue

            lower = line.lower()
            if lower == "q":
                send_stop(sock)
                time.sleep(SEND_DELAY_S)
                send_stop(sock)
                break

            if lower == "stop":
                send_stop(sock)
                dribbler_on = False
                time.sleep(SEND_DELAY_S)
                continue

            parts = line.split()
            cmd = parts[0].lower()

            if cmd == "d":
                if len(parts) != 3:
                    print("Usage: d <speed> <direction_deg>")
                    continue
                speed = get_float(parts[1])
                direction_deg = get_float(parts[2])
                direction_rad = math.radians(direction_deg)
                message = format_command("dash", [str(speed), str(direction_rad)])
            elif cmd == "t":
                if len(parts) != 2:
                    print("Usage: t <degrees_per_sec>")
                    continue
                degrees_per_sec = get_float(parts[1])
                radians_per_sec = math.radians(degrees_per_sec)
                message = format_command("turn", [str(radians_per_sec)])
            elif cmd == "c":
                if dribbler_on:
                    # No explicit dribbler-off command; "skick 0" turns it off.
                    message = format_command("skick", ["0"])
                    dribbler_on = False
                else:
                    message = format_command("catch", [])
                    dribbler_on = True
            elif cmd == "k":
                message = format_command("kick", [])
            else:
                print("Unknown command.")
                continue

            send_message(sock, message)
            time.sleep(SEND_DELAY_S)
    finally:
        sock.close()
        print("Done.")


if __name__ == "__main__":
    main()

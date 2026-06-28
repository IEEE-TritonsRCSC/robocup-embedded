import socket
import sys

ROBOT_IP = "192.168.68.50"
UDP_PORT = 10000


def get_local_ip() -> str:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.connect(("8.8.8.8", 80))
        return sock.getsockname()[0]
    finally:
        sock.close()


def send_command(message: str) -> None:
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
      sock.sendto(message.encode("utf-8"), (ROBOT_IP, UDP_PORT))


def main() -> int:
    print(f"Gateway IP: {get_local_ip()}")
    print(f"Sending to: {ROBOT_IP}:{UDP_PORT}")

    if len(sys.argv) < 3:
        print("Usage:")
        print("  python send.py <robot_id> <command> [arg1] [arg2]")
        print("Examples:")
        print("  python send.py 1 q")
        print("  python send.py 1 c")
        print("  python send.py 1 d 1.0 30")
        print("  python send.py 1 t -90")
        return 1

    robot_id = sys.argv[1]
    command = sys.argv[2]
    args = sys.argv[3:]
    message = " ".join([robot_id, command] + args)

    print(f"UDP payload: {message}")
    send_command(message)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

import socket
import time
ARDUINO_IP = "100.105.19.66"
UDP_PORT   = 4210
MESSAGE    = "Hello Arduino!"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(MESSAGE.encode(), (ARDUINO_IP, UDP_PORT))
sock.close()

print(f"Sent '{MESSAGE}' to {ARDUINO_IP}:{UDP_PORT}")
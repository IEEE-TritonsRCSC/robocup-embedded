"""
This script sends a simple UDP packet to the Arduino Uno Q over WiFi
Wait for the Arduino to print "Ready" in the Serial Monitor before running the script
Troubleshooting:
- Check that both computer and Arduino are connected to the same WiFi
- Upload the code onto the Arduino Uno Q, then unplug it, wait 10 seconds, plug back in, and try again
- Make sure that the PORT matches in WiFiControl.h
- Sometimes the Arduino's COM port doesn't open. Just keep unplugging and replugging until it connects
"""

import socket
import time
ARDUINO_IP = "100.110.254.74"
PORT   = 4210

x = 1

while x <= 1:
   MESSAGE = f"1 s 100.0f"
   sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   sock.sendto(MESSAGE.encode(), (ARDUINO_IP, PORT))
   sock.close()

   print(f"Sent '{MESSAGE}' to {ARDUINO_IP}:{PORT} {x}")
   x += 1
   time.sleep(1)
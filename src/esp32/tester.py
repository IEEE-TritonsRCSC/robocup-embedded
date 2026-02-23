import math
import time
import socket

ROBOT_ID = 1
COMMAND_IP = "239.42.42.42"
COMMAND_PORT = 11000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print("The commands are as follows:")
print("  t <angle>       -> turn to angle (in degrees/second)")
print("  d <speed> <dir> -> dash with speed in direction (dir in degrees between -180 and 180)")
print("  s <power>       -> skick (dribbler pulse; power ignored by STM32)")
print("  k               -> kick")
print("  c               -> catch")
print("  stop            -> stop all motors")

short_to_long = {
    't': 'turn',
    'd': 'dash',
    's': 'skick',
    'k': 'kick',
    'c': 'catch'
}

def get_params(command, n_params):
    try:
        params = command.split()[1:1+n_params]
        if len(params) != n_params:
            raise ValueError
        for i in range(n_params):
            params[i] = str(float(params[i]))
        return params
    except ValueError:
        print(f"Invalid command. Please enter {n_params} parameters.")
        return None

try:
    while True:
        command = input("Enter command (or 'q' to quit): ")
        if command.lower() == 'q':
            raise KeyboardInterrupt
        if command.strip().lower() == 'stop':
            sock.sendto(b"stop\0", (COMMAND_IP, COMMAND_PORT))
            time.sleep(0.5)
            continue
        first_char = command[0] if command else ''
        if first_char == 't':
            params = get_params(command, 1)
            if params is None:
                continue
            params[0] = str(math.radians(float(params[0])))
        elif first_char == 'd':
            params = get_params(command, 2)
            if params is None:
                continue
            params[1] = str(math.radians(float(params[1])))
        elif first_char == 's':
            params = get_params(command, 1)
            if params is None:
                continue
        elif first_char in ['k', 'c']:
            params = []
        else:
            print("Unknown command. Please try again.")
            continue
        message = f"{ROBOT_ID} {short_to_long[first_char]} {' '.join(params)}\0"
        sock.sendto(message.encode(), (COMMAND_IP, COMMAND_PORT))
        time.sleep(0.5)

except KeyboardInterrupt:
    stop_command = b"stop\0"
    print("\nSending stop command and exiting...")
    sock.sendto(stop_command, (COMMAND_IP, COMMAND_PORT))
    time.sleep(0.5)
    sock.sendto(stop_command, (COMMAND_IP, COMMAND_PORT))
    time.sleep(0.5)
    sock.close()
    print("Done.")

import math
import socket
import pygame

# -------------------- CONFIG --------------------
STICK_DEADZONE = 0.12

MAX_POWER = 100.0
MAX_TURN_SPEED = 5.0  # rad/s

COMMAND_IP = "239.42.42.42"
COMMAND_PORT = 10000

ROBOT_ID = 1

# -------------------- SETUP --------------------
pygame.init()
pygame.joystick.init()

js = pygame.joystick.Joystick(1)
js.init()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Controller: {js.get_name()}\n")
clock = pygame.time.Clock()

# -------------------- HELPERS --------------------
def send_command(cmd):
    if cmd == "stop":
        cmd_str = cmd + '\0'
    else:
        cmd_str = str(ROBOT_ID) + ' ' + cmd + '\0'
    sock.sendto(cmd_str.encode(), (COMMAND_IP, COMMAND_PORT))

def get_stick_values(x, y):
    distance = math.sqrt(x**2 + y**2)

    if distance < STICK_DEADZONE:
        return 0, 0

    distance = min(distance, 1.0)
    power = distance * MAX_POWER

    angle = math.atan2(x, -y)
    return power, angle

def get_turn_speed(rx):
    if abs(rx) < STICK_DEADZONE:
        return 0
    return rx * MAX_TURN_SPEED

# -------------------- MAIN LOOP --------------------
print("Controls:")
print("  Left Stick  - Move (dash)")
print("  Right Stick - Turn")
print("  X           - Kick")
print("  Square      - Catch")
print("  Circle      - Short Kick")
print("  Triangle    - Stop")
print()

while True:
    pygame.event.pump()

    # Axes (values are -1.0 to 1.0)
    left_x = js.get_axis(0)
    left_y = js.get_axis(1)
    right_x = js.get_axis(2)

    power, angle = get_stick_values(left_x, left_y)
    turn = get_turn_speed(right_x)

    if power > 0:
        print(power, angle)
        send_command(f"dash {power:.1f} {angle:.3f}")
    elif turn != 0:
        print(turn)
        send_command(f"turn {turn:.2f}")

    # Buttons
    if js.get_button(0):  # X
        send_command("kick")
        print("x")
    elif js.get_button(2):  # Square
        send_command("catch")
        print("square")
    elif js.get_button(1):  # Circle
        print("circle")
        send_command(f"skick {100.0}")
    elif js.get_button(3):  # Triangle
        print(f"triangle")
        send_command("stop")

    clock.tick(50)  # 50 Hz (good for robotics)
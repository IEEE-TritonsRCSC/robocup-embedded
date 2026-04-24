import math
import socket
from evdev import InputDevice, ecodes

# -------------------- CONFIG --------------------
GAMEPAD_PATH = '/dev/input/event15'

STICK_CENTER = 128
STICK_MAX = 127
DEADZONE = 15

MAX_POWER = 100.0
MAX_TURN_SPEED = 5.0  # rad/s

COMMAND_IP = "239.42.42.42"
COMMAND_PORT = 10000

ROBOT_ID = 1

# -------------------- SETUP --------------------
gamepad = InputDevice(GAMEPAD_PATH)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print(f"Controller: {gamepad.name}\n\n\n\n")

# -------------------- STATE --------------------
left_x = STICK_CENTER
left_y = STICK_CENTER
right_x = STICK_CENTER

# -------------------- HELPERS --------------------
def send_command(cmd):
    if cmd == "stop":
        cmd_str = cmd + '\0'
    else:
        cmd_str = str(ROBOT_ID) + ' ' +cmd + '\0'
    sock.sendto(cmd_str.encode(), (COMMAND_IP, COMMAND_PORT))


def get_stick_values(x, y):
    """Returns (power, angle_radians) or (0, 0) if in deadzone"""
    nx = (x - STICK_CENTER) / STICK_MAX
    ny = (y - STICK_CENTER) / STICK_MAX
    
    distance = math.sqrt(nx**2 + ny**2)
    
    if distance < DEADZONE / STICK_MAX:
        return 0, 0
    
    # Clamp distance to 1.0
    distance = min(distance, 1.0)
    power = distance * MAX_POWER
    
    # atan2(x, -y) because Y is inverted and we want 0 = forward
    angle = math.atan2(nx, -ny)
    
    return power, angle

def get_turn_speed(rx):
    """Returns angular speed from right stick X"""
    nx = (rx - STICK_CENTER) / STICK_MAX
    
    if abs(nx) < DEADZONE / STICK_MAX:
        return 0
    
    return nx * MAX_TURN_SPEED

def update_movement():
    """Send dash or turn command based on current stick state"""
    power, angle = get_stick_values(left_x, left_y)
    turn = get_turn_speed(right_x)
    
    if power > 0:
        send_command(f"dash {power:.1f} {angle:.3f}")
    elif turn != 0:
        send_command(f"turn {turn:.2f}")

# -------------------- MAIN LOOP --------------------
print("Controls:")
print("  Left Stick  - Move (dash)")
print("  Right Stick - Turn")
print("  X (Cross)   - Kick")
print("  Square      - Catch/Dribble")
print("  Circle      - Short Kick")
print("  Triangle    - Stop")
print()

for event in gamepad.read_loop():
    
    # Analog sticks
    if event.type == ecodes.EV_ABS:
        if event.code == ecodes.ABS_X:
            left_x = event.value
            update_movement()
        elif event.code == ecodes.ABS_Y:
            left_y = event.value
            update_movement()
        elif event.code == ecodes.ABS_RX:
            right_x = event.value
            update_movement()
    
    # Buttons
    elif event.type == ecodes.EV_KEY and event.value == 1:  # Button press (not release)
        if event.code == ecodes.BTN_SOUTH:      # X / Cross
            send_command("kick")
        elif event.code == ecodes.BTN_WEST:     # Square
            send_command("catch")
        elif event.code == ecodes.BTN_EAST:     # Circle
            send_command(f"skick {100.0}")       # adjust power as needed
        elif event.code == ecodes.BTN_NORTH:    # Triangle
            send_command("stop")
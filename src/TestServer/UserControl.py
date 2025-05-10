import socket, time, math, struct, threading
from pynput import keyboard
from messages_robocup_ssl_detection_pb2 import SSL_DetectionRobot
from ssl_simulation_robot_control_pb2 import (
    RobotCommand, 
    RobotMoveCommand, 
    MoveWheelVelocity, 
    MoveLocalVelocity, 
    MoveGlobalVelocity
)
from triton_bot_communication_pb2 import TritonBotMessage

# Create and populate the TritonBotMessage
message = TritonBotMessage()
message.id = 2

message.vision.confidence = 0.8
message.vision.robot_id = 1
message.vision.x = 1.0
message.vision.y = 1.0
message.vision.orientation = math.pi
message.vision.pixel_x = 1.0
message.vision.pixel_y = 1.0
message.vision.height = 1.0

message.command.id = 1
message.command.move_command.local_velocity.forward = 0
message.command.move_command.local_velocity.left = 0 
message.command.move_command.local_velocity.angular = 0
message.command.kick_speed = 0
message.command.kick_angle = 0
message.command.dribbler_speed = 0

UDP_IP = '192.168.8.80' # Replace with the ESP32's IP address
UDP_PORT = 3333

MULTICAST_GROUP = '224.1.1.1'
PORT = 10500

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
ttl = struct.pack('b', 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

# Dictionary to keep track of key states
key_states = {'w': False, # move forward
              'a': False, # move left
              's': False, # move down
              'd': False, # move right
              'j':False,  # rotate ccw 
              'l':False,  # rotate cw
              'k':False,  # kick
              'b':False,  # dribble
}

def on_press(key):
    try:
        if key.char in key_states:
            key_states[key.char] = True
        
    except AttributeError:
        pass

def on_release(key):
    try:
        if key.char in key_states:
            key_states[key.char] = False
            # print(f"{key_states}")
    except AttributeError:
        pass
    if key == keyboard.Key.esc:
            # Stop listener
            return False
    
listener = keyboard.Listener(on_press=on_press, on_release=on_release) 
listener.start()

while(True):
    message.command.move_command.local_velocity.forward = 0
    message.command.move_command.local_velocity.left = 0 
    message.command.move_command.local_velocity.angular = 0
    message.command.kick_speed = 0
    message.command.dribbler_speed = 0

    if (key_states['w']):
        message.command.move_command.local_velocity.forward = 3
    
    if (key_states['s']):
        message.command.move_command.local_velocity.forward = -3

    if (key_states['a']):
        message.command.move_command.local_velocity.left = -3

    if (key_states['d']):
        message.command.move_command.local_velocity.left = 3

    if (key_states['j']):
        message.command.move_command.local_velocity.angular = 25

    if (key_states['l']):
        message.command.move_command.local_velocity.angular = -25

    if (key_states['k']):
        message.command.kick_speed = 1
        key_states['k'] = False

    if (key_states['b']):
        message.command.dribbler_speed = 1

    data = message.SerializeToString()
    sock.sendto(data, (MULTICAST_GROUP, PORT))
    
    print(key_states)

    time.sleep(0.1)

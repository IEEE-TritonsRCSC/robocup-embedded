import socket, time, math
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
message.id = 1

message.vision.confidence = 0.8
message.vision.robot_id = 1
message.vision.x = 1.0
message.vision.y = 1.0
message.vision.orientation = math.pi
message.vision.pixel_x = 1.0
message.vision.pixel_y = 1.0
message.vision.height = 1.0

message.command.id = 1
message.command.move_command.local_velocity.forward = 1.0
message.command.move_command.local_velocity.left = 1.0
message.command.move_command.local_velocity.angular = math.pi/4.0
message.command.kick_speed = 1.0
message.command.kick_angle = math.pi/2.0
message.command.dribbler_speed = 1.0

UDP_IP = '192.168.8.80' # Replace with the ESP32's IP address

UDP_PORT = 3333

def main():
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Serialize the data
    data = message.SerializeToString()

    try:
        while True:
            # Send data
            sock.sendto(data, (UDP_IP, UDP_PORT))
            print(f'Sent data, id = {message.id}')

            time.sleep(1)
    
    except KeyboardInterrupt:
        print('\nStopping sender.')

    finally:
        sock.close()

if __name__ == '__main__':
    main()

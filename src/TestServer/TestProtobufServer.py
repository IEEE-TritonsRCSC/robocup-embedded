import socket, time, math, struct
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
message.command.move_command.local_velocity.forward = 0
message.command.move_command.local_velocity.left = 0 
message.command.move_command.local_velocity.angular = 0
message.command.kick_speed = 10
message.command.kick_angle = 0
message.command.dribbler_speed = 0

UDP_IP = '192.168.8.80' # Replace with the ESP32's IP address
UDP_PORT = 3333

MULTICAST_GROUP = '224.1.1.1'
PORT = 10500


def main():
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    ttl = struct.pack('b', 1)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

    # Serialize the data
    data = message.SerializeToString()

    count = 0

    try:
        while True:
            # Send data
            sock.sendto(data, (MULTICAST_GROUP, PORT))

            if (message.command.kick_speed > 0):
                message.command.kick_speed = 0;

            print(f'Sent data, velocity = {message.command.move_command.local_velocity.angular}')

            if (count % 5 == 0):
                if (message.command.move_command.local_velocity.angular != 0):
                    message.command.move_command.local_velocity.angular = 0    
                    data = message.SerializeToString()
                else:
                   message.command.move_command.local_velocity.angular = 12 
                   message.command.kick_speed = 10;
                   data = message.SerializeToString()

            count += 1 
            time.sleep(1)

       
    
    except KeyboardInterrupt:
        print('\nStopping sender.')

    finally:
        sock.close()

if __name__ == '__main__':
    main()

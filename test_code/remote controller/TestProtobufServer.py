import socket, time, math



UDP_IP = '192.168.8.80' # Replace with the ESP32's IP address

UDP_PORT = 3333
PORT = 10500
def main():
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Serialize the data

    try:
        while True:
            # Send data
            input1 = input()
            for i in range(0,3):
                input1 += input1
            sock.sendto(input1.encode(), (UDP_IP, UDP_PORT))
            print("sent data")

    
    except KeyboardInterrupt:
        print('\nStopping sender.')

    finally:
        sock.close()

if __name__ == '__main__':
    main()

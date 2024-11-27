import socket
import time
from data_pb2 import SensorData  # Import the generated class

UDP_IP = "192.168.137.205"  # Replace with the ESP32's IP address
UDP_PORT = 3333

def main():
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Create and populate the SensorData object
    sensor_data = SensorData()
    sensor_data.id = 1
    sensor_data.temperature = 25.5
    sensor_data.humidity = 60.0
    
    # Serialize the data
    serialized_data = sensor_data.SerializeToString()
    
    try:
        while True:
            # Send the serialized Protocol Buffer message
            sock.sendto(serialized_data, (UDP_IP, UDP_PORT))
            print(f"Sent: ID={sensor_data.id}, Temperature={sensor_data.temperature}, Humidity={sensor_data.humidity}")
            time.sleep(1)  # Add delay to avoid flooding
    except KeyboardInterrupt:
        print("\nStopping sender.")
    finally:
        sock.close()

if __name__ == "__main__":
    main()

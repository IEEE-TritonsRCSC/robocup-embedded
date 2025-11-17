#!/usr/bin/env python3
"""
Real-time PID tuning visualization tool
Receives telemetry from ESP32 and plots target vs actual speeds
"""
import socket
import struct
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np
import time

# Configuration
MCAST_GRP = "239.42.42.42"
TELEMETRY_PORT = 10001
MAX_POINTS = 200  # Show last 20 seconds of data at 10Hz

def get_local_ip():
    """Auto-detect the local IP on the robot's network (192.168.8.x)"""
    try:
        hostname = socket.gethostname()
        addrs = socket.getaddrinfo(hostname, None)
        for addr in addrs:
            ip = addr[4][0]
            if ip.startswith("192.168.8."):
                return ip
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except:
        return None

class PIDPlotter:
    def __init__(self):
        # Data storage (timestamps, targets, actuals, outputs for 4 wheels)
        self.timestamps = deque(maxlen=MAX_POINTS)
        self.targets = [deque(maxlen=MAX_POINTS) for _ in range(4)]
        self.actuals = [deque(maxlen=MAX_POINTS) for _ in range(4)]
        self.outputs = [deque(maxlen=MAX_POINTS) for _ in range(4)]
        self.errors = [deque(maxlen=MAX_POINTS) for _ in range(4)]
        
        self.start_time = time.time()
        self.last_timestamp = 0
        
        # Detect WiFi interface
        local_ip = get_local_ip()
        if local_ip:
            print(f"🔍 Using network interface: {local_ip}")
        else:
            print("⚠️  Could not detect interface, using system default")
            local_ip = socket.INADDR_ANY
        
        # UDP socket with multicast support
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('', TELEMETRY_PORT))
        
        # Join multicast group on specific interface
        mcast_group = socket.inet_aton(MCAST_GRP)
        if isinstance(local_ip, str):
            local_iface = socket.inet_aton(local_ip)
        else:
            local_iface = struct.pack('I', socket.INADDR_ANY)
        mreq = struct.pack('4s4s', mcast_group, local_iface)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        
        self.sock.settimeout(0.01)  # Non-blocking with 10ms timeout
        
        # Create figure with subplots
        self.fig, self.axes = plt.subplots(2, 2, figsize=(14, 10))
        self.fig.suptitle('Real-Time PID Tuning Monitor', fontsize=16, fontweight='bold')
        
        self.wheel_names = ['Wheel 0 (FR)', 'Wheel 1 (BR)', 'Wheel 2 (BL)', 'Wheel 3 (FL)']
        self.lines_target = []
        self.lines_actual = []
        self.lines_error = []
        
        for i, ax in enumerate(self.axes.flat):
            ax.set_title(self.wheel_names[i], fontsize=12, fontweight='bold')
            ax.set_xlabel('Time (s)')
            ax.set_ylabel('Speed (RPM)')
            ax.grid(True, alpha=0.3)
            
            # Plot lines
            line_target, = ax.plot([], [], 'b--', label='Target', linewidth=2)
            line_actual, = ax.plot([], [], 'g-', label='Actual', linewidth=2)
            line_error, = ax.plot([], [], 'r:', label='Error', linewidth=1, alpha=0.7)
            
            self.lines_target.append(line_target)
            self.lines_actual.append(line_actual)
            self.lines_error.append(line_error)
            
            ax.legend(loc='upper right', fontsize=8)
            ax.set_ylim(-2000, 2000)
        
        plt.tight_layout()
        
        print("🎬 Real-Time PID Monitor Started")
        print(f"📡 Listening for telemetry on UDP port {TELEMETRY_PORT}")
        print("💡 Send commands with wasd_teleop.py to see data")
        print("=" * 60)
    
    def parse_telemetry(self, data):
        """Parse telemetry packet from STM32"""
        if len(data) < 32:
            return False
        
        # Check header
        if data[0] != 0xFE or data[1] != 0xED:
            return False
        
        # Parse timestamp (4 bytes, big-endian)
        timestamp_ms = struct.unpack('>I', data[2:6])[0]
        
        # Calculate relative time (once per packet)
        rel_time = (time.time() - self.start_time)
        
        # Only process if this is a new timestamp
        if not self.timestamps or rel_time > self.timestamps[-1]:
            self.timestamps.append(rel_time)
            
            # Parse wheel data (4 wheels × 6 bytes each)
            for i in range(4):
                idx = 6 + (i * 6)
                target = struct.unpack('>h', data[idx:idx+2])[0]  # signed int16
                actual = struct.unpack('>h', data[idx+2:idx+4])[0]
                output = struct.unpack('>h', data[idx+4:idx+6])[0]
                
                # Store data for this wheel
                self.targets[i].append(target)
                self.actuals[i].append(actual)
                self.outputs[i].append(output)
                self.errors[i].append(target - actual)
        
        self.last_timestamp = timestamp_ms
        return True
    
    def update_plot(self, frame):
        """Animation update function"""
        # Try to read UDP packets
        packets_received = 0
        try:
            while packets_received < 10:  # Process up to 10 packets per frame
                data, addr = self.sock.recvfrom(1024)
                if frame == 0 and packets_received == 0:  # Debug first packet
                    print(f"📦 Received {len(data)} bytes from {addr}: {' '.join(f'{b:02X}' for b in data[:16])}")
                if self.parse_telemetry(data):
                    packets_received += 1
                    if packets_received == 1 and len(self.timestamps) == 1:  # First successful parse
                        print(f"✅ First telemetry packet parsed successfully!")
        except socket.timeout:
            pass
        
        # Update plots
        if len(self.timestamps) > 0:
            times = np.array(self.timestamps)
            
            for i in range(4):
                if len(self.targets[i]) > 0:
                    targets = np.array(self.targets[i])
                    actuals = np.array(self.actuals[i])
                    errors = np.array(self.errors[i])
                    
                    # Update line data
                    self.lines_target[i].set_data(times, targets)
                    self.lines_actual[i].set_data(times, actuals)
                    self.lines_error[i].set_data(times, errors)
                    
                    # Auto-scale x-axis
                    ax = self.axes.flat[i]
                    if len(times) > 1:
                        ax.set_xlim(max(0, times[-1] - 20), times[-1] + 1)  # Show last 20 seconds
                    
                    # Auto-scale y-axis based on data
                    all_values = np.concatenate([targets, actuals])
                    if len(all_values) > 0:
                        y_min = min(-100, np.min(all_values) - 100)
                        y_max = max(100, np.max(all_values) + 100)
                        ax.set_ylim(y_min, y_max)
                    
                    # Update title with current error
                    if len(errors) > 0:
                        current_error = errors[-1]
                        error_pct = (abs(current_error) / max(1, abs(targets[-1]))) * 100 if targets[-1] != 0 else 0
                        ax.set_title(f"{self.wheel_names[i]} | Error: {current_error:.0f} RPM ({error_pct:.1f}%)", 
                                   fontsize=10, fontweight='bold')
        
        return self.lines_target + self.lines_actual + self.lines_error
    
    def run(self):
        """Start the animation"""
        ani = animation.FuncAnimation(
            self.fig, 
            self.update_plot, 
            interval=50,  # Update every 50ms (20 FPS)
            blit=False,
            cache_frame_data=False
        )
        plt.show()

def main():
    plotter = PIDPlotter()
    plotter.run()

if __name__ == "__main__":
    main()

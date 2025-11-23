#!/usr/bin/env python3
"""
Real-time PID Plotter
Receives telemetry from robot and plots target vs actual vs output
"""

import socket
import struct
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import argparse
import numpy as np

TELEMETRY_PORT = 10001
MULTICAST_IP = "239.42.42.42"

class PIDPlotter:
    def __init__(self, max_points=500, wheel_idx=0):
        self.max_points = max_points
        self.wheel_idx = wheel_idx
        
        # Data storage
        self.timestamps = deque(maxlen=max_points)
        self.targets = deque(maxlen=max_points)
        self.actuals = deque(maxlen=max_points)
        self.outputs = deque(maxlen=max_points)
        self.errors = deque(maxlen=max_points)
        
        # Setup UDP socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('', TELEMETRY_PORT))
        
        # Join multicast group
        mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.sock.settimeout(0.01)  # Non-blocking
        
        # Setup plot
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(12, 8))
        self.fig.suptitle(f'PID Response - Wheel {wheel_idx}', fontsize=16)
        
        # Top plot: Target vs Actual
        self.line_target, = self.ax1.plot([], [], 'r-', label='Target', linewidth=2)
        self.line_actual, = self.ax1.plot([], [], 'b-', label='Actual', linewidth=2)
        self.ax1.set_ylabel('Speed')
        self.ax1.set_xlabel('Time (s)')
        self.ax1.legend(loc='upper right')
        self.ax1.grid(True, alpha=0.3)
        
        # Bottom plot: Error and Output
        self.line_error, = self.ax2.plot([], [], 'g-', label='Error', linewidth=1.5)
        self.line_output, = self.ax2.plot([], [], 'm-', label='PID Output', linewidth=1.5, alpha=0.7)
        self.ax2.set_ylabel('Error / Output')
        self.ax2.set_xlabel('Time (s)')
        self.ax2.legend(loc='upper right')
        self.ax2.grid(True, alpha=0.3)
        
        print(f"📊 PID Plotter started for Wheel {wheel_idx}")
        print(f"   Listening on {MULTICAST_IP}:{TELEMETRY_PORT}")
        print("   Waiting for telemetry data...")
        print("\n💡 Tips:")
        print("   - Red line = Target speed (what you commanded)")
        print("   - Blue line = Actual speed (motor feedback)")
        print("   - Green line = Error (target - actual)")
        print("   - Purple line = PID output (control signal)")
        print("\nPress Ctrl+C to exit")
    
    def parse_telemetry(self, data):
        """Parse telemetry packet from robot"""
        if len(data) < 32:
            return None
        
        # Check header
        if data[0] != 0xFE or data[1] != 0xED:
            return None
        
        # Parse timestamp (4 bytes, big-endian)
        timestamp_ms = struct.unpack('>I', data[2:6])[0]
        
        # Parse wheel data (6 bytes per wheel: target, actual, output)
        wheel_data = []
        for i in range(4):
            idx = 6 + (i * 6)
            target = struct.unpack('>h', data[idx:idx+2])[0]  # signed int16
            actual = struct.unpack('>h', data[idx+2:idx+4])[0]
            output = struct.unpack('>h', data[idx+4:idx+6])[0]
            wheel_data.append({
                'target': target,
                'actual': actual,
                'output': output,
                'error': target - actual
            })
        
        return {
            'timestamp': timestamp_ms / 1000.0,  # Convert to seconds
            'wheels': wheel_data
        }
    
    def update(self, frame):
        """Animation update function"""
        # Read all available packets
        try:
            while True:
                data, _ = self.sock.recvfrom(1024)
                telemetry = self.parse_telemetry(data)
                
                if telemetry:
                    wheel = telemetry['wheels'][self.wheel_idx]
                    
                    self.timestamps.append(telemetry['timestamp'])
                    self.targets.append(wheel['target'])
                    self.actuals.append(wheel['actual'])
                    self.outputs.append(wheel['output'])
                    self.errors.append(wheel['error'])
        except socket.timeout:
            pass
        
        if len(self.timestamps) < 2:
            return self.line_target, self.line_actual, self.line_error, self.line_output
        
        # Convert to numpy arrays
        times = np.array(self.timestamps)
        times = times - times[0]  # Relative time
        
        # Update top plot (Target vs Actual)
        self.line_target.set_data(times, self.targets)
        self.line_actual.set_data(times, self.actuals)
        
        # Update bottom plot (Error and Output)
        self.line_error.set_data(times, self.errors)
        self.line_output.set_data(times, self.outputs)
        
        # Auto-scale
        if len(times) > 0:
            for ax in [self.ax1, self.ax2]:
                ax.relim()
                ax.autoscale_view()
        
        # Add statistics text
        if len(self.actuals) > 10:
            recent_error = np.array(list(self.errors)[-50:])
            mean_error = np.mean(recent_error)
            std_error = np.std(recent_error)
            
            stats_text = f"Recent Error: μ={mean_error:.2f}, σ={std_error:.2f}"
            self.ax1.set_title(stats_text, fontsize=10, color='gray')
        
        return self.line_target, self.line_actual, self.line_error, self.line_output
    
    def run(self):
        """Start the animation"""
        ani = animation.FuncAnimation(
            self.fig, self.update, interval=50, blit=True, cache_frame_data=False
        )
        plt.tight_layout()
        plt.show()

def main():
    parser = argparse.ArgumentParser(description="Real-time PID plotter")
    parser.add_argument('--wheel', type=int, default=0, choices=[0, 1, 2, 3],
                        help='Wheel to plot (0=FR, 1=BR, 2=BL, 3=FL)')
    parser.add_argument('--points', type=int, default=500,
                        help='Max data points to display')
    args = parser.parse_args()
    
    wheel_names = ['Front-Right', 'Back-Right', 'Back-Left', 'Front-Left']
    print(f"\n🤖 Plotting wheel {args.wheel} ({wheel_names[args.wheel]})")
    
    plotter = PIDPlotter(max_points=args.points, wheel_idx=args.wheel)
    
    try:
        plotter.run()
    except KeyboardInterrupt:
        print("\n\n👋 Exiting...")

if __name__ == "__main__":
    main()

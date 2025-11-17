#!/usr/bin/env python3
"""
Manual PID Tuner - Simple interactive tool
You control when to test, system shows you the response
"""

import socket
import struct
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

TELEMETRY_PORT = 10001
MULTICAST_IP = "239.42.42.42"
COMMAND_PORT = 10000
ROBOT_ID = 1

class ManualPIDTuner:
    def __init__(self, wheel_idx=0):
        self.wheel_idx = wheel_idx
        
        # Setup sockets
        self.recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.recv_sock.bind(('', TELEMETRY_PORT))
        
        mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
        self.recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.recv_sock.settimeout(0.1)
        
        self.send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Current PID
        self.kp = 0.5
        self.ki = 0.05
        self.kd = 0.0
        
        # Data for plotting
        self.times = []
        self.targets = []
        self.actuals = []
        self.max_points = 300  # 30 seconds at 10Hz
        
        print("🎮 Manual PID Tuner")
        print("="*60)
        print(f"Tuning wheel {wheel_idx}")
        print(f"Current PID: Kp={self.kp}, Ki={self.ki}, Kd={self.kd}")
        print("="*60)
    
    def parse_telemetry(self, data):
        if len(data) < 32 or data[0] != 0xFE or data[1] != 0xED:
            return None
        
        timestamp_ms = struct.unpack('>I', data[2:6])[0]
        wheels = []
        for i in range(4):
            idx = 6 + (i * 6)
            target = struct.unpack('>h', data[idx:idx+2])[0]
            actual = struct.unpack('>h', data[idx+2:idx+4])[0]
            output = struct.unpack('>h', data[idx+4:idx+6])[0]
            wheels.append({'target': target, 'actual': actual, 'output': output})
        
        return {'timestamp': timestamp_ms / 1000.0, 'wheels': wheels}
    
    def send_command(self, cmd):
        self.send_sock.sendto(cmd.encode(), (MULTICAST_IP, COMMAND_PORT))
        print(f"   → {cmd}")
    
    def set_pid(self, kp, ki, kd):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        kp_q = int(kp * 1000)
        ki_q = int(ki * 1000)
        kd_q = int(kd * 1000)
        cmd = f"{ROBOT_ID} pidu {self.wheel_idx} {kp_q} {ki_q} {kd_q}"
        self.send_command(cmd)
    
    def stop(self):
        self.send_command(f"{ROBOT_ID} move 0 0 0")
    
    def move(self, speed):
        self.send_command(f"{ROBOT_ID} move {speed} 0 0")
    
    def update_plot(self, frame):
        """Called by matplotlib to update plot"""
        try:
            data, _ = self.recv_sock.recvfrom(1024)
            telemetry = self.parse_telemetry(data)
            if telemetry:
                wheel = telemetry['wheels'][self.wheel_idx]
                
                self.times.append(telemetry['timestamp'])
                self.targets.append(wheel['target'])
                self.actuals.append(wheel['actual'])
                
                # Keep only recent data
                if len(self.times) > self.max_points:
                    self.times.pop(0)
                    self.targets.pop(0)
                    self.actuals.pop(0)
        except socket.timeout:
            pass
        
        # Update plot
        if len(self.times) > 1:
            # Normalize time
            t0 = self.times[0]
            times_norm = [t - t0 for t in self.times]
            
            self.ax.clear()
            self.ax.plot(times_norm, self.targets, 'r--', label='Target', linewidth=2)
            self.ax.plot(times_norm, self.actuals, 'b-', label='Actual', linewidth=2)
            self.ax.set_xlabel('Time (s)')
            self.ax.set_ylabel('Speed')
            self.ax.set_title(f'Wheel {self.wheel_idx} - Kp={self.kp:.3f}, Ki={self.ki:.3f}, Kd={self.kd:.3f}')
            self.ax.legend()
            self.ax.grid(True, alpha=0.3)
    
    def run_live_plot(self):
        """Start live plotting"""
        print("\n📊 Starting live plot...")
        print("Use the terminal to send commands while watching the plot")
        
        self.fig, self.ax = plt.subplots(figsize=(12, 6))
        ani = FuncAnimation(self.fig, self.update_plot, interval=100, cache_frame_data=False)
        plt.show()

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Manual PID Tuner")
    parser.add_argument('--wheel', type=int, default=0, choices=[0, 1, 2, 3])
    args = parser.parse_args()
    
    tuner = ManualPIDTuner(wheel_idx=args.wheel)
    
    print("\n📋 Commands:")
    print("  p <value>     - Set Kp (e.g., 'p 0.7')")
    print("  i <value>     - Set Ki (e.g., 'i 0.1')")
    print("  d <value>     - Set Kd (e.g., 'd 0.01')")
    print("  m <speed>     - Move (e.g., 'm 1.0')")
    print("  s             - Stop")
    print("  plot          - Start live plotting")
    print("  quit          - Exit")
    print()
    
    try:
        while True:
            cmd = input(">>> ").strip().lower()
            
            if cmd == 'quit' or cmd == 'q':
                tuner.stop()
                break
            
            elif cmd == 'plot':
                tuner.run_live_plot()
            
            elif cmd == 's' or cmd == 'stop':
                tuner.stop()
            
            elif cmd.startswith('p '):
                try:
                    val = float(cmd.split()[1])
                    tuner.set_pid(val, tuner.ki, tuner.kd)
                    print(f"✓ Set Kp={val}")
                except:
                    print("❌ Invalid value")
            
            elif cmd.startswith('i '):
                try:
                    val = float(cmd.split()[1])
                    tuner.set_pid(tuner.kp, val, tuner.kd)
                    print(f"✓ Set Ki={val}")
                except:
                    print("❌ Invalid value")
            
            elif cmd.startswith('d '):
                try:
                    val = float(cmd.split()[1])
                    tuner.set_pid(tuner.kp, tuner.ki, val)
                    print(f"✓ Set Kd={val}")
                except:
                    print("❌ Invalid value")
            
            elif cmd.startswith('m '):
                try:
                    speed = float(cmd.split()[1])
                    tuner.move(speed)
                except:
                    print("❌ Invalid speed")
            
            else:
                print("❌ Unknown command")
    
    except KeyboardInterrupt:
        print("\n👋 Stopping...")
        tuner.stop()

if __name__ == "__main__":
    main()

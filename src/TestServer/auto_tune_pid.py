#!/usr/bin/env python3
"""
Automated PID Auto-Tuner
Collects data, analyzes response, and suggests/applies PID adjustments automatically
"""

import socket
import struct
import time
import numpy as np
import argparse
import json
from datetime import datetime

TELEMETRY_PORT = 10001
MULTICAST_IP = "239.42.42.42"
COMMAND_PORT = 10000
ROBOT_ID = 1

class PIDAutoTuner:
    def __init__(self, wheel_idx=0, auto_apply=False):
        self.wheel_idx = wheel_idx
        self.auto_apply = auto_apply
        
        # Setup sockets
        self.recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.recv_sock.bind(('', TELEMETRY_PORT))
        
        mreq = struct.pack("4sl", socket.inet_aton(MULTICAST_IP), socket.INADDR_ANY)
        self.recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.recv_sock.settimeout(0.5)
        
        self.send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Current PID values
        self.kp = 0.5
        self.ki = 0.05
        self.kd = 0.0
        
        # Data storage
        self.data = []
        
        print(f"🤖 PID Auto-Tuner for Wheel {wheel_idx}")
        print(f"   Auto-apply mode: {auto_apply}")
        print("="*60)
    
    def parse_telemetry(self, data):
        """Parse telemetry packet"""
        if len(data) < 32 or data[0] != 0xFE or data[1] != 0xED:
            return None
        
        timestamp_ms = struct.unpack('>I', data[2:6])[0]
        
        wheel_data = []
        for i in range(4):
            idx = 6 + (i * 6)
            target = struct.unpack('>h', data[idx:idx+2])[0]
            actual = struct.unpack('>h', data[idx+2:idx+4])[0]
            output = struct.unpack('>h', data[idx+4:idx+6])[0]
            wheel_data.append({
                'target': target,
                'actual': actual,
                'output': output,
                'error': target - actual
            })
        
        return {
            'timestamp': timestamp_ms / 1000.0,
            'wheels': wheel_data
        }
    
    def send_command(self, cmd):
        """Send command to robot"""
        self.send_sock.sendto(cmd.encode(), (MULTICAST_IP, COMMAND_PORT))
    
    def set_pid(self, kp, ki, kd):
        """Set PID values for the wheel"""
        kp_q = int(kp * 1000)
        ki_q = int(ki * 1000)
        kd_q = int(kd * 1000)
        cmd = f"{ROBOT_ID} pidu {self.wheel_idx} {kp_q} {ki_q} {kd_q}"
        self.send_command(cmd)
        self.kp = kp
        self.ki = ki
        self.kd = kd
        print(f"   Set PID: Kp={kp:.3f}, Ki={ki:.3f}, Kd={kd:.3f}")
        time.sleep(0.2)
    
    def send_test_command(self, speed=1.0, duration=3.0):
        """Send movement command and collect data"""
        print(f"\n📊 Running test: speed={speed}, duration={duration}s")
        
        # Clear old data
        self.data = []
        
        # Stop first and wait for robot to settle
        print("   Stopping robot...")
        self.send_command(f"{ROBOT_ID} move 0 0 0")
        time.sleep(1.5)  # Longer wait for targets to reach zero
        
        # Start collecting FIRST (capture baseline zeros)
        print("   Pre-collecting baseline data...")
        start_time = time.time()
        baseline_time = 0.5  # Collect 0.5s of baseline
        packet_count = 0
        
        while (time.time() - start_time) < baseline_time:
            try:
                data, _ = self.recv_sock.recvfrom(1024)
                telemetry = self.parse_telemetry(data)
                if telemetry:
                    wheel = telemetry['wheels'][self.wheel_idx]
                    self.data.append({
                        'time': telemetry['timestamp'],
                        'target': wheel['target'],
                        'actual': wheel['actual'],
                        'error': wheel['error'],
                        'output': wheel['output']
                    })
                    packet_count += 1
            except socket.timeout:
                continue
        
        print(f"   Baseline: {packet_count} packets")
        
        # NOW send the move command while still collecting
        print(f"   Sending move command (speed={speed})...")
        self.send_command(f"{ROBOT_ID} move {speed} 0 0")
        
        # Continue collecting for the test duration
        start_time = time.time()
        while (time.time() - start_time) < duration:
            try:
                data, _ = self.recv_sock.recvfrom(1024)
                telemetry = self.parse_telemetry(data)
                if telemetry:
                    wheel = telemetry['wheels'][self.wheel_idx]
                    self.data.append({
                        'time': telemetry['timestamp'],
                        'target': wheel['target'],
                        'actual': wheel['actual'],
                        'error': wheel['error'],
                        'output': wheel['output']
                    })
                    packet_count += 1
                    if packet_count % 10 == 0:
                        print(f"   Collecting... {packet_count} packets", end='\r')
            except socket.timeout:
                continue
        
        print()  # New line after progress
        
        # Stop
        print("   Stopping robot...")
        self.send_command(f"{ROBOT_ID} move 0 0 0")
        time.sleep(0.5)
        
        print(f"   ✓ Collected {len(self.data)} data points")
        
        if len(self.data) > 0:
            targets = [d['target'] for d in self.data]
            actuals = [d['actual'] for d in self.data]
            print(f"   Target range: {min(targets)} to {max(targets)}")
            print(f"   Actual range: {min(actuals)} to {max(actuals)}")
        
        return len(self.data) > 10
    
    def analyze_response(self):
        """Analyze the collected data and diagnose issues"""
        if len(self.data) < 10:
            return None
        
        # Convert to numpy arrays
        times = np.array([d['time'] for d in self.data])
        targets = np.array([d['target'] for d in self.data])
        actuals = np.array([d['actual'] for d in self.data])
        errors = np.array([d['error'] for d in self.data])
        
        # Normalize time to start at 0
        times = times - times[0]
        
        # Find when target changes (step input)
        # Look for first non-zero target or significant change
        target_change_idx = 0
        
        # Method 1: Find first significant target value
        for i in range(len(targets)):
            if abs(targets[i]) > 10:
                target_change_idx = i
                break
        
        # Method 2: If no significant target, look for any change
        if target_change_idx == 0:
            for i in range(1, len(targets)):
                if abs(targets[i] - targets[i-1]) > 5:
                    target_change_idx = i
                    break
        
        if target_change_idx == 0:
            print(f"   ⚠️  No step input detected")
            print(f"   Target values: min={min(targets)}, max={max(targets)}, mean={np.mean(targets):.1f}")
            print(f"   This usually means the robot didn't respond to move command")
            return None
        
        # Analyze from step input onwards
        step_times = times[target_change_idx:]
        step_targets = targets[target_change_idx:]
        step_actuals = actuals[target_change_idx:]
        step_errors = errors[target_change_idx:]
        
        target_value = step_targets[0]
        
        # Calculate metrics
        metrics = {}
        
        # 1. Rise time (10% to 90% of target)
        try:
            idx_10 = np.where(step_actuals >= 0.1 * target_value)[0][0]
            idx_90 = np.where(step_actuals >= 0.9 * target_value)[0][0]
            metrics['rise_time'] = step_times[idx_90] - step_times[idx_10]
        except:
            metrics['rise_time'] = 999
        
        # 2. Settling time (within 5% of target)
        threshold = 0.05 * abs(target_value)
        settled_indices = np.where(np.abs(step_errors) < threshold)[0]
        if len(settled_indices) > 5:
            metrics['settling_time'] = step_times[settled_indices[5]]
        else:
            metrics['settling_time'] = 999
        
        # 3. Overshoot
        max_actual = np.max(step_actuals)
        metrics['overshoot_pct'] = ((max_actual - target_value) / target_value) * 100 if target_value != 0 else 0
        
        # 4. Steady-state error (last 20% of data)
        steady_start = int(len(step_errors) * 0.8)
        metrics['ss_error'] = np.mean(np.abs(step_errors[steady_start:]))
        metrics['ss_error_pct'] = (metrics['ss_error'] / abs(target_value)) * 100 if target_value != 0 else 0
        
        # 5. Oscillation detection
        # Count zero crossings in error signal (after settling)
        if metrics['settling_time'] < 999:
            settle_idx = np.where(step_times >= metrics['settling_time'])[0]
            if len(settle_idx) > 10:
                post_settle_errors = step_errors[settle_idx[0]:]
                zero_crossings = np.sum(np.diff(np.sign(post_settle_errors)) != 0)
                metrics['oscillations'] = zero_crossings
            else:
                metrics['oscillations'] = 0
        else:
            metrics['oscillations'] = 0
        
        return metrics
    
    def diagnose_and_suggest(self, metrics):
        """Diagnose issues and suggest PID changes"""
        print(f"\n📈 Analysis Results:")
        print(f"   Rise time: {metrics['rise_time']:.3f}s")
        print(f"   Settling time: {metrics['settling_time']:.3f}s")
        print(f"   Overshoot: {metrics['overshoot_pct']:.1f}%")
        print(f"   Steady-state error: {metrics['ss_error_pct']:.1f}%")
        print(f"   Oscillations: {metrics['oscillations']}")
        
        suggestions = []
        new_kp = self.kp
        new_ki = self.ki
        new_kd = self.kd
        
        # Diagnosis logic
        if metrics['rise_time'] > 1.0:
            suggestions.append("❌ SLOW RESPONSE: Rise time too long")
            new_kp = self.kp * 1.5
            suggestions.append(f"   → Increase Kp: {self.kp:.3f} → {new_kp:.3f}")
        
        elif metrics['rise_time'] < 0.2 and metrics['overshoot_pct'] > 20:
            suggestions.append("❌ OVERSHOOT: Too aggressive")
            new_kp = self.kp * 0.7
            suggestions.append(f"   → Decrease Kp: {self.kp:.3f} → {new_kp:.3f}")
        
        elif metrics['oscillations'] > 5:
            suggestions.append("❌ OSCILLATION: System unstable")
            new_kp = self.kp * 0.6
            new_ki = self.ki * 0.5
            suggestions.append(f"   → Decrease Kp: {self.kp:.3f} → {new_kp:.3f}")
            suggestions.append(f"   → Decrease Ki: {self.ki:.3f} → {new_ki:.3f}")
        
        elif metrics['ss_error_pct'] > 5:
            suggestions.append("❌ STEADY-STATE ERROR: Not reaching target")
            new_ki = self.ki * 1.5
            suggestions.append(f"   → Increase Ki: {self.ki:.3f} → {new_ki:.3f}")
        
        else:
            suggestions.append("✅ GOOD RESPONSE: System well-tuned!")
            if metrics['ss_error_pct'] > 2:
                new_ki = self.ki * 1.1
                suggestions.append(f"   → Fine-tune Ki: {self.ki:.3f} → {new_ki:.3f}")
        
        print(f"\n💡 Diagnosis:")
        for s in suggestions:
            print(s)
        
        return new_kp, new_ki, new_kd, suggestions
    
    def save_results(self, iteration, metrics, suggestions):
        """Save results to file"""
        filename = f"autotune_wheel{self.wheel_idx}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        
        # Convert numpy types to native Python types
        metrics_clean = {k: float(v) if hasattr(v, 'item') else v for k, v in metrics.items()}
        
        result = {
            'iteration': iteration,
            'wheel': self.wheel_idx,
            'pid': {'kp': self.kp, 'ki': self.ki, 'kd': self.kd},
            'metrics': metrics_clean,
            'suggestions': suggestions,
            'data': self.data
        }
        with open(filename, 'w') as f:
            json.dump(result, f, indent=2)
        print(f"   💾 Saved to {filename}")
    
    def run_auto_tune(self, max_iterations=5):
        """Run automatic tuning process"""
        print(f"\n🚀 Starting auto-tune (max {max_iterations} iterations)")
        print("="*60)
        
        for iteration in range(1, max_iterations + 1):
            print(f"\n\n{'='*60}")
            print(f"ITERATION {iteration}/{max_iterations}")
            print(f"Current PID: Kp={self.kp:.3f}, Ki={self.ki:.3f}, Kd={self.kd:.3f}")
            print('='*60)
            
            # Set current PID values
            self.set_pid(self.kp, self.ki, self.kd)
            
            # Run test
            if not self.send_test_command(speed=1.0, duration=3.0):
                print("❌ Failed to collect data")
                continue
            
            # Analyze
            metrics = self.analyze_response()
            if metrics is None:
                print("❌ Failed to analyze data")
                continue
            
            # Diagnose and suggest
            new_kp, new_ki, new_kd, suggestions = self.diagnose_and_suggest(metrics)
            
            # Save results
            self.save_results(iteration, metrics, suggestions)
            
            # Check if converged
            if "GOOD RESPONSE" in suggestions[0]:
                print(f"\n🎉 TUNING COMPLETE!")
                print(f"   Final PID: Kp={self.kp:.3f}, Ki={self.ki:.3f}, Kd={self.kd:.3f}")
                break
            
            # Apply new values if auto-apply enabled
            if self.auto_apply:
                print(f"\n🔄 Applying new PID values...")
                self.kp = new_kp
                self.ki = new_ki
                self.kd = new_kd
            else:
                response = input(f"\n❓ Apply these changes? (y/n): ")
                if response.lower() == 'y':
                    self.kp = new_kp
                    self.ki = new_ki
                    self.kd = new_kd
                else:
                    print("   Keeping current values")
                    break
            
            time.sleep(1)
        
        print(f"\n{'='*60}")
        print("✅ Auto-tuning session complete!")
        print(f"   Final: Kp={self.kp:.3f}, Ki={self.ki:.3f}, Kd={self.kd:.3f}")
        print('='*60)

def main():
    parser = argparse.ArgumentParser(description="Automated PID Auto-Tuner")
    parser.add_argument('--wheel', type=int, default=0, choices=[0, 1, 2, 3],
                        help='Wheel to tune (0=FR, 1=BR, 2=BL, 3=FL)')
    parser.add_argument('--auto', action='store_true',
                        help='Automatically apply suggested changes')
    parser.add_argument('--iterations', type=int, default=5,
                        help='Maximum tuning iterations')
    parser.add_argument('--kp', type=float, default=0.5,
                        help='Initial Kp value')
    parser.add_argument('--ki', type=float, default=0.05,
                        help='Initial Ki value')
    args = parser.parse_args()
    
    tuner = PIDAutoTuner(wheel_idx=args.wheel, auto_apply=args.auto)
    tuner.kp = args.kp
    tuner.ki = args.ki
    
    try:
        tuner.run_auto_tune(max_iterations=args.iterations)
    except KeyboardInterrupt:
        print("\n\n👋 Auto-tuning interrupted")

if __name__ == "__main__":
    main()

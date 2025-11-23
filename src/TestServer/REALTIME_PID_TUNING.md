# Real-Time PID Tuning with Telemetry

## Setup (One Time)

### 1. Flash Updated Firmware

**STM32:**
- Build and flash `src/drivetrain` with telemetry enabled
- Telemetry now runs every 500ms (non-blocking)

**ESP32:**
- Upload `parserChange/robocup/robocup.ino`
- Telemetry auto-forwards to UDP port 10001

### 2. Install Python Dependencies

```bash
pip install matplotlib numpy
```

## PID Tuning Workflow

### Terminal 1: Real-Time Plotter (Always Running)

```bash
cd "c:\Users\ramsh\Documents\Programming\Robocup Embedded\robocup-embedded\src\TestServer"
python realtime_pid_plotter.py
```

This opens a live graph showing:
- **Blue dashed line**: Target speed (what you commanded)
- **Green solid line**: Actual speed (from encoders)
- **Red dotted line**: Error (target - actual)

### Terminal 2: Send Test Commands

**Option A: Automated Step Test**
```bash
python step_response_tester.py --speed 500 --duration 5
```

**Option B: Manual Control**
```bash
python wasd_teleop.py --gui
```

**Option C: Direct Commands**
```bash
# Forward at 500 RPM
echo "1 move 500 0 0" | nc -u 239.42.42.42 10000

# Stop
echo "1 move 0 0 0" | nc -u 239.42.42.42 10000
```

### Terminal 3: Adjust PID Values

Based on what you see in the plot:

**Problem: Slow response (takes >2 seconds)**
```bash
python quick_pid.py 0.5 0.0  # Increase Kp
```

**Problem: Oscillating/vibrating**
```bash
python quick_pid.py 0.2 0.0  # Decrease Kp
```

**Problem: Doesn't reach exact target (steady-state error)**
```bash
python quick_pid.py 0.3 0.05  # Add Ki
```

**Problem: Overshoots then settles**
```bash
python quick_pid.py 0.25 0.0  # Reduce Kp slightly
```

## Iterative Tuning Process

1. **Start with:** `python quick_pid.py 0.3 0.0`

2. **Run test:** `python step_response_tester.py`

3. **Watch the plot** - observe:
   - Rise time (how fast it reaches 90% of target)
   - Overshoot (does it exceed target?)
   - Settling time (how long to stabilize)
   - Steady-state error (final error value)

4. **Adjust PID** based on observation

5. **Repeat** until satisfied

## Target Performance Metrics

Good PID tuning should achieve:
- ✅ Rise time: < 0.5 seconds to 90% of target
- ✅ Overshoot: < 10%
- ✅ Settling time: < 1 second
- ✅ Steady-state error: < 5%

## Per-Wheel Tuning

If one wheel behaves differently:

```bash
# Tune wheel 2 specifically (Kp=0.5, Ki=0.05)
echo "1 pidu 2 500 50 0" | nc -u 239.42.42.42 10000
```

Wheel indices:
- 0 = Front Right (FR)
- 1 = Back Right (BR)
- 2 = Back Left (BL)
- 3 = Front Left (FL)

## Troubleshooting

**No telemetry showing up?**
- Check ESP32 serial monitor - should see "📊 Telemetry forwarded to UDP"
- Verify STM32 is running (LED toggling on UART commands)
- Make sure both firmwares are freshly flashed

**Robot jittering again?**
- Telemetry rate is already reduced to 500ms
- If still jittering, reduce further in main.c: change `>= 50` to `>= 100` (1 second rate)

**Plot is too noisy?**
- Add filtering in realtime_pid_plotter.py
- Or use moving average on encoder readings in STM32

## Advanced: Logging Data for Analysis

Save data to CSV for later analysis:

```bash
python realtime_pid_plotter.py --log pid_data.csv
```

(Note: Add --log feature if needed)

## Quick Reference

| Command | Purpose |
|---------|---------|
| `python realtime_pid_plotter.py` | Start live graph |
| `python step_response_tester.py` | Automated step test |
| `python quick_pid.py 0.5 0.05` | Set PID gains |
| `python wasd_teleop.py --gui` | Manual control |
| `echo "1 move 500 0 0" \| nc -u 239.42.42.42 10000` | Send direct command |

---

**Happy Tuning! 🎛️📊**

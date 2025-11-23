#!/usr/bin/env bash
# Quick Start Script for PID Plotting

echo "🤖 PID Plotting Quick Start"
echo "=============================="
echo ""
echo "OPTION 1: Real-Time Plotting (Recommended for beginners)"
echo "----------------------------------------------------------"
echo "# Terminal 1: Start the plotter"
echo 'python pid_plotter.py --wheel 0'
echo ""
echo "# Terminal 2: Set PID and test"
echo 'python quick_pid.py 0.5 0.05'
echo 'echo "1 move 1.0 0 0" | nc -u 239.42.42.42 10000'
echo 'sleep 3'
echo 'echo "1 move 0 0 0" | nc -u 239.42.42.42 10000'
echo ""
echo ""
echo "OPTION 2: Log to CSV (Better for analysis)"
echo "----------------------------------------------------------"
echo "# Start logging (30 second test)"
echo 'python pid_logger.py --duration 30'
echo ""
echo "# Then in another terminal, run your tests"
echo 'python quick_pid.py 0.5 0.05'
echo '# ... do your movement tests ...'
echo ""
echo "# Plot the saved data"
echo 'python plot_csv.py pid_log_*.csv --wheel 0'
echo ""
echo ""
echo "OPTION 3: Interactive with WASD"
echo "----------------------------------------------------------"
echo "# Terminal 1: Plotter"
echo 'python pid_plotter.py --wheel 0'
echo ""
echo "# Terminal 2: Set PID"
echo 'python quick_pid.py 0.5 0.05'
echo ""
echo "# Terminal 3: Drive with keyboard"
echo 'python wasd_teleop.py --gui'
echo ""
echo ""
echo "⚠️  IMPORTANT: Before running, make sure:"
echo "   1. ESP32 and STM32 have updated firmware flashed"
echo "   2. Robot is powered on and connected to WiFi"
echo "   3. You're on the same network as the robot"
echo ""
echo "📖 For more details, see PLOTTING_GUIDE.md"

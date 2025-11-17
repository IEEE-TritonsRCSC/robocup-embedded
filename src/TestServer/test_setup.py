#!/usr/bin/env python3
"""
Test if plotting setup is working
Run this BEFORE flashing firmware to check your environment
"""

import sys

print("🔍 Checking PID Plotting Setup...")
print("="*50)

# Check Python version
print(f"✓ Python version: {sys.version.split()[0]}")

# Check required packages
packages = ['matplotlib', 'pandas', 'numpy', 'socket', 'struct']
missing = []

for pkg in packages:
    try:
        if pkg == 'socket' or pkg == 'struct':
            __import__(pkg)
        else:
            __import__(pkg)
        print(f"✓ {pkg} installed")
    except ImportError:
        print(f"✗ {pkg} NOT installed")
        missing.append(pkg)

if missing:
    print(f"\n❌ Missing packages: {', '.join(missing)}")
    print(f"\nInstall with: pip install {' '.join(missing)}")
    sys.exit(1)

print("\n" + "="*50)
print("✅ All dependencies installed!")
print("\n📋 Next steps:")
print("   1. Flash updated STM32 firmware (src/drivetrain/)")
print("   2. Flash updated ESP32 firmware (parserChange/robocup/)")
print("   3. Power on robot and connect to WiFi")
print("   4. Run: python pid_plotter.py --wheel 0")
print("\n💡 Try it now:")
print("   python pid_plotter.py --wheel 0")
print("   (It will wait for data from the robot)")

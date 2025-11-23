#!/usr/bin/env python3
"""
Plot PID data from CSV log file
"""

import pandas as pd
import matplotlib.pyplot as plt
import argparse
import sys

def plot_wheel(df, wheel_idx, ax1, ax2):
    """Plot single wheel data"""
    wheel_names = ['Front-Right', 'Back-Right', 'Back-Left', 'Front-Left']
    
    # Plot target vs actual
    ax1.plot(df['timestamp'], df[f'w{wheel_idx}_target'], 'r-', label='Target', linewidth=2)
    ax1.plot(df['timestamp'], df[f'w{wheel_idx}_actual'], 'b-', label='Actual', linewidth=2)
    ax1.set_ylabel('Speed')
    ax1.set_title(f'{wheel_names[wheel_idx]} - Target vs Actual')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Plot error and output
    ax2.plot(df['timestamp'], df[f'w{wheel_idx}_error'], 'g-', label='Error', linewidth=1.5)
    ax2.plot(df['timestamp'], df[f'w{wheel_idx}_output'], 'm-', label='PID Output', linewidth=1.5, alpha=0.7)
    ax2.set_ylabel('Error / Output')
    ax2.set_xlabel('Time (s)')
    ax2.set_title(f'{wheel_names[wheel_idx]} - Error and PID Output')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # Calculate statistics
    error = df[f'w{wheel_idx}_error']
    print(f"\n{wheel_names[wheel_idx]} Statistics:")
    print(f"  Mean Error: {error.mean():.2f}")
    print(f"  Std Error: {error.std():.2f}")
    print(f"  Max Error: {error.max():.2f}")
    print(f"  Min Error: {error.min():.2f}")
    print(f"  RMS Error: {(error**2).mean()**0.5:.2f}")

def main():
    parser = argparse.ArgumentParser(description="Plot PID data from CSV")
    parser.add_argument('csvfile', help='CSV file to plot')
    parser.add_argument('--wheel', type=int, default=None, choices=[0, 1, 2, 3],
                        help='Specific wheel to plot (default: all)')
    parser.add_argument('--start', type=float, default=None,
                        help='Start time in seconds')
    parser.add_argument('--end', type=float, default=None,
                        help='End time in seconds')
    args = parser.parse_args()
    
    # Read CSV
    try:
        df = pd.read_csv(args.csvfile)
    except FileNotFoundError:
        print(f"❌ Error: File '{args.csvfile}' not found")
        sys.exit(1)
    
    # Filter by time range
    if args.start is not None:
        df = df[df['timestamp'] >= args.start]
    if args.end is not None:
        df = df[df['timestamp'] <= args.end]
    
    print(f"📊 Plotting {len(df)} data points from {args.csvfile}")
    
    if args.wheel is not None:
        # Plot single wheel
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
        plot_wheel(df, args.wheel, ax1, ax2)
    else:
        # Plot all wheels
        fig, axes = plt.subplots(4, 2, figsize=(14, 12))
        fig.suptitle('PID Response - All Wheels', fontsize=16)
        
        for i in range(4):
            plot_wheel(df, i, axes[i, 0], axes[i, 1])
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()

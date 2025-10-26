#!/usr/bin/env python3
"""
WASD controller for the RoboCup robot over UDP multicast.
Adds runtime PID tuning support.
Also supports a lightweight Tkinter GUI with --gui.

UDP multicast -> ESP32 -> STM32 UART protocol (header 0xCA 0xFE):
- Speed command (0x00): [s0H s0L s1H s1L s2H s2L s3H s3L dribble]
- PID update (0xA0):   [idx kpH kpL kiH kiL kdH kdL rsv rsv]  (values in milli-units)

Controls (CLI mode):
  W/S: forward/back (vy) [-10..10]
  A/D: strafe left/right (vx) [-10..10]
  Q/E: rotate CCW/CW (rot) [-10..10]
  SPACE: stop (vx=vy=rot=0)
  K: kick
  1..4: select wheel; 0 = ALL; T/G Kp +/- ; Y/H Ki +/- ; U/J Kd +/- ; P send PID
  Q or ESC: quit

GUI mode (--gui):
  - Sliders for Forward (vy), Strafe (vx), Rotation (rot)
  - Wheel selector (ALL, FR, BR, BL, FL)
  - Kp/Ki/Kd fields and Send PID button
  - Kick and Stop buttons
  - On-screen key guide (W/S/A/D/Q/E)
"""
import argparse
import socket
import struct
import sys
import time

MCAST_GRP = "239.42.42.42"
MCAST_PORT = 10000


def main():
    p = argparse.ArgumentParser(description="WASD teleop for RoboCup robot (UDP multicast) with PID tuning")
    p.add_argument("--robot", type=int, default=1, help="Robot number expected by firmware (ROBOT_NO)")
    p.add_argument("--rate", type=float, default=20.0, help="Send rate (Hz)")
    p.add_argument("--step-power", type=int, default=1, help="Increment per W/S keypress ([-10,10])")
    p.add_argument("--step-rot", type=int, default=1, help="Increment per A/D keypress ([-10,10])")
    p.add_argument("--iface", default=None, help="Optional local interface IP for multicast (e.g., 192.168.x.x)")
    p.add_argument("--gui", action="store_true", help="Launch a simple Tkinter GUI instead of CLI controls")
    # PID step sizes (CLI hotkeys)
    p.add_argument("--kp-step", type=float, default=0.05, help="Delta for Kp on T/G")
    p.add_argument("--ki-step", type=float, default=0.01, help="Delta for Ki on Y/H")
    p.add_argument("--kd-step", type=float, default=0.00, help="Delta for Kd on U/J")
    args = p.parse_args()

    # UDP socket to multicast group
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack('b', 1))
    if args.iface:
        try:
            sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(args.iface))
        except OSError as e:
            print(f"Warning: failed to set multicast interface {args.iface}: {e}")

    def send_move(vx: int, vy: int, rot: int = 0):
        msg = f"{args.robot} move {int(vx)} {int(vy)} {int(rot)}\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    def send_kick():
        msg = f"{args.robot} kick\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    def send_pid(idx: int, kp: float, ki: float, kd: float):
        kp_q = int(round(kp * 1000.0))
        ki_q = int(round(ki * 1000.0))
        kd_q = int(round(kd * 1000.0))
        msg = f"{args.robot} pidu {idx} {kp_q} {ki_q} {kd_q}\n"
        sock.sendto(msg.encode("utf-8"), (MCAST_GRP, MCAST_PORT))

    # Shared state
    vx = 0  # strafe
    vy = 0  # forward
    rot = 0 # rotation
    MIN_V, MAX_V = -10, 10

    pid = [
        {"kp": 0.43, "ki": 0.01, "kd": 0.00},
        {"kp": 0.30, "ki": 0.01, "kd": 0.00},
        {"kp": 0.273, "ki": 0.01, "kd": 0.00},
        {"kp": 0.403, "ki": 0.01, "kd": 0.00},
    ]
    selected = 0  # 0..3; 4 means ALL

    if args.gui:
        # --- GUI MODE ---
        try:
            import tkinter as tk
            from tkinter import ttk
        except Exception as e:
            print(f"Tkinter not available: {e}. Falling back to CLI mode.")
        else:
            root = tk.Tk()
            root.title(f"RoboCup Teleop (Robot {args.robot})")

            # Frames
            frmMove = ttk.LabelFrame(root, text="Motion")
            frmMove.grid(row=0, column=0, padx=8, pady=6, sticky="nsew")
            frmPID = ttk.LabelFrame(root, text="PID Tuning")
            frmPID.grid(row=1, column=0, padx=8, pady=6, sticky="nsew")

            # Key help
            ttk.Label(frmMove, text="Keys: W/S=Forward/Back  A/D=Strafe  Q/E=Rotate").grid(row=0, column=0, columnspan=3, sticky="w", padx=6)

            # Sliders: Forward (vy), Strafe (vx), Rotation (rot)
            vy_var = tk.IntVar(value=0)
            vx_var = tk.IntVar(value=0)
            r_var  = tk.IntVar(value=0)
            tk.Scale(frmMove, from_=10, to=-10, orient=tk.VERTICAL, variable=vy_var, label="Forward").grid(row=1, column=0, padx=8, pady=4)
            tk.Scale(frmMove, from_=10, to=-10, orient=tk.VERTICAL, variable=vx_var, label="Strafe").grid(row=1, column=1, padx=8, pady=4)
            tk.Scale(frmMove, from_=10, to=-10, orient=tk.VERTICAL, variable=r_var,  label="Rotate").grid(row=1, column=2, padx=8, pady=4)

            # Buttons
            def on_stop():
                vy_var.set(0); vx_var.set(0); r_var.set(0); send_move(0, 0, 0)
            ttk.Button(frmMove, text="Kick", command=send_kick).grid(row=2, column=0, padx=6, pady=4, sticky="ew")
            ttk.Button(frmMove, text="Stop", command=on_stop).grid(row=2, column=1, padx=6, pady=4, sticky="ew")

            # Wheel selector and PID controls
            wheel_var = tk.StringVar(value="FR (1)")
            wheel_map = {"ALL": 4, "FR (1)": 0, "BR (2)": 1, "BL (3)": 2, "FL (4)": 3}
            ttk.Label(frmPID, text="Wheel:").grid(row=0, column=0, sticky="e")
            wheel_cb = ttk.Combobox(frmPID, textvariable=wheel_var, values=list(wheel_map.keys()), state="readonly", width=10)
            wheel_cb.grid(row=0, column=1, padx=4, pady=4)
            wheel_cb.current(0)

            kp_var = tk.DoubleVar(value=pid[0]['kp'])
            ki_var = tk.DoubleVar(value=pid[0]['ki'])
            kd_var = tk.DoubleVar(value=pid[0]['kd'])

            def load_selected_pid(*_):
                idx = wheel_map.get(wheel_var.get(), 4)
                if idx == 4:
                    kp_var.set(sum(p['kp'] for p in pid)/4.0)
                    ki_var.set(sum(p['ki'] for p in pid)/4.0)
                    kd_var.set(sum(p['kd'] for p in pid)/4.0)
                else:
                    kp_var.set(pid[idx]['kp'])
                    ki_var.set(pid[idx]['ki'])
                    kd_var.set(pid[idx]['kd'])
            wheel_cb.bind('<<ComboboxSelected>>', load_selected_pid)

            ttk.Label(frmPID, text="Kp").grid(row=1, column=0, sticky="e")
            ttk.Entry(frmPID, textvariable=kp_var, width=8).grid(row=1, column=1, sticky="w")
            ttk.Label(frmPID, text="Ki").grid(row=1, column=2, sticky="e")
            ttk.Entry(frmPID, textvariable=ki_var, width=8).grid(row=1, column=3, sticky="w")
            ttk.Label(frmPID, text="Kd").grid(row=1, column=4, sticky="e")
            ttk.Entry(frmPID, textvariable=kd_var, width=8).grid(row=1, column=5, sticky="w")

            def on_send_pid():
                idx = wheel_map.get(wheel_var.get(), 4)
                kp = float(kp_var.get()); ki = float(ki_var.get()); kd = float(kd_var.get())
                if idx == 4:
                    for i in range(4):
                        pid[i]['kp'], pid[i]['ki'], pid[i]['kd'] = kp, ki, kd
                        send_pid(i, kp, ki, kd)
                else:
                    pid[idx]['kp'], pid[idx]['ki'], pid[idx]['kd'] = kp, ki, kd
                    send_pid(idx, kp, ki, kd)
            ttk.Button(frmPID, text="Send PID", command=on_send_pid).grid(row=1, column=6, padx=8)

            # Status label
            status = tk.StringVar(value="Ready")
            ttk.Label(root, textvariable=status).grid(row=2, column=0, sticky="w", padx=8)

            # Periodic sender
            period_ms = int(1000.0 / max(args.rate, 1e-3))
            def tick_send():
                nonlocal vx, vy, rot
                vy = max(MIN_V, min(MAX_V, int(vy_var.get())))
                vx = max(MIN_V, min(MAX_V, int(vx_var.get())))
                rot = max(MIN_V, min(MAX_V, int(r_var.get())))
                send_move(vx, vy, rot)
                status.set(f"vx={vx}, vy={vy}, rot={rot}")
                root.after(period_ms, tick_send)
            root.after(period_ms, tick_send)

            # Key bindings
            def on_key(event):
                c = event.keysym.lower()
                if c == 'w': vy_var.set(min(10, vy_var.get()+1))
                elif c == 's': vy_var.set(max(-10, vy_var.get()-1))
                elif c == 'a': vx_var.set(max(-10, vx_var.get()-1))
                elif c == 'd': vx_var.set(min(10, vx_var.get()+1))
                elif c == 'q': r_var.set(min(10, r_var.get()+1))
                elif c == 'e': r_var.set(max(-10, r_var.get()-1))
                elif c == 'space': on_stop()
                elif c == 'k': send_kick()
            root.bind('<Key>', on_key)

            def on_close():
                try:
                    for _ in range(3):
                        send_move(0,0,0); time.sleep(0.05)
                finally:
                    root.destroy()
            root.protocol("WM_DELETE_WINDOW", on_close)

            root.mainloop()
            return

    # --- CLI MODE ---
    using_msvcrt = sys.platform.startswith("win")

    print("WASD teleop active -> sending to %s:%d for robot %d" % (MCAST_GRP, MCAST_PORT, args.robot))
    print("Controls: W/S forward/back, A/D strafe, Q/E rotate, SPACE stop, K kick, 1..4 wheel, 0=ALL, T/G Kp +/- , Y/H Ki +/- , U/J Kd +/- , P send PID, Q/ESC quit")

    period = 1.0 / max(args.rate, 1e-3)
    last_print = 0.0

    def print_status():
        sidx = "ALL" if selected == 4 else (selected + 1)
        def fmt(p):
            return f"kp={p['kp']:.3f} ki={p['ki']:.3f} kd={p['kd']:.3f}"
        if selected == 4:
            ptxt = " | ".join(fmt(p) for p in pid)
        else:
            ptxt = fmt(pid[selected])
        print(f"vx={vx:>3}, vy={vy:>3}, rot={rot:>3} | sel={sidx} | {ptxt}    ", end="\r", flush=True)

    try:
        if using_msvcrt:
            import msvcrt
            while True:
                while msvcrt.kbhit():
                    ch = msvcrt.getwch()
                    if ch in ('q', 'Q') or ord(ch) == 27:
                        return
                    if ch in ('w', 'W'):
                        vy = min(MAX_V, vy + args.step_power)
                    elif ch in ('s', 'S'):
                        vy = max(MIN_V, vy - args.step_power)
                    elif ch in ('a', 'A'):
                        vx = max(MIN_V, vx - args.step_power)
                    elif ch in ('d', 'D'):
                        vx = min(MAX_V, vx + args.step_power)
                    elif ch in ('e', 'E'):
                        rot = max(MIN_V, rot - args.step_rot)
                    elif ch in ('q', 'Q'):
                        rot = min(MAX_V, rot + args.step_rot)
                    elif ch == ' ':
                        vx = 0; vy = 0; rot = 0
                    elif ch in ('k', 'K'):
                        send_kick()
                    elif ch in ('1','2','3','4','0'):
                        selected = 4 if ch == '0' else (ord(ch)-ord('1'))
                    elif ch in ('t','T','g','G','y','Y','h','H','u','U','j','J','p','P'):
                        if ch in ('t','T'):  # Kp +
                            delta = args.kp_step
                            if selected == 4:
                                for p in pid: p['kp'] += delta
                            else:
                                pid[selected]['kp'] += delta
                        elif ch in ('g','G'):
                            delta = -args.kp_step
                            if selected == 4:
                                for p in pid: p['kp'] = max(0.0, p['kp'] + delta)
                            else:
                                pid[selected]['kp'] = max(0.0, pid[selected]['kp'] + delta)
                        elif ch in ('y','Y'):
                            delta = args.ki_step
                            if selected == 4:
                                for p in pid: p['ki'] += delta
                            else:
                                pid[selected]['ki'] += delta
                        elif ch in ('h','H'):
                            delta = -args.ki_step
                            if selected == 4:
                                for p in pid: p['ki'] = max(0.0, p['ki'] + delta)
                            else:
                                pid[selected]['ki'] = max(0.0, pid[selected]['ki'] + delta)
                        elif ch in ('u','U'):
                            delta = args.kd_step
                            if selected == 4:
                                for p in pid: p['kd'] += delta
                            else:
                                pid[selected]['kd'] += delta
                        elif ch in ('j','J'):
                            delta = -args.kd_step
                            if selected == 4:
                                for p in pid: p['kd'] = max(0.0, p['kd'] + delta)
                            else:
                                pid[selected]['kd'] = max(0.0, pid[selected]['kd'] + delta)
                        elif ch in ('p','P'):
                            if selected == 4:
                                for i in range(4):
                                    send_pid(i, pid[i]['kp'], pid[i]['ki'], pid[i]['kd'])
                            else:
                                send_pid(selected, pid[selected]['kp'], pid[selected]['ki'], pid[selected]['kd'])
                send_move(vx, vy, rot)
                now = time.time()
                if now - last_print > 0.2:
                    print_status()
                    last_print = now
                time.sleep(period)
        else:
            import tty, termios, select
            fd = sys.stdin.fileno()
            old_settings = termios.tcgetattr(fd)
            tty.setcbreak(fd)
            try:
                while True:
                    r, _, _ = select.select([sys.stdin], [], [], 0)
                    if r:
                        ch = sys.stdin.read(1)
                        if ch in ('\x1b'):
                            return
                        if ch in ('w','W'):
                            vy = min(MAX_V, vy + args.step_power)
                        elif ch in ('s','S'):
                            vy = max(MIN_V, vy - args.step_power)
                        elif ch in ('a','A'):
                            vx = max(MIN_V, vx - args.step_power)
                        elif ch in ('d','D'):
                            vx = min(MAX_V, vx + args.step_power)
                        elif ch in ('e','E'):
                            rot = max(MIN_V, rot - args.step_rot)
                        elif ch in ('q','Q'):
                            rot = min(MAX_V, rot + args.step_rot)
                        elif ch == ' ':
                            vx = 0; vy = 0; rot = 0
                        elif ch in ('k','K'):
                            send_kick()
                        elif ch in ('1','2','3','4','0'):
                            selected = 4 if ch == '0' else (ord(ch)-ord('1'))
                        elif ch in ('t','T','g','G','y','Y','h','H','u','U','j','J','p','P'):
                            if ch in ('t','T'):
                                delta = args.kp_step
                                if selected == 4:
                                    for p in pid: p['kp'] += delta
                                else:
                                    pid[selected]['kp'] += delta
                            elif ch in ('g','G'):
                                delta = -args.kp_step
                                if selected == 4:
                                    for p in pid: p['kp'] = max(0.0, p['kp'] + delta)
                                else:
                                    pid[selected]['kp'] = max(0.0, pid[selected]['kp'] + delta)
                            elif ch in ('y','Y'):
                                delta = args.ki_step
                                if selected == 4:
                                    for p in pid: p['ki'] += delta
                                else:
                                    pid[selected]['ki'] += delta
                            elif ch in ('h','H'):
                                delta = -args.ki_step
                                if selected == 4:
                                    for p in pid: p['ki'] = max(0.0, p['ki'] + delta)
                                else:
                                    pid[selected]['ki'] = max(0.0, pid[selected]['ki'] + delta)
                            elif ch in ('u','U'):
                                delta = args.kd_step
                                if selected == 4:
                                    for p in pid: p['kd'] += delta
                                else:
                                    pid[selected]['kd'] += delta
                            elif ch in ('j','J'):
                                delta = -args.kd_step
                                if selected == 4:
                                    for p in pid: p['kd'] = max(0.0, p['kd'] + delta)
                                else:
                                    pid[selected]['kd'] = max(0.0, pid[selected]['kd'] + delta)
                            elif ch in ('p','P'):
                                if selected == 4:
                                    for i in range(4):
                                        send_pid(i, pid[i]['kp'], pid[i]['ki'], pid[i]['kd'])
                                else:
                                    send_pid(selected, pid[selected]['kp'], pid[selected]['ki'], pid[selected]['kd'])
                    send_move(vx, vy, rot)
                    now = time.time()
                    if now - last_print > 0.2:
                        print_status()
                        last_print = now
                    time.sleep(period)
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    finally:
        for _ in range(3):
            send_move(0, 0, 0)
            time.sleep(0.05)
        print("\nStopped.")


if __name__ == "__main__":
    main()

"""
PID Visualizer + Live Tuning Console

Features:
- Live plot of target/actual/output telemetry (UDP multicast).
- Interactive console to set PID gains while the robot runs.

Telemetry format (expected):
  [0xFE, 0xED] + uint32 timestamp_ms (big-endian) +
  4 wheels x (int16 target, int16 actual, int16 output)

PID update command format (expected by firmware/ESP32 parser):
  "<ROBOT_ID> pidu <wheel_idx> <kp_q> <ki_q> <kd_q>"
  where kp_q = int(kp * 1000), etc.

Note:
  Live PID updates require firmware support for the "pidu" command.
  If your current firmware does not accept PID updates, the plot will still work,
  but PID changes will have no effect.
"""

import argparse
import math
import socket
import struct
import threading
import time
from collections import deque

try:
    import matplotlib.pyplot as plt
    import matplotlib.animation as animation
    from matplotlib.widgets import Button
except ImportError as exc:
    raise SystemExit("matplotlib is required. Install with: pip install matplotlib") from exc


DEFAULT_MCAST_IP = "239.42.42.42"
DEFAULT_TELEMETRY_PORT = 10001
DEFAULT_COMMAND_PORT = 10000
DEFAULT_ROBOT_ID = 1

DEFAULT_MAX_POINTS = 600
DEFAULT_PID_SCALE = 1000.0
TELEMETRY_MIN_LEN = 30

WHEEL_NAMES = ["Front-Right", "Back-Right", "Back-Left", "Front-Left"]


class PIDVisualizer:
    def __init__(self, args):
        self.args = args
        self.stop_event = threading.Event()
        self.start_time = time.time()
        self.last_rx_time = None
        self.warned_no_telemetry = False

        # Telemetry socket (multicast)
        self.recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.recv_sock.bind(("", args.telemetry_port))

        mreq = struct.pack("4sl", socket.inet_aton(args.mcast_ip), socket.INADDR_ANY)
        self.recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.recv_sock.settimeout(0.01)

        # Command socket (multicast send)
        self.send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.send_sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack("b", 1))

        self.t0 = None

        # Data buffers: one deque set per wheel
        self.data = []
        for _ in range(4):
            self.data.append({
                "t": deque(maxlen=args.points),
                "target": deque(maxlen=args.points),
                "actual": deque(maxlen=args.points),
                "output": deque(maxlen=args.points),
                "error": deque(maxlen=args.points),
            })

        self._setup_plot()

    def _setup_plot(self):
        if self.args.view == "single":
            wheel = self.args.wheel
            self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(12, 8))
            self.fig.suptitle(f"PID Visualizer - Wheel {wheel} ({WHEEL_NAMES[wheel]})")

            self.line_target, = self.ax1.plot([], [], "r--", label="Target")
            self.line_actual, = self.ax1.plot([], [], "b-", label="Actual")
            self.line_output, = self.ax1.plot([], [], "m:", label="Output")
            self.ax1.set_ylabel("Speed / Output")
            self.ax1.grid(True, alpha=0.3)
            self.ax1.legend(loc="upper right")

            self.line_error, = self.ax2.plot([], [], "g-", label="Error")
            self.ax2.set_ylabel("Error")
            self.ax2.set_xlabel("Time (s)")
            self.ax2.grid(True, alpha=0.3)
            self.ax2.legend(loc="upper right")
        else:
            self.fig, self.axes = plt.subplots(2, 2, figsize=(12, 8), sharex=True)
            self.fig.suptitle("PID Visualizer - All Wheels")
            self.lines = []
            for idx, ax in enumerate(self.axes.flat):
                ax.set_title(f"Wheel {idx} ({WHEEL_NAMES[idx]})", fontsize=10)
                line_target, = ax.plot([], [], "r--", label="Target")
                line_actual, = ax.plot([], [], "b-", label="Actual")
                line_output, = ax.plot([], [], "m:", label="Output")
                ax.grid(True, alpha=0.3)
                if idx >= 2:
                    ax.set_xlabel("Time (s)")
                if idx % 2 == 0:
                    ax.set_ylabel("Speed / Output")
                if idx == 0:
                    ax.legend(loc="upper right", fontsize=8)
                self.lines.append((line_target, line_actual, line_output))

        self.error_texts = []
        if self.args.view == "single":
            text = self.fig.text(0.02, 0.105, f"{WHEEL_NAMES[self.args.wheel]} Diff=--", fontsize=9)
            self.error_texts.append(text)
        else:
            x_positions = [0.02, 0.27, 0.52, 0.77]
            for i, xpos in enumerate(x_positions):
                text = self.fig.text(xpos, 0.105, f"{WHEEL_NAMES[i]} Diff=--", fontsize=9)
                self.error_texts.append(text)

        self.fig.subplots_adjust(bottom=0.2)

        reset_ax = self.fig.add_axes([0.54, 0.04, 0.12, 0.06])
        stop_ax = self.fig.add_axes([0.68, 0.04, 0.12, 0.06])
        dash_ax = self.fig.add_axes([0.82, 0.04, 0.12, 0.06])
        self.reset_button = Button(reset_ax, "Reset", color="#e6e6e6", hovercolor="#cfcfcf")
        self.stop_button = Button(stop_ax, "STOP", color="#f2d6d6", hovercolor="#f5baba")
        self.dash_button = Button(dash_ax, "Dash 50", color="#d6e8f2", hovercolor="#b7d6ea")
        self.reset_button.on_clicked(self._on_reset_clicked)
        self.stop_button.on_clicked(self._on_stop_clicked)
        self.dash_button.on_clicked(self._on_dash50_clicked)

        # Avoid tight_layout warnings with manually placed button axes.

    def _reset_data(self):
        self.t0 = None
        for i in range(4):
            for key in ("t", "target", "actual", "output", "error"):
                self.data[i][key].clear()

    def _on_reset_clicked(self, _event):
        self._reset_data()
        self.start_time = time.time()
        self.warned_no_telemetry = False
        self._update_error_texts()
        self.fig.canvas.draw_idle()

    def _on_stop_clicked(self, _event):
        self.send_command("stop")
        print("Sent: stop")

    def _on_dash50_clicked(self, _event):
        cmd = f"{self.args.robot_id} dash 50 0"
        self.send_command(cmd)
        print(f"Sent: {cmd}")

    def _format_error(self, value):
        if value is None:
            return "--"
        return f"{value:+.0f}"

    def _update_error_texts(self):
        if self.args.view == "single":
            idx = self.args.wheel
            err = self.data[idx]["error"][-1] if self.data[idx]["error"] else None
            self.error_texts[0].set_text(f"{WHEEL_NAMES[idx]} Diff={self._format_error(err)}")
        else:
            for i, text in enumerate(self.error_texts):
                err = self.data[i]["error"][-1] if self.data[i]["error"] else None
                text.set_text(f"{WHEEL_NAMES[i]} Diff={self._format_error(err)}")

    def parse_telemetry(self, data):
        if len(data) < TELEMETRY_MIN_LEN or data[0] != 0xFE or data[1] != 0xED:
            return None

        timestamp_ms = struct.unpack(">I", data[2:6])[0]
        wheels = []
        for i in range(4):
            idx = 6 + (i * 6)
            target = struct.unpack(">h", data[idx:idx + 2])[0]
            actual = struct.unpack(">h", data[idx + 2:idx + 4])[0]
            output = struct.unpack(">h", data[idx + 4:idx + 6])[0]
            wheels.append({
                "target": target,
                "actual": actual,
                "output": output,
                "error": target - actual,
            })

        return {
            "timestamp": timestamp_ms / 1000.0,
            "wheels": wheels,
        }

    def _append_telemetry(self, telemetry):
        if self.t0 is None:
            self.t0 = telemetry["timestamp"]
        t = telemetry["timestamp"] - self.t0

        for i in range(4):
            wheel = telemetry["wheels"][i]
            self.data[i]["t"].append(t)
            self.data[i]["target"].append(wheel["target"])
            self.data[i]["actual"].append(wheel["actual"])
            self.data[i]["output"].append(wheel["output"])
            self.data[i]["error"].append(wheel["error"])

    def update(self, _frame):
        received = False
        try:
            while True:
                data, _ = self.recv_sock.recvfrom(1024)
                telemetry = self.parse_telemetry(data)
                if telemetry:
                    received = True
                    self._append_telemetry(telemetry)
        except socket.timeout:
            pass
        if received:
            self.last_rx_time = time.time()
        elif not self.warned_no_telemetry and (time.time() - self.start_time) > 2.0:
            print(
                "No telemetry received yet. Check STM32/ESP32 telemetry firmware, "
                "multicast IP/port, and network."
            )
            self.warned_no_telemetry = True

        if self.args.view == "single":
            idx = self.args.wheel
            times = list(self.data[idx]["t"])
            self.line_target.set_data(times, list(self.data[idx]["target"]))
            self.line_actual.set_data(times, list(self.data[idx]["actual"]))
            self.line_output.set_data(times, list(self.data[idx]["output"]))
            self.line_error.set_data(times, list(self.data[idx]["error"]))

            for ax in (self.ax1, self.ax2):
                ax.relim()
                ax.autoscale_view()
            self._update_error_texts()
            return (
                self.line_target,
                self.line_actual,
                self.line_output,
                self.line_error,
                *self.error_texts,
            )

        # All wheels view
        for i in range(4):
            times = list(self.data[i]["t"])
            line_target, line_actual, line_output = self.lines[i]
            line_target.set_data(times, list(self.data[i]["target"]))
            line_actual.set_data(times, list(self.data[i]["actual"]))
            line_output.set_data(times, list(self.data[i]["output"]))
            ax = self.axes.flat[i]
            ax.relim()
            ax.autoscale_view()

        self._update_error_texts()

        # Return all line artists
        artists = []
        for trio in self.lines:
            artists.extend(trio)
        artists.extend(self.error_texts)
        return tuple(artists)

    def send_command(self, cmd):
        if not cmd.endswith("\0") and not cmd.endswith("\n"):
            cmd = f"{cmd}\0"
        self.send_sock.sendto(cmd.encode("utf-8"), (self.args.mcast_ip, self.args.command_port))

    def send_pid_update(self, wheel, kp, ki, kd):
        kp_q = int(kp * self.args.pid_scale)
        ki_q = int(ki * self.args.pid_scale)
        kd_q = int(kd * self.args.pid_scale)
        cmd = f"{self.args.robot_id} pidu {wheel} {kp_q} {ki_q} {kd_q}"
        self.send_command(cmd)
        print(f"Sent: {cmd}")

    def input_loop(self):
        help_text = (
            "\nCommands:\n"
            "  pid <wheel|all> <kp> <ki> [kd]   Set PID (kp/ki/kd in float)\n"
            "  pidraw <wheel|all> <kp_q> <ki_q> <kd_q>  Set PID in quantized units\n"
            "  t <deg_per_sec>                  Turn (degrees/sec, auto-converted to rad/s)\n"
            "  d <power> <dir_deg>               Dash (power, direction in degrees)\n"
            "  s <power>                         Short kick (skick)\n"
            "  k                                 Kick\n"
            "  c                                 Catch (dribbler on)\n"
            "  stop                              Emergency stop\n"
            "  wheel <fr> <br> <bl> <fl> [drib]   Direct wheel control (rad/s, drib -100..100)\n"
            "  wheel <idx> <speed>               Direct single-wheel control (rad/s)\n"
            "  wheelraw <...>                    Same as wheel but values are raw (rad/s*100)\n"
            "  help                              Show this help\n"
            "  quit                              Exit\n"
        )
        print(help_text)

        while not self.stop_event.is_set():
            try:
                line = input("> ").strip()
            except (EOFError, KeyboardInterrupt):
                self.stop_event.set()
                break

            if not line:
                continue
            if line in ("quit", "exit"):
                self.stop_event.set()
                break
            if line == "help":
                print(help_text)
                continue

            tokens = line.split()
            cmd = tokens[0].lower()

            if cmd in ("pid", "pidraw"):
                if len(tokens) < 4:
                    print("Usage: pid <wheel|all> <kp> <ki> [kd]")
                    continue

                target = tokens[1]
                kd_val = 0.0
                if len(tokens) >= 5:
                    kd_val = float(tokens[4])

                wheels = []
                if target == "all":
                    wheels = [0, 1, 2, 3]
                else:
                    try:
                        wheels = [int(target)]
                    except ValueError:
                        print("Wheel must be 0-3 or 'all'.")
                        continue

                try:
                    if cmd == "pid":
                        kp = float(tokens[2])
                        ki = float(tokens[3])
                        kd = float(kd_val)
                        for w in wheels:
                            self.send_pid_update(w, kp, ki, kd)
                    else:
                        kp_q = int(tokens[2])
                        ki_q = int(tokens[3])
                        kd_q = int(tokens[4]) if len(tokens) >= 5 else 0
                        for w in wheels:
                            cmd = f"{self.args.robot_id} pidu {w} {kp_q} {ki_q} {kd_q}"
                            self.send_command(cmd)
                            print(f"Sent: {cmd}")
                except ValueError:
                    print("Invalid PID values.")
                continue

            if cmd in ("t", "turn"):
                if len(tokens) != 2:
                    print("Usage: t <deg_per_sec>")
                    continue
                try:
                    deg = float(tokens[1])
                except ValueError:
                    print("Invalid angle.")
                    continue
                rad = math.radians(deg)
                self.send_command(f"{self.args.robot_id} turn {rad}")
                continue

            if cmd in ("d", "dash"):
                if len(tokens) != 3:
                    print("Usage: d <power> <dir_deg>")
                    continue
                try:
                    power = float(tokens[1])
                    deg = float(tokens[2])
                except ValueError:
                    print("Invalid dash parameters.")
                    continue
                rad = math.radians(deg)
                self.send_command(f"{self.args.robot_id} dash {power} {rad}")
                continue

            if cmd in ("s", "skick", "setpower"):
                if len(tokens) != 2:
                    print("Usage: s <power>")
                    continue
                try:
                    power = float(tokens[1])
                except ValueError:
                    print("Invalid power.")
                    continue
                self.send_command(f"{self.args.robot_id} skick {power}")
                continue

            if cmd in ("k", "kick"):
                self.send_command(f"{self.args.robot_id} kick")
                continue

            if cmd in ("c", "catch"):
                self.send_command(f"{self.args.robot_id} catch")
                continue

            if cmd == "stop":
                self.send_command("stop")
                continue

            if cmd in ("wheel", "wheelraw"):
                if len(tokens) < 3:
                    print("Usage: wheel <fr> <br> <bl> <fl> [drib] OR wheel <idx> <speed>")
                    continue

                try:
                    values = [float(v) for v in tokens[1:]]
                except ValueError:
                    print("Invalid wheel values.")
                    continue

                # Allow wheel <idx> <speed> shorthand.
                if len(values) == 2:
                    idx = int(values[0])
                    speed = values[1]
                    if idx < 0 or idx > 3:
                        print("Wheel index must be 0..3.")
                        continue
                    speeds = [0.0, 0.0, 0.0, 0.0]
                    speeds[idx] = speed
                    drib = 0.0
                else:
                    if len(values) not in (4, 5):
                        print("Usage: wheel <fr> <br> <bl> <fl> [drib]")
                        continue
                    speeds = values[:4]
                    drib = values[4] if len(values) == 5 else 0.0

                # If raw, convert from int units (rad/s*100) to rad/s.
                if cmd == "wheelraw":
                    speeds = [v / 100.0 for v in speeds]
                    drib = drib

                self.send_command(
                    f"{self.args.robot_id} wheel {speeds[0]} {speeds[1]} {speeds[2]} {speeds[3]} {drib}"
                )
                continue

            print("Unknown command. Type 'help' for options.")

    def run(self):
        input_thread = threading.Thread(target=self.input_loop, daemon=True)
        input_thread.start()

        ani = animation.FuncAnimation(
            self.fig, self.update, interval=50, blit=False, cache_frame_data=False
        )
        try:
            plt.show()
        finally:
            self.stop_event.set()


def main():
    parser = argparse.ArgumentParser(description="PID visualizer with live tuning")
    parser.add_argument("--view", choices=["all", "single"], default="all",
                        help="Plot all wheels or a single wheel")
    parser.add_argument("--wheel", type=int, default=0, choices=[0, 1, 2, 3],
                        help="Wheel to plot when --view single")
    parser.add_argument("--points", type=int, default=DEFAULT_MAX_POINTS,
                        help="Max data points to keep in the plot")
    parser.add_argument("--mcast-ip", default=DEFAULT_MCAST_IP,
                        help="Multicast IP for telemetry/commands")
    parser.add_argument("--telemetry-port", type=int, default=DEFAULT_TELEMETRY_PORT,
                        help="Telemetry UDP port")
    parser.add_argument("--command-port", type=int, default=DEFAULT_COMMAND_PORT,
                        help="Command UDP port")
    parser.add_argument("--robot-id", type=int, default=DEFAULT_ROBOT_ID,
                        help="Robot ID used in command messages")
    parser.add_argument("--pid-scale", type=float, default=DEFAULT_PID_SCALE,
                        help="Quantization scale for PID values (kp_q = kp * scale)")
    args = parser.parse_args()

    print("PID Visualizer")
    print(f"Telemetry: {args.mcast_ip}:{args.telemetry_port}")
    print(f"Commands:  {args.mcast_ip}:{args.command_port}")
    print("Press Ctrl+C to exit.\n")

    viz = PIDVisualizer(args)
    viz.run()


if __name__ == "__main__":
    main()

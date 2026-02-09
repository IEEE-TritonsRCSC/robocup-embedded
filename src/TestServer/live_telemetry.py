#!/usr/bin/env python3
"""
Live telemetry viewer - plots target vs actual for all wheels.
Includes a reset button and per-wheel error readouts.
"""

import argparse
import socket
import struct
import time
from collections import deque

try:
    import matplotlib.pyplot as plt
    import matplotlib.animation as animation
    from matplotlib.widgets import Button
except ImportError as exc:
    raise SystemExit("matplotlib is required. Install with: pip install matplotlib") from exc


DEFAULT_TELEMETRY_PORT = 10001
DEFAULT_MCAST_IP = "239.42.42.42"
DEFAULT_MAX_POINTS = 600
TELEMETRY_MIN_LEN = 30

WHEEL_NAMES = ["Front-Right", "Back-Right", "Back-Left", "Front-Left"]


class LiveTelemetryPlot:
    def __init__(self, args):
        self.args = args
        self.t0 = None
        self.start_time = time.time()
        self.warned_no_telemetry = False

        # Data buffers: one deque set per wheel
        self.data = []
        for _ in range(4):
            self.data.append({
                "t": deque(maxlen=args.points),
                "target": deque(maxlen=args.points),
                "actual": deque(maxlen=args.points),
                "error": deque(maxlen=args.points),
            })

        # Telemetry socket (multicast)
        self.recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.recv_sock.bind(("", args.telemetry_port))
        mreq = struct.pack("4sl", socket.inet_aton(args.mcast_ip), socket.INADDR_ANY)
        self.recv_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.recv_sock.settimeout(0.01)

        self._setup_plot()

    def _setup_plot(self):
        self.fig, self.axes = plt.subplots(2, 2, figsize=(12, 8), sharex=True)
        self.fig.suptitle("Live Telemetry - Target vs Actual")

        self.lines = []
        for idx, ax in enumerate(self.axes.flat):
            ax.set_title(f"Wheel {idx} ({WHEEL_NAMES[idx]})", fontsize=10)
            line_target, = ax.plot([], [], "r--", label="Target")
            line_actual, = ax.plot([], [], "b-", label="Actual")
            ax.grid(True, alpha=0.3)
            if idx >= 2:
                ax.set_xlabel("Time (s)")
            if idx % 2 == 0:
                ax.set_ylabel("Speed")
            if idx == 0:
                ax.legend(loc="upper right", fontsize=8)
            self.lines.append((line_target, line_actual))

        self.error_texts = []
        x_positions = [0.02, 0.27, 0.52, 0.77]
        for i, xpos in enumerate(x_positions):
            text = self.fig.text(xpos, 0.105, f"{WHEEL_NAMES[i]} Diff=--", fontsize=9)
            self.error_texts.append(text)

        self.fig.subplots_adjust(bottom=0.2)
        reset_ax = self.fig.add_axes([0.82, 0.04, 0.12, 0.06])
        self.reset_button = Button(reset_ax, "Reset", color="#e6e6e6", hovercolor="#cfcfcf")
        self.reset_button.on_clicked(self._on_reset_clicked)

        # Avoid tight_layout warnings with manually placed button axes.

    def _reset_data(self):
        self.t0 = None
        for i in range(4):
            for key in ("t", "target", "actual", "error"):
                self.data[i][key].clear()

    def _on_reset_clicked(self, _event):
        self._reset_data()
        self.start_time = time.time()
        self.warned_no_telemetry = False
        self._update_error_texts()
        self.fig.canvas.draw_idle()

    def _format_error(self, value):
        if value is None:
            return "--"
        return f"{value:+.0f}"

    def _update_error_texts(self):
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
            wheels.append({
                "target": target,
                "actual": actual,
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

        if not received and not self.warned_no_telemetry and (time.time() - self.start_time) > 2.0:
            print(
                "No telemetry received yet. Check STM32/ESP32 telemetry firmware, "
                "multicast IP/port, and network."
            )
            self.warned_no_telemetry = True

        for i in range(4):
            times = list(self.data[i]["t"])
            line_target, line_actual = self.lines[i]
            line_target.set_data(times, list(self.data[i]["target"]))
            line_actual.set_data(times, list(self.data[i]["actual"]))
            ax = self.axes.flat[i]
            ax.relim()
            ax.autoscale_view()

        self._update_error_texts()

        artists = []
        for pair in self.lines:
            artists.extend(pair)
        artists.extend(self.error_texts)
        return tuple(artists)

    def run(self):
        animation.FuncAnimation(
            self.fig, self.update, interval=50, blit=True, cache_frame_data=False
        )
        plt.show()


def main():
    parser = argparse.ArgumentParser(description="Live telemetry plotter")
    parser.add_argument("--mcast-ip", default=DEFAULT_MCAST_IP,
                        help="Multicast IP for telemetry")
    parser.add_argument("--telemetry-port", type=int, default=DEFAULT_TELEMETRY_PORT,
                        help="Telemetry UDP port")
    parser.add_argument("--points", type=int, default=DEFAULT_MAX_POINTS,
                        help="Max data points to keep in the plot")
    args = parser.parse_args()

    print("Live Telemetry")
    print(f"Telemetry: {args.mcast_ip}:{args.telemetry_port}")
    print("Press Ctrl+C to exit.\n")

    plotter = LiveTelemetryPlot(args)
    plotter.run()


if __name__ == "__main__":
    main()

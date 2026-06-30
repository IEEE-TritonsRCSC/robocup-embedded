"""
send_gui.py — CustomTkinter GUI for RoboCup UDP Control
Wraps send.py's UDP logic with a full graphical interface.
"""

import threading
import time
from datetime import datetime

import customtkinter as ctk

from send import get_local_ip, send_command

# ── Appearance ────────────────────────────────────────────────────────────────
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

ROBOT_IP_DEFAULT = "192.168.8.50" 
UDP_PORT_DEFAULT = "10000"
MACRO_DELAY_S = 1.0  # seconds between macro steps


# ── Helpers ───────────────────────────────────────────────────────────────────

def timestamp() -> str:
    return datetime.now().strftime("%H:%M:%S.%f")[:-3]


def build_message(robot_id: str, command: str, *args: str) -> str:
    parts = [robot_id, command] + [str(a) for a in args]
    return " ".join(parts)


# ── Main Application ──────────────────────────────────────────────────────────

class RoboCupGUI(ctk.CTk):
    def __init__(self) -> None:
        super().__init__()
        self.title("RoboCup UDP Controller")
        self.resizable(False, False)

        # Threading state for macro
        self._macro_thread: threading.Thread | None = None
        self._macro_cancel = threading.Event()

        self._build_ui()

    # ── UI Construction ───────────────────────────────────────────────────────

    def _build_ui(self) -> None:
        PAD = {"padx": 10, "pady": 6}

        # ── Connection Frame ──────────────────────────────────────────────────
        conn_frame = ctk.CTkFrame(self)
        conn_frame.grid(row=0, column=0, columnspan=2, sticky="ew", **PAD)
        conn_frame.columnconfigure((1, 3, 5), weight=1)

        ctk.CTkLabel(conn_frame, text="Target IP:").grid(row=0, column=0, padx=(10, 4), pady=8, sticky="e")
        self.ip_entry = ctk.CTkEntry(conn_frame, width=150, placeholder_text=ROBOT_IP_DEFAULT)
        self.ip_entry.insert(0, ROBOT_IP_DEFAULT)
        self.ip_entry.grid(row=0, column=1, padx=4, pady=8, sticky="w")

        ctk.CTkLabel(conn_frame, text="Port:").grid(row=0, column=2, padx=(16, 4), pady=8, sticky="e")
        self.port_entry = ctk.CTkEntry(conn_frame, width=80, placeholder_text=UDP_PORT_DEFAULT)
        self.port_entry.insert(0, UDP_PORT_DEFAULT)
        self.port_entry.grid(row=0, column=3, padx=4, pady=8, sticky="w")

        ctk.CTkLabel(conn_frame, text="Robot ID:").grid(row=0, column=4, padx=(16, 4), pady=8, sticky="e")
        self.robot_id_var = ctk.StringVar(value="1")
        self.robot_id_menu = ctk.CTkOptionMenu(conn_frame, values=[str(i) for i in range(1, 7)],
                                                variable=self.robot_id_var, width=70)
        self.robot_id_menu.grid(row=0, column=5, padx=(4, 10), pady=8, sticky="w")

        # Local IP display
        try:
            local_ip = get_local_ip()
        except Exception:
            local_ip = "Unknown"
        self.local_ip_label = ctk.CTkLabel(conn_frame, text=f"Local IP: {local_ip}",
                                            text_color="gray60", font=ctk.CTkFont(size=11))
        self.local_ip_label.grid(row=1, column=0, columnspan=6, padx=10, pady=(0, 6), sticky="w")

        # ── Quick Commands Frame ──────────────────────────────────────────────
        quick_frame = ctk.CTkFrame(self)
        quick_frame.grid(row=1, column=0, sticky="nsew", **PAD)

        ctk.CTkLabel(quick_frame, text="Quick Commands",
                     font=ctk.CTkFont(weight="bold")).grid(row=0, column=0, columnspan=2, pady=(8, 4))

        btn_cfg = [
            ("⛔  STOP",  "q", "red",    "#c0392b"),
            ("🦵  KICK",  "k", "#e67e22", "#d35400"),
            ("🤲  CATCH", "c", "green",  "#1e8449"),
            ("🫳  DROP",  "o", "#7d3c98", "#6c3483"),
        ]
        for i, (label, cmd, fg, hover) in enumerate(btn_cfg):
            row, col = divmod(i, 2)
            ctk.CTkButton(
                quick_frame, text=label, width=130, height=45,
                fg_color=fg, hover_color=hover,
                font=ctk.CTkFont(size=14, weight="bold"),
                command=lambda c=cmd: self._send(c),
            ).grid(row=row + 1, column=col, padx=8, pady=5)

        # ── Parametric Commands Frame ─────────────────────────────────────────
        param_frame = ctk.CTkFrame(self)
        param_frame.grid(row=1, column=1, sticky="nsew", **PAD)

        ctk.CTkLabel(param_frame, text="Parametric Commands",
                     font=ctk.CTkFont(weight="bold")).grid(row=0, column=0, columnspan=3, pady=(8, 4))

        # Dash row
        ctk.CTkLabel(param_frame, text="DASH", font=ctk.CTkFont(weight="bold", size=13)).grid(
            row=1, column=0, padx=(10, 4), pady=6, sticky="e")
        self.dash_power = ctk.CTkEntry(param_frame, width=80, placeholder_text="power")
        self.dash_power.insert(0, "1.0")
        self.dash_power.grid(row=1, column=1, padx=4, pady=6)
        self.dash_angle = ctk.CTkEntry(param_frame, width=80, placeholder_text="angle")
        self.dash_angle.insert(0, "0")
        self.dash_angle.grid(row=1, column=2, padx=4, pady=6)
        ctk.CTkButton(param_frame, text="Send", width=70,
                      command=self._send_dash).grid(row=1, column=3, padx=(4, 10), pady=6)

        # Turn row
        ctk.CTkLabel(param_frame, text="TURN", font=ctk.CTkFont(weight="bold", size=13)).grid(
            row=2, column=0, padx=(10, 4), pady=6, sticky="e")
        self.turn_speed = ctk.CTkEntry(param_frame, width=80, placeholder_text="speed")
        self.turn_speed.insert(0, "-90")
        self.turn_speed.grid(row=2, column=1, padx=4, pady=6)
        ctk.CTkButton(param_frame, text="Send", width=70,
                      command=self._send_turn).grid(row=2, column=3, padx=(4, 10), pady=6)

        # ── Sequence Macro Frame ──────────────────────────────────────────────
        macro_frame = ctk.CTkFrame(self)
        macro_frame.grid(row=2, column=0, columnspan=2, sticky="ew", **PAD)
        macro_frame.columnconfigure((1, 2, 4, 5), weight=1)

        ctk.CTkLabel(macro_frame, text="Sequence Macro",
                     font=ctk.CTkFont(weight="bold")).grid(row=0, column=0, columnspan=7, pady=(8, 2))

        ctk.CTkLabel(macro_frame, text="Sequence:  Dash 1  →  Turn  →  Catch  →  Drop  →  Stop  →  Dash 2",
                     text_color="gray60", font=ctk.CTkFont(size=11)).grid(
            row=1, column=0, columnspan=7, pady=(0, 6))

        # Dash 1
        ctk.CTkLabel(macro_frame, text="Dash 1", font=ctk.CTkFont(weight="bold")).grid(
            row=2, column=0, padx=(10, 4), pady=5, sticky="e")
        self.m_dash1_power = ctk.CTkEntry(macro_frame, width=80, placeholder_text="power")
        self.m_dash1_power.insert(0, "1.0")
        self.m_dash1_power.grid(row=2, column=1, padx=4, pady=5)
        self.m_dash1_angle = ctk.CTkEntry(macro_frame, width=80, placeholder_text="angle")
        self.m_dash1_angle.insert(0, "0")
        self.m_dash1_angle.grid(row=2, column=2, padx=4, pady=5)

        # Turn
        ctk.CTkLabel(macro_frame, text="Turn", font=ctk.CTkFont(weight="bold")).grid(
            row=2, column=3, padx=(16, 4), pady=5, sticky="e")
        self.m_turn_speed = ctk.CTkEntry(macro_frame, width=80, placeholder_text="speed")
        self.m_turn_speed.insert(0, "-90")
        self.m_turn_speed.grid(row=2, column=4, padx=4, pady=5)

        # Dash 2
        ctk.CTkLabel(macro_frame, text="Dash 2", font=ctk.CTkFont(weight="bold")).grid(
            row=2, column=5, padx=(16, 4), pady=5, sticky="e")
        self.m_dash2_power = ctk.CTkEntry(macro_frame, width=80, placeholder_text="power")
        self.m_dash2_power.insert(0, "1.0")
        self.m_dash2_power.grid(row=2, column=6, padx=4, pady=5)
        self.m_dash2_angle = ctk.CTkEntry(macro_frame, width=80, placeholder_text="angle")
        self.m_dash2_angle.insert(0, "45")
        self.m_dash2_angle.grid(row=2, column=7, padx=(4, 4), pady=5)

        # Run / Cancel buttons
        btn_row = ctk.CTkFrame(macro_frame, fg_color="transparent")
        btn_row.grid(row=3, column=0, columnspan=8, pady=(4, 10))
        self.run_macro_btn = ctk.CTkButton(
            btn_row, text="▶  Run Macro", width=140, height=36,
            fg_color="#1a7a4a", hover_color="#145e38",
            font=ctk.CTkFont(weight="bold"),
            command=self._run_macro,
        )
        self.run_macro_btn.grid(row=0, column=0, padx=8)
        self.cancel_macro_btn = ctk.CTkButton(
            btn_row, text="✕  Cancel", width=100, height=36,
            fg_color="#7f1c1c", hover_color="#5e1515",
            state="disabled",
            command=self._cancel_macro,
        )
        self.cancel_macro_btn.grid(row=0, column=1, padx=8)
        self.macro_status_label = ctk.CTkLabel(btn_row, text="", text_color="gray60",
                                               font=ctk.CTkFont(size=11))
        self.macro_status_label.grid(row=0, column=2, padx=8)

        # ── Activity Log ──────────────────────────────────────────────────────
        log_frame = ctk.CTkFrame(self)
        log_frame.grid(row=3, column=0, columnspan=2, sticky="nsew", **PAD)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(1, weight=1)

        log_header = ctk.CTkFrame(log_frame, fg_color="transparent")
        log_header.grid(row=0, column=0, sticky="ew", padx=8, pady=(6, 0))
        ctk.CTkLabel(log_header, text="Activity Log",
                     font=ctk.CTkFont(weight="bold")).pack(side="left")
        ctk.CTkButton(log_header, text="Clear", width=60, height=24,
                      command=self._clear_log).pack(side="right")

        self.log_box = ctk.CTkTextbox(log_frame, width=700, height=160,
                                      font=ctk.CTkFont(family="Courier", size=12),
                                      state="disabled")
        self.log_box.grid(row=1, column=0, sticky="nsew", padx=8, pady=(4, 8))

        self.columnconfigure((0, 1), weight=1)
        self.rowconfigure(3, weight=1)

    # ── Send Logic ────────────────────────────────────────────────────────────

    def _current_ip(self) -> str:
        return self.ip_entry.get().strip() or ROBOT_IP_DEFAULT

    def _current_port(self) -> int:
        try:
            return int(self.port_entry.get().strip())
        except ValueError:
            return int(UDP_PORT_DEFAULT)

    def _current_robot_id(self) -> str:
        return self.robot_id_var.get()

    def _dispatch(self, command: str, *args: str) -> None:
        """Build, send, and log a UDP command. Patches send.py globals at call time."""
        import send as _send_module
        _send_module.ROBOT_IP = self._current_ip()
        _send_module.UDP_PORT = self._current_port()

        message = build_message(self._current_robot_id(), command, *args)
        try:
            send_command(message)
            self._log(f"✅  Sent:  {message}")
        except Exception as exc:
            self._log(f"❌  Error: {exc}")

    def _send(self, command: str, *args: str) -> None:
        self._dispatch(command, *args)

    def _send_dash(self) -> None:
        power = self.dash_power.get().strip()
        angle = self.dash_angle.get().strip()
        if not power or not angle:
            self._log("⚠️  Dash requires both power and angle.")
            return
        self._dispatch("d", power, angle)

    def _send_turn(self) -> None:
        speed = self.turn_speed.get().strip()
        if not speed:
            self._log("⚠️  Turn requires a speed value.")
            return
        self._dispatch("t", speed)

    # ── Macro Logic ───────────────────────────────────────────────────────────

    def _collect_macro_params(self) -> dict | None:
        """Validate and collect all macro input fields. Returns None on error."""
        fields = {
            "dash1_power": self.m_dash1_power.get().strip(),
            "dash1_angle": self.m_dash1_angle.get().strip(),
            "turn_speed":  self.m_turn_speed.get().strip(),
            "dash2_power": self.m_dash2_power.get().strip(),
            "dash2_angle": self.m_dash2_angle.get().strip(),
        }
        for key, val in fields.items():
            if not val:
                self._log(f"⚠️  Macro: '{key}' cannot be empty.")
                return None
        return fields

    def _run_macro(self) -> None:
        if self._macro_thread and self._macro_thread.is_alive():
            return  # Already running

        params = self._collect_macro_params()
        if params is None:
            return

        self._macro_cancel.clear()
        self.run_macro_btn.configure(state="disabled")
        self.cancel_macro_btn.configure(state="normal")
        self._update_macro_status("Running…")

        self._macro_thread = threading.Thread(
            target=self._macro_worker,
            args=(params,),
            daemon=True,
        )
        self._macro_thread.start()

    def _macro_worker(self, params: dict) -> None:
        """Runs the 6-step macro sequence on a background thread."""
        steps = [
            ("d", params["dash1_power"], params["dash1_angle"]),
            ("t", params["turn_speed"]),
            ("c",),
            ("o",),
            ("q",),
            ("d", params["dash2_power"], params["dash2_angle"]),
        ]
        step_names = ["Dash 1", "Turn", "Catch", "Drop", "Stop", "Dash 2"]

        for i, (step, name) in enumerate(zip(steps, step_names), start=1):
            if self._macro_cancel.is_set():
                self._log("🚫  Macro cancelled.")
                break

            self._update_macro_status(f"Step {i}/6 — {name}")
            self._dispatch(*step)

            # Wait MACRO_DELAY_S but remain responsive to cancel
            if i < len(steps):
                cancelled = self._macro_cancel.wait(timeout=MACRO_DELAY_S)
                if cancelled:
                    self._log("🚫  Macro cancelled.")
                    break
        else:
            self._update_macro_status("Complete ✅")
            self._log("🏁  Macro sequence complete.")

        self.after(0, self._macro_finished)

    def _cancel_macro(self) -> None:
        self._macro_cancel.set()

    def _macro_finished(self) -> None:
        self.run_macro_btn.configure(state="normal")
        self.cancel_macro_btn.configure(state="disabled")

    def _update_macro_status(self, text: str) -> None:
        """Thread-safe status label update."""
        self.after(0, lambda: self.macro_status_label.configure(text=text))

    # ── Log Helpers ───────────────────────────────────────────────────────────

    def _log(self, message: str) -> None:
        """Thread-safe append to the activity log."""
        entry = f"[{timestamp()}]  {message}\n"
        self.after(0, self._append_log, entry)

    def _append_log(self, entry: str) -> None:
        self.log_box.configure(state="normal")
        self.log_box.insert("end", entry)
        self.log_box.see("end")
        self.log_box.configure(state="disabled")

    def _clear_log(self) -> None:
        self.log_box.configure(state="normal")
        self.log_box.delete("1.0", "end")
        self.log_box.configure(state="disabled")


# ── Entry Point ───────────────────────────────────────────────────────────────

if __name__ == "__main__":
    app = RoboCupGUI()
    app.mainloop()
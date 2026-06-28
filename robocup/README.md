# RoboCup Embedded UDP Control

This firmware receives plain-text UDP packets over Wi-Fi and translates them into Moteus position commands for the robot.

## Overview

- Target board: Arduino UNO R4 WiFi
- Transport: UDP
- Default robot ID: `1`
- Default UDP port: `10000`
- Default robot IP: `192.168.68.50`

The sketch lives in [`robocup.ino`](robocup.ino), and the UDP command parsing / actuator helpers live in [`commands.cpp`](commands.cpp) and [`commands.h`](commands.h).

## Build Flags

[`commands.h`](commands.h) controls the main compile-time switches:

- `ENABLE_MOTORS`
  - `1` enables CAN initialization and motor output
  - `0` keeps the sketch from sending commands to motor hardware
- `ENABLE_TEST_MOTORS`
  - `1` compiles the test-motor layout
  - `0` compiles the full robot layout
- `NUM_TEST_MOTORS`
  - Used only when `ENABLE_TEST_MOTORS == 1`
  - Defaults to `1` if it is not defined elsewhere

When test mode is enabled:

- `NUM_MOTORS` and `NUM_WHEELS` both follow `NUM_TEST_MOTORS`
- The dribbler is only available if the build includes motor index `4`

When full mode is enabled:

- `NUM_MOTORS = 5`
- `NUM_WHEELS = 4`

## Network Setup

The sketch uses a static IP configuration via `WiFiS3`:

- Arduino IP: `192.168.68.50`
- Gateway: `192.168.68.1`
- Subnet mask: `255.255.255.0`
- UDP port: `10000`

The sender script in [`send.py`](send.py) is configured for the same IP and port.

If your network changes, update:

- `LOCAL_IP_ADDRESS` and `GATEWAY_IP_ADDRESS` in [`credentials.h`](credentials.h) or the file where they are defined
- `ROBOT_IP` in [`send.py`](send.py)

## Packet Format

UDP packets are whitespace-separated plain text in this format:

```text
<robot_id> <command> [arg1] [arg2]
```

Rules:

- `robot_id` must match `ROBOT_ID` in [`commands.h`](commands.h)
- Extra whitespace is ignored
- Incomplete packets are rejected before execution

## Commands

Supported command characters:

- `d` = Dash, requires 2 args
- `t` = Turn, requires 1 arg
- `k` = Kick, requires 0 args
- `c` = Catch dribbler, requires 0 args
- `o` = Drop dribbler, requires 0 args
- `q` = Stop all motors, requires 0 args

Examples:

```text
1 q
1 c
1 d 1.0 30
1 t -90
```

## Python Sender

Use [`send.py`](send.py) to send commands from your computer.

It prints:

- your local IP address, labeled by the script as `Gateway IP: ...`
- the destination as `Sending to: 192.168.68.50:10000`
- the final UDP payload before transmission

Examples:

```bash
python send.py 1 q
python send.py 1 c
python send.py 1 d 1.0 30
python send.py 1 t -90
```

## Runtime Behavior

When a valid packet is received:

- `d` calls `dash(power, angle)`
- `t` calls `turn(turnSpeed)`
- `k` triggers the kicker GPIO pulse
- `c` starts the dribbler if the build includes one
- `o` stops the dribbler if the build includes one
- `q` stops all motor commands

The firmware also runs a watchdog:

- If no valid UDP command arrives within `WATCHDOG_TIMEOUT` milliseconds, the robot is stopped
- The default timeout is `4000` ms

If `ENABLE_MOTORS` is `0`, the sketch still parses UDP packets and logs them, but it does not initialize CAN or send motor commands.

## Notes

- `dash()` scales the requested power into the configured velocity range and maps it onto the wheel angles defined in [`commands.h`](commands.h).
- `turn()` currently uses an experimental scaling factor in [`commands.cpp`](commands.cpp).
- The firmware prints wheel and motor velocity snapshots over Serial when commands are accepted.
- Wi-Fi credentials live in [`credentials.h`](credentials.h).

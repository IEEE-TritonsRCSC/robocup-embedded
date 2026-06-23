# RoboCup Embedded UDP Control

This sketch receives simple UDP commands over Wi-Fi and converts them into Moteus position commands for the robot.

## Network Setup

The Arduino sketch uses a static IP configuration:

- Arduino IP: `192.168.68.50`
- Gateway: `192.168.68.1`
- Subnet mask: `255.255.255.0`
- UDP port: `10000`

The sender script in [`send.py`](send.py) is configured to send commands to `192.168.68.50:10000`.

If your network changes, update both:

- `LOCAL_IP_ADDRESS` and `GATEWAY_IP_ADDRESS` in [`robocup.ino`](robocup.ino)
- `ROBOT_IP` in [`send.py`](send.py)

## Command Format

Commands are sent as plain text UDP packets using this format:

```text
<Robot ID> <Command Character> <arg1> <arg2>
```

- Robot ID: `1` to `6`
- The sketch only executes commands when the ID matches `ROBOT_ID` in [`robocup.ino`](robocup.ino)

## Commands

- `d` = Dash, 2 args
- `t` = Turn, 1 arg
- `k` = Kick, 0 args
- `c` = Catch, 0 args
- `o` = Drop, 0 args
- `q` = Stop, 0 args

Examples:

```text
1 q
1 c
1 d 1.0 30
1 t -90
```

## Python Sender

Use [`send.py`](send.py) to send commands from your computer.

It prints your local IP address before sending the packet, which is useful for confirming the interface being used.

Examples:

```bash
python send.py 1 q
python send.py 1 c
python send.py 1 d 1.0 30
python send.py 1 t -90
```

## Robot Behavior

When a valid UDP packet is received:

- `d` calls `dash(power, angle)`
- `t` calls `turn(turnSpeed)`
- `k` triggers the kicker pin
- `c` starts the dribbler
- `o` stops the dribbler
- `q` stops all motor commands

## Notes

- The sketch currently updates cached `PositionCommand` values and then prints the live velocities over Serial.
- If the Moteus objects are not initialized yet, `sendPositionCommands()` safely returns without sending.
- The current Wi-Fi credentials live in [`robocup.ino`](robocup.ino).

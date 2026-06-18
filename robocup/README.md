# RoboCup Embedded

Arduino sketch for the RoboCup robot motor controller. This project uses an
Arduino UNO R4 WiFi with an ACAN2517FD CAN-FD shield and Moteus motor
controllers to drive the robot's wheels, dribbler, and kicker.

## What this project does

- Initializes the CAN-FD bus at 1 Mbit/s.
- Creates `Moteus` objects for the configured motors.
- Provides helpers for locomotion, turning, dribbler control, and kicking.
- Currently keeps the robot in a safe stopped state in `loop()`.

## Repository Layout

- `robocup.ino` - Arduino entry point and startup logic.
- `MotorControl.h` - Shared constants and function declarations.
- `MotorControl.cpp` - Motor helper implementations.
- `.vscode/` - Arduino IDE / VS Code configuration.

## Hardware Assumptions

- Board: `arduino:avr:unowifi`
- CAN controller: `MCP2517` / `ACAN2517FD`
- Motor controller library: `Moteus`

## Pin Configuration

The current sketch uses these pins:

- `MCP2517_CS = 10`
- `MCP2517_INT = 2`
- `MCP2517_SDI = 11`
- `MCP2517_SDO = 12`
- `MCP2517_SCK = 13`
- `KICKER_PIN = 13`

> Note: `KICKER_PIN` currently shares pin `13` with `MCP2517_SCK`. If that is
> not intentional for your wiring, change one of them before deploying to
> hardware.

## Motor Mapping

Motor indices are defined in `MotorControl.h`:

- `0` to `3` are wheel motors
- `4` is the dribbler motor

The startup code in `robocup.ino` currently creates only `TEST_NUM_MOTORS`
motors, which is set to `2`. That makes this sketch behave like a reduced test
setup unless you change the constant.

## Build Requirements

Install these Arduino libraries:

- `Moteus`
- `ACAN2517FD`

The included `.vscode/arduino.json` and `sketch.yaml.disabled` show the intended
board profile and library versions.

## Build and Upload

1. Open the `robocup` folder in Arduino IDE or VS Code with the Arduino
   extension.
2. Select the board profile for `Arduino UNO R4 WiFi`.
3. Make sure the `Moteus` and `ACAN2517FD` libraries are installed.
4. Build and upload the sketch.
5. Open the serial monitor at `115200` baud if you want startup status and CAN
   error messages.

## Current Behavior

On boot, the sketch:

1. Initializes the serial port at `115200`.
2. Starts SPI.
3. Brings up the ACAN2517FD CAN interface.
4. Creates the configured `Moteus` motor objects.
5. Sends each motor a `BeginStop()` command.

The main loop currently calls:

```cpp
stop(motors, KICKER_PIN);
```

So the robot is held in a stopped state by default.

## Motion Helpers

`MotorControl.cpp` provides helper functions that can be wired into higher-level
robot behavior:

- `dash(...)` - drive the wheel motors in a chosen direction
- `turn(...)` - rotate the robot in place
- `dribblerCatch(...)` - spin the dribbler to retain the ball
- `shortKick(...)` - briefly reverse the dribbler to push the ball forward
- `kick(...)` - pulse the kicker output
- `stop(...)` - stop locomotion, dribbler, and kicker

## Notes

- `dash()` and `turn()` currently iterate over `TEST_NUM_MOTORS`, so they are
  configured for the current test setup rather than all four wheel motors.
- `kick()` toggles the kicker pin high and immediately low. If the solenoid
  needs a real pulse width, add a delay or timer-based pulse logic.
- Several constants in `MotorControl.h` are tuning values and may need to be
  adjusted on the robot.

## Next Steps

If you want, I can also write a more polished README with:

1. a wiring diagram section,
2. a command reference for each helper function, or
3. a quick-start guide for your exact hardware setup.

# Changelog

## Recent Changes

### ESP32 Firmware

#### Major Additions
- **Web Server with REST API** (`src/esp32/robocup.ino`)
  - Added web server with SSE (Server-Sent Events) telemetry streaming
  - New endpoints:
    - `GET /` - PID tuning dashboard with web UI
    - `GET /api/telemetry` - Returns current telemetry data as JSON
    - `POST /api/pid` - Update PID parameters (kp, ki, kd) for specific wheels
    - `POST /api/setpoint` - Send velocity commands (u, v, w)
    - `GET /api/header` - Get current communication header bytes
    - `POST /api/header` - Update header bytes with persistence
  - Telemetry mirroring over SSE/UDP

#### Refactoring & Cleanup
- **Restructured project layout** - Moved files from `lukas/robocup/` to `src/esp32/`
- **Updated helpers** (`src/esp32/helpers.cpp`, `src/esp32/helpers.h`)
  - Added STM32 telemetry frame parsing
  - Added velocity conversion utilities
  - Enhanced header configuration management with NVS persistence

### Removed Files

#### Cleanup of Temporary/Old Code
- Removed `parserChange/` directory and all contents:
  - `parserChange/robocup/credentials.h`
  - `parserChange/robocup/robocup.ino`
  - `parserChange/robocup/velocityConversions.cpp/.h`
  - `parserChange/robocup/wifi_debug_snippet.txt`
  - `parserChange/velocityConversions.cpp/.h`

#### Test Code Cleanup
Removed obsolete test files:
- `test_code/dribblerControl/dribblerControl.ino`
- `test_code/esp32/RecieveUART/RecieveUART.ino`
- `test_code/esp32/TransmitUART/TransmitUART.ino`
- `test_code/esp32/espInterrupt.ino`
- `test_code/esp32/espInterruptMultithreaded.ino`
- `test_code/esp32/velocityConversionsRework/velocityConversionsRework.cpp/.h`
- `test_code/remote controller/TestProtobufServer.py`
- `test_code/remote controller/sketch_jan31b/` (PID.cpp, PID.h, sketch_jan31b.ino, testing.cpp, testing.h)
- `test_code/stm32/SendRecieveUART/main.c`
- `test_code/stm32/UARTReceiveInterruptTest_WIP.c`
- `test_code/stm32/UARTReceiveTest.c`
- `test_code/stm32/receive_checkCA.c`

#### Protobuf Cleanup
Removed unused generated protobuf files from `src/protobuf/classes/`:
- All SSL simulation and game controller proto files
- Vision detection and geometry protos
- AI debug info and coordinated pass protos

### STM32 Drivetrain (`src/drivetrain/Core/Src/main.c`)

#### Telemetry System
- Added periodic telemetry transmission via UART4
  - Frame structure: header (2 bytes) + timestamp (4 bytes) + 4 wheel data blocks (6 bytes each) + dribbler (2 bytes) = 30 bytes
  - Each wheel reports: target speed, actual speed (×100), PID output (int16)
  - Transmission interval: configurable via `TELEMETRY_INTERVAL_MS`

#### UART Command Interface
- Enhanced packet parsing with configurable header bytes
- Added big-endian float parsing for velocity commands
- New command types supported:
  - Motor velocity commands (header + 3 floats for u, v, w)
  - PID updates (header + wheel_id + kp, ki, kd floats)
  - Header configuration commands

#### Header Persistence (Backup SRAM)
- Added backup domain access for persistent storage across resets
- Header bytes stored with checksum validation
- Supports legacy header fallback with grace period
- Storage format: magic (4) + version (4) + active_pair (4) + legacy_pair (4) + legacy_enabled (4) + checksum (4) = 24 bytes

#### PID Control Updates
- Default PID gains changed to Kp=100.0, Ki=0.0, Kd=0.0 for all wheels
- PID output now clamped to int16 range for telemetry
- Added `send_telemetry_if_due()` called in main loop

#### New Helper Functions
- `bytes_to_float_be()` - Convert big-endian bytes to float
- `compose_telemetry_frame()` - Build telemetry packet
- `clamp_to_int16()` - Clamp float to int16 range
- `load_persisted_header()` / `persist_header_state()` - Backup SRAM management
- `pack_header_pair()` / `unpack_header_pair()` - Header encoding/decoding


### ESP32 Firmware

#### New Web Server (`src/esp32/robocup.ino`)
- Full-featured async web server on port 80
- Embedded PID tuning dashboard (HTML/CSS/JS in PROGMEM)
- Real-time telemetry via Server-Sent Events (`/telemetry`)

#### REST API Endpoints
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | PID tuning dashboard web UI |
| GET | `/api/telemetry` | JSON snapshot of latest telemetry |
| POST | `/api/pid` | Update PID gains (kp, ki, kd) for specific wheel or all wheels |
| POST | `/api/setpoint` | Send velocity command (u, v, w in m/s and rad/s) |
| GET | `/api/header` | Get current UART header bytes |
| POST | `/api/header` | Update header bytes (persisted to NVS) |

#### Data Structures
- `WheelTelemetry` - target, actual, output (int16)
- `TelemetrySnapshot` - timestamp + 4 wheels + dribbler
- `HeaderBytes` - byte1, byte2

#### STM32 UART Communication
- Velocity commands sent as big-endian floats
- PID updates sent with wheel_id (0-3 or 0xFF for all)
- Header configuration commands update STM32 backup SRAM

#### Telemetry Processing
- Async SSE streaming to connected clients
- UDP multicast mirroring (239.42.42.42)
- Thread-safe snapshot copying with mutex
- JSON serialization for API responses

#### Header Persistence (NVS)
- Uses Preferences library for header byte storage
- Namespace: "uart_hdr", keys: "h1", "h2"
- Loaded on boot and applied to motor command frames


## Summary
- **Total**: 76 files changed, 1,237 insertions(+), 6,302 deletions(-)
- Focus on production-ready ESP32 firmware with web-based PID tuning interface
- Significant cleanup of test code and generated protobuf files
- Streamlined project structure

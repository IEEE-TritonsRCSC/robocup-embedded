#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <pgmspace.h>
#include "helpers.h"
#include "credentials.h"
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
}

#define BAUD_RATE 115200

WiFiUDP UDP;
WiFiUDP telemetryUdp;
IPAddress multicastIP(239, 42, 42, 42);
HardwareSerial robotSerial(2);
AsyncWebServer server(80);
AsyncEventSource telemetryEvents("/telemetry");

uint16_t packet_size;
char packet_buffer[MAX_PACKET_SIZE];
unsigned long packet_time;
std::array<uint8_t, MOTOR_COMMAND_SIZE> motor_command;
std::array<uint8_t, MOTOR_CMD_HEADER_SIZE> motor_cmd_headers = {DEFAULT_HEADER_BYTE_1, DEFAULT_HEADER_BYTE_2};

struct WheelTelemetry {
  int16_t target;
  int16_t actual;
  int16_t output;
};

struct TelemetrySnapshot {
  uint32_t timestamp_ms;
  WheelTelemetry wheels[TELEMETRY_WHEEL_COUNT];
  int16_t dribbler;
};

struct HeaderBytes {
  uint8_t byte1;
  uint8_t byte2;
};

Preferences headerPrefs;
bool headerPrefsReady = false;
const char *const HEADER_PREF_NAMESPACE = "uart_hdr";
const char *const HEADER_PREF_KEY_1 = "h1";
const char *const HEADER_PREF_KEY_2 = "h2";

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>PID Web Tuning</title>
  <style>
    :root {
      --bg: #030712;
      --panel: rgba(11, 22, 44, 0.82);
      --accent: #69f7be;
      --accent-2: #f2a365;
      --text: #f5f7ff;
      --muted: #92a1c6;
      font-family: "Space Grotesk", "Segoe UI", system-ui, -apple-system, BlinkMacSystemFont, sans-serif;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      background: radial-gradient(circle at top, #0f1c3f 0%, #050913 70%, #010205 100%);
      color: var(--text);
    }
    main { max-width: 1200px; margin: 0 auto; padding: 2.5rem 1.5rem 3rem; }
    header h1 { margin: 0; font-size: clamp(1.8rem, 4vw, 2.8rem); }
    header p { color: var(--muted); margin-top: 0.4rem; }
    .status-pill {
      display: inline-flex;
      align-items: center;
      padding: 0.25rem 0.85rem;
      border-radius: 999px;
      background: rgba(255, 255, 255, 0.08);
      font-size: 0.85rem;
      margin-top: 0.75rem;
    }
    .status-pill.success { color: var(--accent); }
    .status-pill.error { color: #ff7b7b; }
    .panel {
      background: var(--panel);
      border: 1px solid rgba(255, 255, 255, 0.08);
      border-radius: 18px;
      padding: 1.5rem;
      box-shadow: 0 25px 50px rgba(0, 0, 0, 0.35);
      backdrop-filter: blur(12px);
      margin-top: 1.75rem;
    }
    .panel h2 {
      margin: 0 0 1rem 0;
      font-size: 1.1rem;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      color: var(--accent);
    }
    table { width: 100%; border-collapse: collapse; }
    thead th {
      text-align: left;
      font-size: 0.8rem;
      letter-spacing: 0.1em;
      color: var(--muted);
      padding-bottom: 0.5rem;
    }
    tbody td {
      padding: 0.5rem 0;
      border-top: 1px solid rgba(255, 255, 255, 0.08);
    }
    tbody td.wheel-name { font-weight: 600; color: var(--text); }
    #dribbler { margin-top: 0.75rem; color: var(--muted); }
    .grid { display: grid; gap: 1.5rem; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); }
    label {
      display: flex;
      flex-direction: column;
      font-size: 0.8rem;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      margin-bottom: 1rem;
      color: var(--muted);
    }
    input, select {
      margin-top: 0.35rem;
      padding: 0.65rem 0.75rem;
      border-radius: 12px;
      border: 1px solid rgba(255, 255, 255, 0.12);
      background: rgba(4, 6, 16, 0.7);
      color: var(--text);
      font-size: 0.95rem;
    }
    input:focus, select:focus { outline: 2px solid var(--accent); }
    button {
      width: 100%;
      padding: 0.9rem;
      border: none;
      border-radius: 12px;
      font-size: 0.95rem;
      font-weight: 600;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      background: linear-gradient(135deg, var(--accent), var(--accent-2));
      color: #03111f;
      cursor: pointer;
      transition: transform 0.2s ease, box-shadow 0.2s ease;
    }
    button:hover { transform: translateY(-1px); box-shadow: 0 15px 35px rgba(105, 247, 190, 0.35); }
    .form-status { min-height: 1.2rem; font-size: 0.85rem; }
    .form-status.success { color: var(--accent); }
    .form-status.error { color: #ff7b7b; }
    @media (max-width: 600px) {
      main { padding: 2rem 1rem 2.5rem; }
      .panel { padding: 1.25rem; }
    }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>PID Web Tuning Console</h1>
      <p>Monitor wheel telemetry in real time and push new PID gains or velocity commands directly from your browser.</p>
      <span class="status-pill" id="telemetry-status">Connecting to telemetry…</span>
    </header>

    <section class="panel">
      <h2>Live Telemetry</h2>
      <table>
        <thead>
          <tr><th>Wheel</th><th>Target</th><th>Actual</th><th>PID Output</th></tr>
        </thead>
        <tbody id="telemetry-body"></tbody>
      </table>
      <div id="dribbler">Dribbler: --</div>
    </section>

    <section class="grid">
      <div class="panel">
        <h2>PID Gains</h2>
        <form id="pid-form">
          <label>Wheel
            <select id="wheel-select">
              <option value="-1">All Wheels</option>
              <option value="0">Front Right</option>
              <option value="1">Back Right</option>
              <option value="2">Back Left</option>
              <option value="3">Front Left</option>
            </select>
          </label>
          <label>Kp<input type="number" step="0.01" id="kp-input" required></label>
          <label>Ki<input type="number" step="0.001" id="ki-input" required></label>
          <label>Kd<input type="number" step="0.001" id="kd-input" required></label>
          <button type="submit">Send PID Gains</button>
          <p class="form-status" id="pid-status"></p>
        </form>
      </div>

      <div class="panel">
        <h2>Velocity Setpoint</h2>
        <form id="setpoint-form">
          <label>Linear U (m/s)<input type="number" step="0.01" id="u-input" value="0"></label>
          <label>Linear V (m/s)<input type="number" step="0.01" id="v-input" value="0"></label>
          <label>Angular W (rad/s)<input type="number" step="0.01" id="w-input" value="0"></label>
          <button type="submit">Send Command</button>
          <p class="form-status" id="setpoint-status"></p>
        </form>
      </div>

      <div class="panel">
        <h2>UART Header</h2>
        <form id="header-form">
          <label>Byte 1 (hex)
            <input type="text" id="header1" value="CA" maxlength="2" pattern="[0-9a-fA-F]{2}" required>
          </label>
          <label>Byte 2 (hex)
            <input type="text" id="header2" value="FE" maxlength="2" pattern="[0-9a-fA-F]{2}" required>
          </label>
          <button type="submit">Apply Header</button>
          <p class="form-status" id="header-status"></p>
        </form>
      </div>
    </section>
  </main>

  <script>
    const wheelNames = ["Front Right", "Back Right", "Back Left", "Front Left"];
    const telemetryBody = document.getElementById('telemetry-body');
    const dribblerEl = document.getElementById('dribbler');
    const telemetryStatus = document.getElementById('telemetry-status');
    const pidStatus = document.getElementById('pid-status');
    const setpointStatus = document.getElementById('setpoint-status');
    const headerStatus = document.getElementById('header-status');

    function renderSkeleton() {
      telemetryBody.innerHTML = wheelNames.map((name, idx) => `
        <tr data-wheel="${idx}">
          <td class="wheel-name">${name}</td>
          <td class="target">--</td>
          <td class="actual">--</td>
          <td class="output">--</td>
        </tr>
      `).join('');
    }

    function setStatus(el, message, type) {
      el.textContent = message;
      el.classList.remove('success', 'error');
      if (type) {
        el.classList.add(type);
      }
    }

    function updateTelemetry(payload) {
      if (!payload || !payload.wheels) {
        return;
      }
      payload.wheels.forEach((wheel) => {
        const row = telemetryBody.querySelector(`tr[data-wheel="${wheel.index}"]`);
        if (!row) return;
        row.querySelector('.target').textContent = wheel.target;
        row.querySelector('.actual').textContent = wheel.actual;
        row.querySelector('.output').textContent = wheel.output;
      });
      dribblerEl.textContent = `Dribbler: ${payload.dribbler ?? '--'}`;
      const timeLabel = payload.timestamp_ms ? new Date(payload.timestamp_ms).toLocaleTimeString() : 'LIVE';
      setStatus(telemetryStatus, `Telemetry streaming · ${timeLabel}`, 'success');
    }

    function pollTelemetryFallback() {
      fetch('/api/telemetry')
        .then((resp) => (resp.status === 204 ? null : resp.json()))
        .then((data) => {
          if (data) {
            updateTelemetry(data);
          }
        })
        .catch(() => {});
    }

    renderSkeleton();
    fetch('/api/header')
      .then((resp) => resp.json())
      .then((data) => {
        if (!data) return;
        document.getElementById('header1').value = (data.byte1 ?? 0).toString(16).padStart(2, '0').toUpperCase();
        document.getElementById('header2').value = (data.byte2 ?? 0).toString(16).padStart(2, '0').toUpperCase();
      })
      .catch(() => {});

    const eventSource = new EventSource('/telemetry');
    eventSource.addEventListener('telemetry', (event) => {
      updateTelemetry(JSON.parse(event.data));
    });
    eventSource.onerror = () => {
      setStatus(telemetryStatus, 'Telemetry stream offline – retrying…', 'error');
    };

    setInterval(pollTelemetryFallback, 4000);

    document.getElementById('pid-form').addEventListener('submit', (event) => {
      event.preventDefault();
      const payload = {
        wheel: parseInt(document.getElementById('wheel-select').value, 10),
        kp: parseFloat(document.getElementById('kp-input').value),
        ki: parseFloat(document.getElementById('ki-input').value),
        kd: parseFloat(document.getElementById('kd-input').value)
      };

      fetch('/api/pid', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      })
        .then((resp) => {
          if (!resp.ok) {
            throw new Error('PID update failed');
          }
          return resp;
        })
        .then(() => {
          setStatus(pidStatus, 'PID gains sent successfully', 'success');
        })
        .catch((error) => setStatus(pidStatus, error.message, 'error'));
    });

    document.getElementById('setpoint-form').addEventListener('submit', (event) => {
      event.preventDefault();
      const payload = {
        u: parseFloat(document.getElementById('u-input').value),
        v: parseFloat(document.getElementById('v-input').value),
        w: parseFloat(document.getElementById('w-input').value)
      };

      fetch('/api/setpoint', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      })
        .then((resp) => {
          if (!resp.ok) {
            throw new Error('Command rejected');
          }
          return resp;
        })
        .then(() => setStatus(setpointStatus, 'Velocity command queued', 'success'))
        .catch((error) => setStatus(setpointStatus, error.message, 'error'));
    });

    document.getElementById('header-form').addEventListener('submit', (event) => {
      event.preventDefault();
      const b1 = document.getElementById('header1').value.trim();
      const b2 = document.getElementById('header2').value.trim();
      const byte1 = parseInt(b1, 16);
      const byte2 = parseInt(b2, 16);
      if (Number.isNaN(byte1) || Number.isNaN(byte2)) {
        setStatus(headerStatus, 'Invalid hex bytes', 'error');
        return;
      }
      fetch('/api/header', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ byte1, byte2 })
      })
        .then((resp) => {
          if (!resp.ok) {
            throw new Error('Header update failed');
          }
          return resp.json();
        })
        .then((data) => {
          setStatus(headerStatus, `Header set to ${data.hex}`, 'success');
        })
        .catch((error) => setStatus(headerStatus, error.message, 'error'));
    });
  </script>
</body>
</html>
)rawliteral";

portMUX_TYPE telemetryMux = portMUX_INITIALIZER_UNLOCKED;
TelemetrySnapshot latestTelemetry;
bool telemetryAvailable = false;

enum class TelemetryParseState {
  WAIT_HEADER_1 = 0,
  WAIT_HEADER_2,
  READ_FRAME
};

TelemetryParseState telemetryState = TelemetryParseState::WAIT_HEADER_1;
std::array<uint8_t, TELEMETRY_FRAME_SIZE> telemetryBuffer;
size_t telemetryBytesRead = 0;

void process_robot_serial();
void resetTelemetryParser();
void handleTelemetryFrame(const uint8_t *frame);
bool copy_latest_telemetry(TelemetrySnapshot &out);
String telemetry_to_json(const TelemetrySnapshot &snapshot);
String header_config_to_json();
int16_t read_be16(const uint8_t *data);
uint32_t read_be32(const uint8_t *data);
void start_web_server();
void init_header_preferences();
bool apply_header_bytes(uint8_t header1, uint8_t header2, bool persist);
bool update_headers_runtime(uint8_t header1, uint8_t header2);
HeaderBytes get_active_header_bytes();
void sync_motor_command_header();

void setup() {
  Serial.begin(BAUD_RATE);
  connect_wifi();
  UDP.beginMulticast(multicastIP, MULTICAST_PORT);
  telemetryUdp.begin(TELEMETRY_PORT);
  robotSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  init_header_preferences();
  init_motor_command();
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(KICKER_PIN, OUTPUT);
  start_charging_kicker();
  start_web_server();
  PRINT("READY\n");
}

void loop() {
  uint16_t last_packet_size = 0;
  packet_size = UDP.parsePacket();
  // The network may have multiple UDP packets queue up in the buffer
  // Read all of them, but only process the last one to reduce jitter
  while (packet_size > 0) {
    UDP.read(packet_buffer, 511);
    last_packet_size = packet_size;
    packet_size = UDP.parsePacket();
  }

  if (last_packet_size > 0) {
    packet_time = micros();
    PRINT((int)last_packet_size, " | ");
    for (uint16_t i = 0; i < last_packet_size; i++) {
      char c = packet_buffer[i];
      handleNewChar(c);
    }
  }

  process_robot_serial();
  check_kicker_status();
}

void connect_wifi() {
  PRINT("\nConnecting WiFi to ", SSID);
  // Attempt connection every 500 ms
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PRINT(".");
  }
  PRINT("\nWiFi connected", "\nIP address: ", WiFi.localIP(), "\n");
}

void init_motor_command() {
  for (int index = 0; index < MOTOR_COMMAND_SIZE; index++) {
    if (index < MOTOR_CMD_HEADER_SIZE) {
      motor_command[index] = motor_cmd_headers[index];
    } else {
      motor_command[index] = 0;
    }
  }
}

void process_robot_serial() {
  while (robotSerial.available() > 0) {
    uint8_t byte_read = robotSerial.read();
    switch (telemetryState) {
      case TelemetryParseState::WAIT_HEADER_1:
        if (byte_read == TELEMETRY_HEADER_BYTE_1) {
          telemetryBuffer[0] = byte_read;
          telemetryBytesRead = 1;
          telemetryState = TelemetryParseState::WAIT_HEADER_2;
        }
        break;
      case TelemetryParseState::WAIT_HEADER_2:
        if (byte_read == TELEMETRY_HEADER_BYTE_2) {
          telemetryBuffer[1] = byte_read;
          telemetryBytesRead = 2;
          telemetryState = TelemetryParseState::READ_FRAME;
        } else if (byte_read == TELEMETRY_HEADER_BYTE_1) {
          telemetryBuffer[0] = byte_read;
          telemetryBytesRead = 1;
          telemetryState = TelemetryParseState::WAIT_HEADER_2;
        } else {
          resetTelemetryParser();
        }
        break;
      case TelemetryParseState::READ_FRAME:
        if (telemetryBytesRead < TELEMETRY_FRAME_SIZE) {
          telemetryBuffer[telemetryBytesRead++] = byte_read;
        }
        if (telemetryBytesRead >= TELEMETRY_FRAME_SIZE) {
          handleTelemetryFrame(telemetryBuffer.data());
          resetTelemetryParser();
        }
        break;
      default:
        resetTelemetryParser();
        break;
    }
  }
}

void resetTelemetryParser() {
  telemetryState = TelemetryParseState::WAIT_HEADER_1;
  telemetryBytesRead = 0;
}

void handleTelemetryFrame(const uint8_t *frame) {
  if (frame[0] != TELEMETRY_HEADER_BYTE_1 || frame[1] != TELEMETRY_HEADER_BYTE_2) {
    return;
  }

  TelemetrySnapshot snapshot;
  snapshot.timestamp_ms = read_be32(frame + 2);
  for (size_t i = 0; i < TELEMETRY_WHEEL_COUNT; ++i) {
    size_t base = 6 + (i * TELEMETRY_WHEEL_STRIDE);
    snapshot.wheels[i].target = read_be16(frame + base);
    snapshot.wheels[i].actual = read_be16(frame + base + 2);
    snapshot.wheels[i].output = read_be16(frame + base + 4);
  }
  size_t dribbler_index = 6 + (TELEMETRY_WHEEL_COUNT * TELEMETRY_WHEEL_STRIDE);
  snapshot.dribbler = read_be16(frame + dribbler_index);

  portENTER_CRITICAL(&telemetryMux);
  latestTelemetry = snapshot;
  telemetryAvailable = true;
  portEXIT_CRITICAL(&telemetryMux);

  String payload = telemetry_to_json(snapshot);
  telemetryEvents.send(payload.c_str(), "telemetry");

  telemetryUdp.beginPacket(multicastIP, TELEMETRY_PORT);
  telemetryUdp.write(frame, TELEMETRY_FRAME_SIZE);
  telemetryUdp.endPacket();
}

bool copy_latest_telemetry(TelemetrySnapshot &out) {
  bool hasTelemetry = false;
  portENTER_CRITICAL(&telemetryMux);
  if (telemetryAvailable) {
    out = latestTelemetry;
    hasTelemetry = true;
  }
  portEXIT_CRITICAL(&telemetryMux);
  return hasTelemetry;
}

String telemetry_to_json(const TelemetrySnapshot &snapshot) {
  StaticJsonDocument<512> doc;
  doc["timestamp_ms"] = snapshot.timestamp_ms;
  doc["dribbler"] = snapshot.dribbler;
  JsonArray wheels = doc.createNestedArray("wheels");
  for (size_t i = 0; i < TELEMETRY_WHEEL_COUNT; ++i) {
    JsonObject w = wheels.createNestedObject();
    w["index"] = static_cast<uint8_t>(i);
    w["target"] = snapshot.wheels[i].target;
    w["actual"] = snapshot.wheels[i].actual;
    w["output"] = snapshot.wheels[i].output;
  }
  String json;
  serializeJson(doc, json);
  return json;
}

String header_config_to_json() {
  HeaderBytes headers = get_active_header_bytes();
  StaticJsonDocument<192> doc;
  doc["byte1"] = headers.byte1;
  doc["byte2"] = headers.byte2;
  char hex_repr[6];
  snprintf(hex_repr, sizeof(hex_repr), "%02X%02X", headers.byte1, headers.byte2);
  doc["hex"] = hex_repr;
  doc["persisted"] = headerPrefsReady;
  String json;
  serializeJson(doc, json);
  return json;
}

int16_t read_be16(const uint8_t *data) {
  return static_cast<int16_t>((static_cast<uint16_t>(data[0]) << 8) | data[1]);
}

uint32_t read_be32(const uint8_t *data) {
  return (static_cast<uint32_t>(data[0]) << 24) |
         (static_cast<uint32_t>(data[1]) << 16) |
         (static_cast<uint32_t>(data[2]) << 8) |
         static_cast<uint32_t>(data[3]);
}

void start_web_server() {
  telemetryEvents.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      PRINT("[SSE] Reconnected, lastId: ", client->lastId(), "\n");
    }
    client->send("connected", "status", millis());
    TelemetrySnapshot snapshot;
    if (copy_latest_telemetry(snapshot)) {
      String payload = telemetry_to_json(snapshot);
      client->send(payload.c_str(), "telemetry");
    }
  });

  server.addHandler(&telemetryEvents);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/api/telemetry", HTTP_GET, [](AsyncWebServerRequest *request) {
    TelemetrySnapshot snapshot;
    if (copy_latest_telemetry(snapshot)) {
      request->send(200, "application/json", telemetry_to_json(snapshot));
    } else {
      request->send(204);
    }
  });

  auto pidHandler = new AsyncCallbackJsonWebHandler("/api/pid", [](AsyncWebServerRequest *request, JsonVariant &json) {
    if (!json.is<JsonObject>()) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    JsonObject obj = json.as<JsonObject>();
    if (!obj.containsKey("kp") || !obj.containsKey("ki") || !obj.containsKey("kd")) {
      request->send(400, "application/json", "{\"error\":\"Missing kp/ki/kd\"}");
      return;
    }
    int wheel = obj.containsKey("wheel") ? obj["wheel"].as<int>() : -1;
    float kp = obj["kp"].as<float>();
    float ki = obj["ki"].as<float>();
    float kd = obj["kd"].as<float>();
    uint8_t wheel_id = (wheel < 0) ? 0xFF : static_cast<uint8_t>(wheel);
    if (wheel_id != 0xFF && wheel_id >= TELEMETRY_WHEEL_COUNT) {
      request->send(400, "application/json", "{\"error\":\"wheel must be 0-3 or -1\"}");
      return;
    }
    send_pid_update(wheel_id, kp, ki, kd);
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });
  server.addHandler(pidHandler);

  auto setpointHandler = new AsyncCallbackJsonWebHandler("/api/setpoint", [](AsyncWebServerRequest *request, JsonVariant &json) {
    if (!json.is<JsonObject>()) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    JsonObject obj = json.as<JsonObject>();
    if (!obj.containsKey("u") || !obj.containsKey("v") || !obj.containsKey("w")) {
      request->send(400, "application/json", "{\"error\":\"Missing u/v/w\"}");
      return;
    }
    float u = obj["u"].as<float>();
    float v = obj["v"].as<float>();
    float w = obj["w"].as<float>();
    command_velocity(u, v, w);
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });
  server.addHandler(setpointHandler);

  server.on("/api/header", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", header_config_to_json());
  });

  auto headerHandler = new AsyncCallbackJsonWebHandler("/api/header", [](AsyncWebServerRequest *request, JsonVariant &json) {
    if (!json.is<JsonObject>()) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    JsonObject obj = json.as<JsonObject>();
    if (!obj.containsKey("byte1") || !obj.containsKey("byte2")) {
      request->send(400, "application/json", "{\"error\":\"Missing byte1/byte2\"}");
      return;
    }
    int b1 = obj["byte1"].as<int>();
    int b2 = obj["byte2"].as<int>();
    if (b1 < 0 || b1 > 255 || b2 < 0 || b2 > 255) {
      request->send(400, "application/json", "{\"error\":\"Bytes must be 0-255\"}");
      return;
    }
    if (!update_headers_runtime(static_cast<uint8_t>(b1), static_cast<uint8_t>(b2))) {
      request->send(500, "application/json", "{\"error\":\"Unable to apply header\"}");
      return;
    }
    request->send(200, "application/json", header_config_to_json());
  });
  server.addHandler(headerHandler);

  server.begin();
}

void init_header_preferences() {
  headerPrefsReady = headerPrefs.begin(HEADER_PREF_NAMESPACE, false);
  uint8_t header1 = DEFAULT_HEADER_BYTE_1;
  uint8_t header2 = DEFAULT_HEADER_BYTE_2;
  if (headerPrefsReady) {
    header1 = headerPrefs.getUChar(HEADER_PREF_KEY_1, DEFAULT_HEADER_BYTE_1);
    header2 = headerPrefs.getUChar(HEADER_PREF_KEY_2, DEFAULT_HEADER_BYTE_2);
  }
  apply_header_bytes(header1, header2, false);
}

bool apply_header_bytes(uint8_t header1, uint8_t header2, bool persist) {
  motor_cmd_headers[0] = header1;
  motor_cmd_headers[1] = header2;
  sync_motor_command_header();
  bool stored = false;
  if (persist && headerPrefsReady) {
    stored = headerPrefs.putUChar(HEADER_PREF_KEY_1, header1);
    stored &= headerPrefs.putUChar(HEADER_PREF_KEY_2, header2);
  }
  return stored || !persist;
}

bool update_headers_runtime(uint8_t header1, uint8_t header2) {
  if (!apply_header_bytes(header1, header2, true)) {
    // Still continue so the runtime value changes even if persistence failed
    apply_header_bytes(header1, header2, false);
  }
  send_header_config(header1, header2);
  return true;
}

HeaderBytes get_active_header_bytes() {
  HeaderBytes headers{motor_cmd_headers[0], motor_cmd_headers[1]};
  return headers;
}

void sync_motor_command_header() {
  for (size_t i = 0; i < MOTOR_CMD_HEADER_SIZE; ++i) {
    motor_command[i] = motor_cmd_headers[i];
  }
}

void start_charging_kicker() {
  digitalWrite(KICKER_PIN, HIGH);  // turn OFF kicker
  digitalWrite(SOLENOID_PIN, HIGH);  // START charging 
  delay(KICKER_CHARGING_TIME);
  digitalWrite(SOLENOID_PIN, LOW);  // STOP charging
  kicker_charged = true;
}

void check_kicker_status() {
  if (charging_kicker) {
    unsigned long time_elasped = millis() - start_charge_time;
    if (time_elasped >= KICKER_CHARGING_TIME) {
      digitalWrite(SOLENOID_PIN, LOW);  // STOP charging
      charging_kicker = false;
      kicker_charged = true;
    }
  } else {
    unsigned long time_elasped = millis() - last_kick_time;
    if (time_elasped >= WAIT_BEFORE_CHARGE_AGAIN) {
      digitalWrite(SOLENOID_PIN, HIGH);  // START charging
      charging_kicker = true;
      start_charge_time = millis();
    } else if (time_elasped >= KICKING_TIME) {
      digitalWrite(KICKER_PIN, HIGH);  // turn OFF the kicker
    }
  }
}

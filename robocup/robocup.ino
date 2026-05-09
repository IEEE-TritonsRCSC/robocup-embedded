#include <Arduino.h>
#include <CAN.h>
#include <cmath>
#undef isfinite

inline int isfinite(double value) {
  return std::isfinite(value);
}

#include <Moteus.h>

namespace {

constexpr uint8_t kTransceiverStandbyPin = 2;   // D2 -> PB3
constexpr uint8_t kTransceiverEnablePin = 3;    // D3 -> PB0
constexpr unsigned long kCommandPeriodMs = 20;
constexpr CanBitRate kCanArbitrationBitrate = CanBitRate::BR_1000k;
constexpr uint32_t kCanDataBitrate = 5000000;

class UnoQTransport {
 public:
  void poll() {
    // The UNO Q Zephyr CAN driver already buffers received frames, so
    // there is no explicit pump step required here.
  }

  bool available() {
    return CAN.available() > 0;
  }

  bool receive(CANFDMessage& msg) {
    if (!available()) {
      return false;
    }

    const CanFDMsg rx = CAN.readFD();
    msg.id = rx.id;
    msg.ext = rx.isExtendedId();
    msg.idx = 0;
    msg.len = rx.data_length;
    memcpy(msg.data, rx.data, rx.data_length);

    // The UNO Q CAN wrapper currently exposes FD messages but does not
    // surface the BRS flag to user code, so we conservatively mark all
    // received frames as FD without bitrate switching.
    msg.type = CANFDMessage::CANFD_NO_BIT_RATE_SWITCH;
    return true;
  }

  bool tryToSend(const CANFDMessage& msg) {
    const uint8_t len = mjbots::moteus::RoundUpDlc(msg.len);
    CanFDMsg tx(msg.id, len, msg.data);
    return CAN.writeFD(tx) > 0;
  }

  bool try_send(const CANFDMessage& msg) {
    return tryToSend(msg);
  }
};

using Moteus = MoteusController<UnoQTransport>;

UnoQTransport g_can_transport;
Moteus g_moteus(
    g_can_transport,
    [] {
      Moteus::Options options;
      options.id = 1;
      options.source = 0;
      options.disable_brs = true;
      options.default_query = true;
      return options;
    }());

unsigned long g_next_command_ms = 0;

template <typename T>
void PrintBoth(const T& value) {
  Serial.print(value);
  Serial1.print(value);
}

template <typename T>
void PrintBothLine(const T& value) {
  Serial.println(value);
  Serial1.println(value);
}

void WaitForSerial(unsigned long timeout_ms) {
  const unsigned long start = millis();
  while (!Serial && (millis() - start) < timeout_ms) {
    delay(10);
  }
}

void PrintReply(const Moteus::Query::Result& result) {
  PrintBoth(F("pos="));
  PrintBoth(result.position);
  PrintBoth(F(" vel="));
  PrintBoth(result.velocity);
  PrintBoth(F(" mode="));
  PrintBoth(static_cast<int>(result.mode));
  PrintBoth(F(" fault="));
  PrintBothLine(static_cast<int>(result.fault));
}

}  // namespace

void setup() {
  pinMode(kTransceiverStandbyPin, OUTPUT);
  pinMode(kTransceiverEnablePin, OUTPUT);
  digitalWrite(kTransceiverStandbyPin, HIGH);
  digitalWrite(kTransceiverEnablePin, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  Serial1.begin(115200);
  WaitForSerial(2000);

  PrintBothLine(F("UNO Q moteus CAN-FD demo starting"));
  PrintBothLine(F("Transceiver set to normal-operating mode"));

  if (!CAN.beginFD(kCanArbitrationBitrate, kCanDataBitrate, false)) {
    PrintBothLine(F("ERROR: CAN FD initialization failed"));
    while (true) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(1000);
    }
  }

  PrintBothLine(F("CAN FD initialized at 1 Mbps / 5 Mbps, BRS disabled"));

  if (!g_moteus.SetStop()) {
    PrintBothLine(F("Initial stop command timed out"));
  } else {
    PrintBothLine(F("Initial stop command acknowledged"));
  }

  g_next_command_ms = millis();
}

void loop() {
  const unsigned long now = millis();
  if (static_cast<long>(now - g_next_command_ms) < 0) {
    return;
  }
  g_next_command_ms = now + kCommandPeriodMs;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  Moteus::PositionMode::Command cmd;
  cmd.position = ((now / 2000UL) % 2UL == 0UL) ? 0.25 : -0.25;
  cmd.velocity = 0.0;

  if (g_moteus.SetPosition(cmd)) {
    PrintReply(g_moteus.last_result().values);
  } else {
    PrintBothLine(F("No reply from moteus ID 1"));
  }
}

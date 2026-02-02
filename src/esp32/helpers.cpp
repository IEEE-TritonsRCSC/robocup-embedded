#include "helpers.h"
#include <algorithm>
#include <array>
#include <cstring>

namespace {

constexpr size_t PID_FRAME_SIZE = MOTOR_CMD_HEADER_SIZE + MOTOR_CMD_TYPE_SIZE + UART_PID_PAYLOAD_SIZE;
constexpr size_t CONFIG_FRAME_SIZE = MOTOR_CMD_HEADER_SIZE + MOTOR_CMD_TYPE_SIZE + UART_CONFIG_PAYLOAD_SIZE;

inline void write_float_be(float value, uint8_t *dest) {
  uint32_t raw;
  std::memcpy(&raw, &value, sizeof(float));
  dest[0] = static_cast<uint8_t>((raw >> 24) & 0xFF);
  dest[1] = static_cast<uint8_t>((raw >> 16) & 0xFF);
  dest[2] = static_cast<uint8_t>((raw >> 8) & 0xFF);
  dest[3] = static_cast<uint8_t>(raw & 0xFF);
}

}

int size;
char buffer[MAX_BUFFER_SIZE];
char cmd_buffer[MAX_COMMAND_BUFFER];

int n_read = 0;
float power = 0.0;
float dir = 0.0f;
float angular_speed = 0.0f;

float vel_u = 0.0f;
float vel_v = 0.0f;
float vel_w = 0.0f;

bool kicker_charged = false;
bool charging_kicker = false;
unsigned long start_charge_time = 0;
unsigned long last_kick_time = 0;

bool stop_dribbler_on_next_command = false;

float cosFront = cosf(FRONT_ANGLE * M_PI/180);
float sinFront = sinf(FRONT_ANGLE * M_PI/180);
float cosBack = cosf(BACK_ANGLE * M_PI/180);
float sinBack = sinf(BACK_ANGLE * M_PI/180);
float wheel_velocities[4] = {0.0f, 0.0f, 0.0f, 0.0f};

// --------------------------------Parsers--------------------------------
void handleNewChar(char c) {
  // Each line or "string" received is a message
  if (c == '\n' || c == '\0') {
    buffer[size] = '\0';  // null terminate the string
    if (size) {  // if buffer not empty
      parseMsg(buffer);  // process the message
      buffer[0] = '\0';  // reset buffer
      size = 0;  // reset size
    }
  } else {
    buffer[size] = c;  // add char to buffer
    if (++size == MAX_BUFFER_SIZE - 1) {  // prevent overflow
      handleNewChar('\0');  // force terminate the string
    }
  }
}

void parseMsg(char *msg) {
  if (strcmp(msg, "stop") == 0) {
    execute_stop();
  } else if (sscanf(msg, RELEVANT_FORMAT, &cmd_buffer, &n_read) == 1) {
    msg += n_read;
    parseCommand(cmd_buffer, msg);
  }
  cmd_buffer[0] = '\0';
}

void parseCommand(char *command, char *parameters) {
  switch (command[0]) {
    case 't':  // turn
      if (sscanf(parameters, " %f", &angular_speed) == 1) {
        execute_turn(angular_speed);
        break;
      }
      return;
    case 'd':  // dash
      if (sscanf(parameters, " %f %f", &power, &dir) == 2) {
        execute_dash(power, dir);
        break;
      }
      return;
    case 's':  // skick
      if (sscanf(parameters, " %f", &power) == 1) {
        execute_skick(power);
        break;
      }
      return;
    case 'k':  // kick
      execute_kick();
      break;
    case 'c':  // catch
      execute_catch();
      break;
    default:
      return;
  }
  prepare_and_send_motor_command();
  Serial.println(micros() - packet_time);
}

// --------------------------------Executors--------------------------------
void execute_stop() {
  PRINT("Stopping | ");
  vel_u = 0.0f;
  vel_v = 0.0f;
  vel_w = 0.0f;
  setDribbler(0.0f);
  prepare_and_send_motor_command();
}

void execute_turn(float angular_speed) {
  PRINT("Turning at ", angular_speed, " rad/s | ");
  vel_w = angular_speed;
}

void execute_dash(float power, float dir) {
  PRINT("Dashing with ", power, " power in ", dir, " radians | ");
  float acceleration = 0.006 * power;
  vel_u += acceleration * sinf(dir);
  vel_v += acceleration * cosf(dir);
  vel_w = 0.0f;
}

void execute_skick(float power) {
  PRINT("Short Kicking the ball with ", power, " power | ");
  setDribbler(-power);    // reverse the direction to skick the ball
  stop_dribbler_on_next_command = true;
  vel_w = 0.0f;
}

void execute_kick() {
  PRINT("Kicking the ball | ");
  setDribbler(0.0f);
  if (kicker_charged) {
    kicker_charged = false;
    digitalWrite(KICKER_PIN, LOW);  // turn ON the kicker
    last_kick_time = millis();
  }
  vel_w = 0.0f;
}

void execute_catch() {
  PRINT("Catching the ball | ");
  setDribbler(100.0f);
  vel_w = 0.0f;
}

// --------------------------------Hardware Controllers--------------------------------
void setDribbler(float power) {
  motor_command[DRIBBLER_MOTOR_INDEX] = static_cast<int8_t>(roundf(power));
}

void prepare_and_send_motor_command() {
  motor_command[MOTOR_CMD_HEADER_SIZE] = FRAME_TYPE_DRIVE_COMMAND;
  // Translate vel_u and vel_v into wheel velocities
  wheel_velocities[0] = (vel_u * -sinFront) + (vel_v * -cosFront);  // front-right
  wheel_velocities[1] = (vel_u * sinBack) + (vel_v * -cosBack);  // back-right
  wheel_velocities[2] = (vel_u * sinBack) + (vel_v * cosBack);  // back-left
  wheel_velocities[3] = (vel_u * -sinFront) + (vel_v * cosFront);  // front-left

  PRINT("(");
  for (int wheel_i = 0; wheel_i < 4; wheel_i++) {
    // Translate wheel velocities into angular velocities
    wheel_velocities[wheel_i] = wheel_velocities[wheel_i] / rad_wheel;
    // Add in angular velocities
    wheel_velocities[wheel_i] += vel_w * rad_robot / rad_wheel;
    
    // Set wheel rad/s in motor command
    int speed = static_cast<int>(roundf(wheel_velocities[wheel_i] * 100.0f));
    speed = std::clamp(speed, static_cast<int>(INT16_MIN), static_cast<int>(INT16_MAX));
    int index = MOTOR_CMD_PAYLOAD_OFFSET + (wheel_i * 2);
    motor_command[index] = (speed >> 8 & 0xFF);
    motor_command[index + 1] = (speed & 0xFF);
    PRINT(speed, " ");
  }

  // Send motor command
  PRINT(static_cast<int8_t>(motor_command[DRIBBLER_MOTOR_INDEX]), ") | ");
  PRINT("(", vel_u, " ", vel_v, " ", vel_w, ")\n");
  robotSerial.write(motor_command.data(), motor_command.size());

  if (stop_dribbler_on_next_command) {
    setDribbler(0.0f);
    stop_dribbler_on_next_command = false;
  }

  // Decay vel_u and vel_v
  vel_u *= 0.4;
  vel_v *= 0.4;
}

void command_velocity(float u, float v, float w) {
  vel_u = u;
  vel_v = v;
  vel_w = w;
  prepare_and_send_motor_command();
}

void send_pid_update(uint8_t wheel, float kp, float ki, float kd) {
  std::array<uint8_t, PID_FRAME_SIZE> pid_frame{};
  pid_frame[0] = motor_cmd_headers[0];
  pid_frame[1] = motor_cmd_headers[1];
  pid_frame[MOTOR_CMD_HEADER_SIZE] = FRAME_TYPE_PID_UPDATE;
  pid_frame[MOTOR_CMD_HEADER_SIZE + 1] = wheel;

  write_float_be(kp, pid_frame.data() + MOTOR_CMD_HEADER_SIZE + 2);
  write_float_be(ki, pid_frame.data() + MOTOR_CMD_HEADER_SIZE + 6);
  write_float_be(kd, pid_frame.data() + MOTOR_CMD_HEADER_SIZE + 10);

  robotSerial.write(pid_frame.data(), pid_frame.size());
}

void send_header_config(uint8_t header1, uint8_t header2) {
  std::array<uint8_t, CONFIG_FRAME_SIZE> config_frame{};
  config_frame[0] = motor_cmd_headers[0];
  config_frame[1] = motor_cmd_headers[1];
  config_frame[MOTOR_CMD_HEADER_SIZE] = FRAME_TYPE_HEADER_CONFIG;
  config_frame[MOTOR_CMD_HEADER_SIZE + 1] = header1;
  config_frame[MOTOR_CMD_HEADER_SIZE + 2] = header2;
  config_frame[MOTOR_CMD_HEADER_SIZE + 3] = HEADER_CHECKSUM_SEED ^ header1 ^ header2;

  robotSerial.write(config_frame.data(), config_frame.size());
}

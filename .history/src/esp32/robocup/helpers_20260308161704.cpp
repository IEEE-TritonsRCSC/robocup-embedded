#include "helpers.h"

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
  if (strcmp(command, "pidu") == 0) {
    int wheel_idx = 0;
    long kp_q = 0;
    long ki_q = 0;
    long kd_q = 0;
    int parsed = sscanf(parameters, " %d %ld %ld %ld", &wheel_idx, &kp_q, &ki_q, &kd_q);
    if (parsed >= 3) {
      if (parsed == 3) {
        kd_q = 0;
      }
      send_pid_update(wheel_idx, static_cast<int>(kp_q), static_cast<int>(ki_q), static_cast<int>(kd_q));
      PRINT("PID update | ");
    }
    return;
  }

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
    case 'p':  // kicktest (pulse)
      execute_kicktest();
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
    digitalWrite(KICKER_PIN, HIGH);  // turn ON the kicker
    last_kick_time = millis();
  }
  vel_w = 0.0f;
  /*
      digitalWrite(kickPin, HIGH);
    delay(200);
    digitalWrite(kickPin, LOW);
    Serial.println("---------KICKED----------");
    kick = false;
  */
}

void execute_catch() {
  PRINT("Catching the ball | ");
  setDribbler(100.0f);
  vel_w = 0.0f;
}

void execute_kicktest() {
  PRINT("Kicktest pulse | ");
  setDribbler(0.0f);
  digitalWrite(KICKER_PIN, LOW);  // force ON
  delay(100);
  digitalWrite(KICKER_PIN, HIGH);  // back OFF
  vel_w = 0.0f;
}

// --------------------------------Hardware Controllers--------------------------------
void setDribbler(float power) {
  motor_command[DRIBBLER_MOTOR_INDEX] = static_cast<int8_t>(roundf(power));
}

void prepare_and_send_motor_command() {
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
    int index = MOTOR_CMD_HEADER_SIZE + (wheel_i * 2);
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

static int16_t clamp_int16_value(long value) {
  if (value > 32767) {
    return 32767;
  }
  if (value < -32768) {
    return -32768;
  }
  return static_cast<int16_t>(value);
}

void send_pid_update(int wheel_idx, int kp_q, int ki_q, int kd_q) {
  if (wheel_idx < 0 || wheel_idx > 255) {
    return;
  }

  uint8_t packet[UART_PID_PACKET_SIZE];
  packet[0] = UART_HEADER_1;
  packet[1] = UART_HEADER_2_PID;
  packet[2] = static_cast<uint8_t>(wheel_idx);

  int16_t kp = clamp_int16_value(kp_q);
  int16_t ki = clamp_int16_value(ki_q);
  int16_t kd = clamp_int16_value(kd_q);

  packet[3] = static_cast<uint8_t>((kp >> 8) & 0xFF);
  packet[4] = static_cast<uint8_t>(kp & 0xFF);
  packet[5] = static_cast<uint8_t>((ki >> 8) & 0xFF);
  packet[6] = static_cast<uint8_t>(ki & 0xFF);
  packet[7] = static_cast<uint8_t>((kd >> 8) & 0xFF);
  packet[8] = static_cast<uint8_t>(kd & 0xFF);

  robotSerial.write(packet, sizeof(packet));
}

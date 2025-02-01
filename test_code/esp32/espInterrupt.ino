#define INTERRUPT_PIN 16

volatile bool interrupt_flag = false;

void IRAM_ATTR handleInterrupt() {
  interrupt_flag = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN),
                                        handleInterrupt,
                                        FALLING);
}

void loop() {
  if (interrupt_flag) {
    Serial.println("Interrupt detected!");
    interrupt_flag = false;
  }

}

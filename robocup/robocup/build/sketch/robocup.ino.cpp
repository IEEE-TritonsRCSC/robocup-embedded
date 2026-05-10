#include <Arduino.h>
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup();
#line 4 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void loop();
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup() {
   pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
   digitalWrite(LED_BUILTIN,HIGH);
   delay(1000);
   digitalWrite(LED_BUILTIN,LOW);
   delay(1000);
}

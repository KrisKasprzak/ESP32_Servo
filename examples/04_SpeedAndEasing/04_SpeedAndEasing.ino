/*
 * 04_SpeedAndEasing
 *
 * Demonstrates non-blocking, speed-controlled motion:
 *   - setSpeed()   limits the servo to a number of degrees per second
 *   - setEasing()  shapes the motion profile (Linear, EaseIn, EaseOut, EaseInOut)
 *   - moveTo()     starts a move; update() advances it from loop()
 *
 * Because nothing blocks, the on-board LED keeps blinking while the servo
 * moves - proving the sketch is free to do other work.
 *
 * Wiring:
 *   Servo signal -> GPIO 18, V+ -> external 5 V, GND -> common ground
 *
 * ---------------------------------------------------------------------------
 * ESP32 PINS THAT CANNOT (OR SHOULD NOT) BE USED FOR A SERVO SIGNAL
 * ---------------------------------------------------------------------------
 * Classic ESP32 (ESP32-WROOM / WROVER, DevKitC, NodeMCU-32S ...):
 *   GPIO 6, 7, 8, 9, 10, 11  - wired to the internal SPI flash. NEVER use.
 *   GPIO 34, 35, 36, 39      - input only, cannot output PWM.
 *   GPIO 1 (TX0), 3 (RX0)    - USB serial / programming. Avoid.
 *   GPIO 16, 17              - used by PSRAM on WROVER modules. Avoid there.
 *   GPIO 0, 2, 5, 12, 15     - strapping pins; a servo load can disturb boot
 *                              (GPIO 12 high at boot can brick flash voltage).
 *                              Use only if nothing else is free.
 *   Recommended: 4, 13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
 *
 * ESP32-S2 / ESP32-S3:
 *   GPIO 26-32               - SPI flash / PSRAM. NEVER use.
 *   GPIO 33-37               - octal PSRAM on S3 "R8" modules. Avoid there.
 *   GPIO 19, 20              - native USB D-/D+. Avoid.
 *   GPIO 43, 44              - UART0 TX/RX. Avoid.
 *   GPIO 0, 3, 45, 46        - strapping pins. Avoid.
 *
 * ESP32-C3:
 *   GPIO 12-17               - SPI flash. NEVER use.
 *   GPIO 18, 19              - native USB D-/D+. Avoid.
 *   GPIO 20, 21              - UART0 RX/TX. Avoid.
 *   GPIO 2, 8, 9             - strapping pins. Avoid.
 *
 * Always check your specific board's pinout - many boards hard-wire LEDs,
 * buttons or other peripherals to some of the "free" pins.
 * NOTE: GPIO 18 used here is a USB pin on the ESP32-C3 - pick another pin there.
 * ---------------------------------------------------------------------------
 */
#include <ESP32_Servo.h>

const int SERVO_PIN = 18;

#ifdef LED_BUILTIN
const int LED_PIN = LED_BUILTIN;
#else
const int LED_PIN = 2;  // on-board LED of most ESP32 DevKit boards
#endif

PositionalServo servo;

const ServoEasing EASINGS[] = {ServoEasing::Linear, ServoEasing::EaseIn,
                               ServoEasing::EaseOut, ServoEasing::EaseInOut};
const char* EASING_NAMES[] = {"Linear", "EaseIn", "EaseOut", "EaseInOut"};
const int NUM_EASINGS = sizeof(EASINGS) / sizeof(EASINGS[0]);

int easingIndex = 0;
bool goingUp = true;
uint32_t lastBlink = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    servo.attach(SERVO_PIN);
    servo.write(0);
    delay(500);

    servo.setSpeed(90);  // 90 degrees per second -> a full sweep takes 2 s
    servo.setEasing(EASINGS[easingIndex]);
    servo.moveTo(180);
    Serial.printf("Easing: %s\n", EASING_NAMES[easingIndex]);
}

void loop() {
    // Advance the servo. When the move completes, start the next one.
    if (!servo.update()) {
        delay(300);  // short pause at each end (keeps the demo easy to watch)
        goingUp = !goingUp;

        // After each full there-and-back cycle, switch to the next easing.
        if (goingUp) {
            easingIndex = (easingIndex + 1) % NUM_EASINGS;
            servo.setEasing(EASINGS[easingIndex]);
            Serial.printf("Easing: %s\n", EASING_NAMES[easingIndex]);
        }
        servo.moveTo(goingUp ? 180 : 0);
    }

    // Meanwhile, do other work: blink the LED without blocking.
    if (millis() - lastBlink >= 250) {
        lastBlink = millis();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}

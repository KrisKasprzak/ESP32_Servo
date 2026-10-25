/*
 * 01_BasicSweep
 *
 * The simplest possible example: sweep a standard positional servo back and
 * forth between 0 and 180 degrees using write().
 *
 * Wiring:
 *   Servo signal (orange/yellow) -> GPIO 18
 *   Servo V+ (red)               -> external 5 V supply (not the ESP32 3V3 pin!)
 *   Servo GND (brown/black)      -> supply GND *and* ESP32 GND (common ground)
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

PositionalServo servo;

void setup() {
    Serial.begin(115200);

    // Default pulse range 500-2500 us over 180 degrees suits most servos.
    // If yours buzzes at the ends, narrow it, e.g. servo.attach(SERVO_PIN, 1000, 2000);
    if (!servo.attach(SERVO_PIN)) {
        Serial.println("Failed to attach servo - check the pin number.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("Servo attached, sweeping...");
}

void loop() {
    for (int angle = 0; angle <= 180; angle++) {
        servo.write(angle);
        delay(15);
    }
    for (int angle = 180; angle >= 0; angle--) {
        servo.write(angle);
        delay(15);
    }
}

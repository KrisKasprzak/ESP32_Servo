/*
 * 05_ContinuousRotation
 *
 * Controls a 360-degree continuous rotation servo (FS90R, FS5103R, ...):
 *   - setSpeed(-100..100)   direction and speed in percent
 *   - setAcceleration()     smooth ramps instead of sudden jumps
 *   - runFor()              spin for a fixed time, then stop automatically
 *   - setNeutral()          calibrate the stop point if the servo creeps
 *
 * Wiring:
 *   Servo signal -> GPIO 19, V+ -> external 5 V, GND -> common ground
 *
 * Calibration tip: if the servo slowly turns when it should be stopped,
 * either turn the small trim potentiometer on the servo (if present) or
 * adjust NEUTRAL_US below by a few microseconds until it stands still.
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
 * NOTE: GPIO 19 used here is fine on a classic ESP32 but is a USB pin on
 * S2/S3/C3 - pick another pin on those chips.
 * ---------------------------------------------------------------------------
 */
#include <ESP32_Servo.h>

const int SERVO_PIN = 19;
const uint16_t NEUTRAL_US = 1500;  // adjust if the servo creeps when stopped

ContinuousServo wheel;

// Simple step sequencer so the demo runs without blocking delays.
int step = 0;
uint32_t stepStart = 0;

void nextStep() {
    step++;
    stepStart = millis();
}

void setup() {
    Serial.begin(115200);

    wheel.attach(SERVO_PIN, 1000, 2000, NEUTRAL_US);
    wheel.setDeadband(2);         // treat |speed| <= 2% as stop
    wheel.setAcceleration(100);   // 100% per second -> 0 to full in 1 s

    stepStart = millis();
    Serial.println("Ramping up forward...");
    wheel.forward(100);
}

void loop() {
    wheel.update();  // must be called often for ramps and timed runs

    uint32_t elapsed = millis() - stepStart;

    switch (step) {
        case 0:  // ramp up forward, hold for 2 s
            if (elapsed > 3000) {
                Serial.println("Ramping to full reverse (passes through zero smoothly)...");
                wheel.backward(100);
                nextStep();
            }
            break;

        case 1:  // ramp to reverse, hold
            if (elapsed > 4000) {
                Serial.println("Soft stop...");
                wheel.softStop();
                nextStep();
            }
            break;

        case 2:  // wait until stopped
            if (!wheel.isRamping() && elapsed > 1500) {
                Serial.println("Timed run: 30% forward for 2 s (ramping disabled)...");
                wheel.setAcceleration(0);
                wheel.runFor(30, 2000);
                nextStep();
            }
            break;

        case 3:  // wait for timed run to finish
            if (!wheel.update()) {
                Serial.println("Done. Restarting in 2 s.\n");
                nextStep();
            }
            break;

        case 4:
            if (elapsed > 2000) {
                step = 0;
                stepStart = millis();
                wheel.setAcceleration(100);
                Serial.println("Ramping up forward...");
                wheel.forward(100);
            }
            break;
    }
}

/*
 * 03_PotentiometerControl
 *
 * Turn a potentiometer (pot) to set the servo angle from 0 to 180 degrees.
 * The reading is smoothed and the servo is only updated when the angle
 * changes, which stops the servo from jittering.
 *
 * Wiring:
 *   Pot outer leg 1   -> 3V3
 *   Pot outer leg 2   -> GND
 *   Pot middle leg    -> POT_PIN (see below)
 *   Servo signal      -> GPIO 18
 *   Servo V+          -> external 5 V supply (not the ESP32 3V3 pin!)
 *   Servo GND         -> supply GND *and* ESP32 GND (common ground)
 *
 * IMPORTANT: never connect the pot to 5 V - ESP32 analog inputs accept at
 * most 3.3 V.
 *
 * ---------------------------------------------------------------------------
 * CHOOSING THE POT PIN (must be an analog / ADC input)
 * ---------------------------------------------------------------------------
 * The pot needs a pin with an analog-to-digital converter (ADC). GPIO 19 does
 * NOT work on a classic ESP32 or an ESP32-C3 because it has no ADC there.
 * On the ESP32-S2/S3, GPIO 19 is ADC2 but is also the native USB D- line, so
 * it is a poor choice there too.
 *
 * This example therefore defaults to GPIO 34. Change POT_PIN to suit your board:
 *   Classic ESP32 : ADC1 = GPIO 32, 33, 34, 35, 36, 39   (best choice)
 *                   ADC2 = GPIO 0, 2, 4, 12-15, 25-27    (NOT usable with WiFi on)
 *                   GPIO 34, 35, 36, 39 are input-only - ideal for a pot.
 *   ESP32-S2/S3   : ADC1 = GPIO 1-10                     (best choice)
 *                   ADC2 = GPIO 11-20                    (limited with WiFi on)
 *   ESP32-C3      : ADC1 = GPIO 0-4
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
 * ---------------------------------------------------------------------------
 */
#include <ESP32_Servo.h>

const int SERVO_PIN = 18;
const int POT_PIN = 34;          // must be an ADC pin - see the notes above

const int ADC_MAX = 4095;        // 12-bit ADC reading range: 0..4095
const float SMOOTHING = 0.15f;   // 0..1, lower = smoother but slower to respond
const int MIN_CHANGE_DEG = 1;    // ignore changes smaller than this

PositionalServo servo;

float filteredRaw = 0;
int lastAngle = -1;

void setup() {
    Serial.begin(115200);

    analogReadResolution(12);
    filteredRaw = analogRead(POT_PIN);

    if (!servo.attach(SERVO_PIN)) {
        Serial.println("Failed to attach servo - check the pin number.");
        while (true) {
            delay(1000);
        }
    }
}

void loop() {
    // Exponential moving average removes ADC noise.
    int rawValue = analogRead(POT_PIN);
    filteredRaw += SMOOTHING * (rawValue - filteredRaw);

    int angle = map((long)(filteredRaw + 0.5f), 0, ADC_MAX, 0, 180);
    angle = constrain(angle, 0, 180);

    // Only move when the angle really changed, so the servo does not twitch.
    if (lastAngle < 0 || abs(angle - lastAngle) >= MIN_CHANGE_DEG) {
        servo.write(angle);
        lastAngle = angle;
        Serial.printf("pot raw: %4d  angle: %3d\n", rawValue, angle);
    }

    delay(10);
}

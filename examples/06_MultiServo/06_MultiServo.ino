/*
 * 06_MultiServo
 *
 * Drives several servos at once - here a 4-joint arm (positional servos)
 * plus one continuous rotation servo as a turntable/conveyor.
 *
 * Shows how to:
 *   - keep servos in arrays and update them all from one loop
 *   - synchronize several joints with moveTo(angle, durationMs) so they all
 *     start and arrive together, no matter how far each one has to travel
 *   - set per-servo limits, trim and direction
 *   - manage mixed servo types through a ServoBase* list
 *
 * Wiring (classic ESP32):
 *   Base      -> GPIO 13
 *   Shoulder  -> GPIO 14
 *   Elbow     -> GPIO 27
 *   Gripper   -> GPIO 26
 *   Turntable -> GPIO 25 (continuous servo)
 *
 * POWER: several servos can draw well over 1 A when moving. Use a dedicated
 * 5-6 V supply rated for it, connect its GND to the ESP32 GND, and add a
 * large capacitor (470-1000 uF) across the servo supply.
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
 * NOTE: GPIO 26 and 27 used here are fine on a classic ESP32 but are flash
 * pins on S2/S3 - remap all pins for those chips (e.g. 4, 5, 6, 7, 15 on S3).
 *
 * Channel budget: each servo uses one LEDC channel. Classic ESP32 has 16,
 * S2/S3 have 8, C3/C6 have 6.
 * ---------------------------------------------------------------------------
 */
#include <ESP32_Servo.h>

enum Joint { BASE, SHOULDER, ELBOW, GRIPPER, NUM_JOINTS };

const int JOINT_PINS[NUM_JOINTS] = {13, 14, 27, 26};
const char* JOINT_NAMES[NUM_JOINTS] = {"base", "shoulder", "elbow", "gripper"};
const int TURNTABLE_PIN = 25;

PositionalServo joints[NUM_JOINTS];
ContinuousServo turntable;

// Every servo, regardless of type, for one-line updating.
ServoBase* allServos[] = {&joints[BASE], &joints[SHOULDER], &joints[ELBOW],
                          &joints[GRIPPER], &turntable};
const int NUM_SERVOS = sizeof(allServos) / sizeof(allServos[0]);

// A sequence of arm poses: {base, shoulder, elbow, gripper} in degrees.
struct Pose {
    float angles[NUM_JOINTS];
    uint32_t durationMs;  // time to reach this pose
    const char* label;
};

const Pose POSES[] = {
    {{90, 90, 90, 30}, 1000, "home"},
    {{40, 120, 60, 30}, 1500, "reach left"},
    {{40, 135, 50, 80}, 600, "grip"},
    {{40, 90, 90, 80}, 1000, "lift"},
    {{140, 90, 90, 80}, 2000, "swing right"},
    {{140, 125, 60, 80}, 1200, "lower"},
    {{140, 125, 60, 30}, 600, "release"},
};
const int NUM_POSES = sizeof(POSES) / sizeof(POSES[0]);

int poseIndex = 0;

bool anyMoving() {
    bool moving = false;
    for (int idx = 0; idx < NUM_SERVOS; idx++) {
        // update() every servo (don't short-circuit) and remember if any is busy.
        if (allServos[idx]->update()) {
            moving = true;
        }
    }
    return moving;
}

void goToPose(int index) {
    const Pose& pose = POSES[index];
    Serial.printf("-> %s\n", pose.label);
    // Same duration for every joint = coordinated, simultaneous motion.
    for (int jnt = 0; jnt < NUM_JOINTS; jnt++) {
        joints[jnt].moveTo(pose.angles[jnt], pose.durationMs);
    }
}

void setup() {
    Serial.begin(115200);

    for (int jnt = 0; jnt < NUM_JOINTS; jnt++) {
        if (!joints[jnt].attach(JOINT_PINS[jnt])) {
            Serial.printf("Failed to attach %s on GPIO %d\n", JOINT_NAMES[jnt], JOINT_PINS[jnt]);
        }
        joints[jnt].setEasing(ServoEasing::EaseInOut);
    }

    // Per-joint tuning.
    joints[SHOULDER].setLimits(60, 150);  // keep the arm off the table
    joints[ELBOW].setReversed(true);      // mounted mirrored
    joints[ELBOW].setTrim(-4);            // horn sits 4 degrees off
    joints[GRIPPER].setLimits(20, 90);    // don't crush the gripper

    turntable.attach(TURNTABLE_PIN);
    turntable.setAcceleration(50);

    goToPose(poseIndex);
}

void loop() {
    if (!anyMoving()) {
        poseIndex = (poseIndex + 1) % NUM_POSES;

        // Slowly run the conveyor while the arm is carrying something.
        if (poseIndex == 4) {
            turntable.setSpeed(40);
        } else if (poseIndex == 6) {
            turntable.softStop();
        }

        goToPose(poseIndex);
    }
}

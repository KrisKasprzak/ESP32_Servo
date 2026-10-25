/*
 * ContinuousServo.h - 360-degree continuous rotation servo.
 *
 * Part of the ESP32_Servo library.
 *
 * ---------------------------------------------------------------------------
 * Revision History
 * ---------------------------------------------------------------------------
 * Version  Date        Author                Description
 * 1.2      2026-10-25  Kasprzak/Claude Code  Initial release: speed control with
 *                                            acceleration, timed runs and calibration.
 * ---------------------------------------------------------------------------
 *
 * MIT License
 *
 * Copyright (c) 2026 Kasprzak/Claude Code
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CONTINUOUSSERVO_H
#define CONTINUOUSSERVO_H

#define CONTINUOUSSERVO_LIBRARY_VERSION "1.2"

#include "ServoBase.h"

#define ESP32_SERVO_CONTINUOUS_MIN_US 1000
#define ESP32_SERVO_CONTINUOUS_MAX_US 2000
#define ESP32_SERVO_CONTINUOUS_NEUTRAL_US 1500

/**
 * A continuous rotation servo (e.g. FS90R, FS5103R, modified SG90) whose
 * pulse width controls speed and direction rather than position.
 *
 * Speed is expressed in percent: -100 (full reverse) .. 0 (stop) .. +100
 * (full forward). Supports neutral-point calibration, a dead band,
 * acceleration ramps and timed runs.
 */
class ContinuousServo : public ServoBase {
public:
    ContinuousServo();

    /**
     * Attach the servo to a GPIO. The servo starts stopped (neutral pulse).
     * @param pinNum     GPIO driving the servo signal line
     * @param minUs      pulse width for full reverse
     * @param maxUs      pulse width for full forward
     * @param neutralUs  pulse width at which the servo stands still
     * @param freqHz     PWM frequency
     * @param channel    LEDC channel, or ESP32_SERVO_AUTO_CHANNEL
     * @return true on success
     */
    bool attach(int pinNum,
                uint16_t minUs = ESP32_SERVO_CONTINUOUS_MIN_US,
                uint16_t maxUs = ESP32_SERVO_CONTINUOUS_MAX_US,
                uint16_t neutralUs = ESP32_SERVO_CONTINUOUS_NEUTRAL_US,
                uint32_t freqHz = ESP32_SERVO_DEFAULT_FREQUENCY_HZ,
                int channel = ESP32_SERVO_AUTO_CHANNEL);

    // ---- Speed control ---------------------------------------------------

    /**
     * Set the target speed in percent (-100..100). If an acceleration is
     * configured the speed ramps there gradually; call update() from loop().
     */
    void setSpeed(float percent);

    /** Set the speed immediately, ignoring any acceleration setting. */
    void setSpeedImmediate(float percent);

    /** @return the speed currently being output, in percent. */
    float getSpeed() const;

    /** @return the speed being ramped towards, in percent. */
    float getTargetSpeed() const;

    /** Convenience: spin forward at the given speed (0..100). */
    void forward(float percent = 100.0f);

    /** Convenience: spin backward at the given speed (0..100). */
    void backward(float percent = 100.0f);

    /** Stop immediately (neutral pulse), canceling any ramp or timed run. */
    void stop();

    /** Ramp down to zero using the configured acceleration. */
    void softStop();

    /**
     * Spin at a speed for a fixed time, then stop. Non-blocking: call
     * update() from loop().
     */
    void runFor(float percent, uint32_t durationMs);

    /** Advance ramps / timed runs. @return true while speed is changing or a timed run is active. */
    bool update() override;

    /** @return true while the output speed differs from the target. */
    bool isRamping() const;

    /** @return true if the servo is turning (speed outside the dead band). */
    bool isRunning() const;

    // ---- Configuration ---------------------------------------------------

    /**
     * Acceleration limit in percent per second (e.g. 200 = 0->100% in 0.5 s).
     * 0 disables ramping (default).
     */
    void setAcceleration(float percentPerSecond);
    float getAcceleration() const;

    /**
     * Calibrate the stop point. Many continuous servos creep slightly at
     * 1500 us; adjust until the servo stands still.
     */
    void setNeutral(uint16_t pulseUs);
    uint16_t getNeutral() const;

    /** Speeds whose magnitude is at or below this percentage output neutral. */
    void setDeadband(float percent);
    float getDeadband() const;

    /** Swap forward and reverse (useful for mirrored wheels on a robot). */
    void setReversed(bool reversed);
    bool isReversed() const;

private:
    void applySpeed(float percent);

    uint16_t _neutralUs;
    float _deadband;
    bool _reversed;

    float _accel;
    float _currentSpeed;
    float _targetSpeed;
    uint32_t _lastUpdateMs;

    bool _timedRun;
    uint32_t _runUntilMs;
};

#endif  // CONTINUOUSSERVO_H

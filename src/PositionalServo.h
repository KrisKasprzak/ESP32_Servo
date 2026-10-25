/*
 * PositionalServo.h - Standard (angle-controlled) hobby servo.
 *
 * Part of the ESP32_Servo library.
 *
 * ---------------------------------------------------------------------------
 * Revision History
 * ---------------------------------------------------------------------------
 * Version  Date        Author                Description
 * 1.2      2026-10-25  Kasprzak/Claude Code  Initial release: angle control with
 *                                            speed, easing, limits, trim and reverse.
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
#ifndef POSITIONALSERVO_H
#define POSITIONALSERVO_H

#define POSITIONALSERVO_LIBRARY_VERSION "1.2"

#include "ServoBase.h"

/** Speed profile used by non-blocking moves. */
enum class ServoEasing : uint8_t {
    Linear,     // constant speed
    EaseIn,     // start slow, finish fast
    EaseOut,    // start fast, finish slow
    EaseInOut   // slow at both ends (smoothest)
};

/**
 * A standard positional servo (e.g. SG90, MG996R) whose output shaft moves
 * to a commanded angle.
 *
 * Supports instant moves (write), speed-limited non-blocking moves
 * (moveTo + update), timed moves, easing, software angle limits, trim and
 * direction reversal.
 */
class PositionalServo : public ServoBase {
public:
    PositionalServo();

    /**
     * Attach the servo to a GPIO.
     * @param pinNum    GPIO driving the servo signal line
     * @param minUs     pulse width at 0 degrees
     * @param maxUs     pulse width at the maximum angle
     * @param maxAngle  mechanical travel in degrees (180 for most servos,
     *                  270 for wide-range models)
     * @param freqHz    PWM frequency (50 Hz for virtually all analog servos)
     * @param channel   LEDC channel, or ESP32_SERVO_AUTO_CHANNEL
     * @return true on success
     */
    bool attach(int pinNum,
                uint16_t minUs = ESP32_SERVO_DEFAULT_MIN_US,
                uint16_t maxUs = ESP32_SERVO_DEFAULT_MAX_US,
                float maxAngle = 180.0f,
                uint32_t freqHz = ESP32_SERVO_DEFAULT_FREQUENCY_HZ,
                int channel = ESP32_SERVO_AUTO_CHANNEL);

    // ---- Immediate positioning -------------------------------------------

    /** Jump to an angle immediately (cancels any move in progress). */
    void write(float angle);

    /** @return the current commanded angle in degrees. */
    float read() const;

    /** Raw pulse output; also cancels any move and updates read(). */
    void writeMicroseconds(uint16_t pulseUs) override;

    // ---- Non-blocking motion ---------------------------------------------

    /**
     * Set the default speed for moveTo().
     * @param degreesPerSecond  0 means "as fast as possible" (instant)
     */
    void setSpeed(float degreesPerSecond);
    float getSpeed() const;

    /** Set the speed profile used by moveTo(). Default: Linear. */
    void setEasing(ServoEasing easing);
    ServoEasing getEasing() const;

    /**
     * Start moving towards an angle at the configured speed.
     * Call update() regularly from loop() to advance the motion.
     * With speed 0 this behaves like write().
     */
    void moveTo(float angle);

    /**
     * Start moving towards an angle so that it arrives after durationMs,
     * regardless of the configured speed. Handy for synchronizing several
     * servos. Call update() regularly from loop().
     */
    void moveTo(float angle, uint32_t durationMs);

    /** Shift the target by a relative amount (positive or negative). */
    void moveBy(float deltaDegrees);

    /**
     * Blocking move: like moveTo() but returns only once the target is
     * reached. Convenient for simple sketches; avoid in multitasking code.
     */
    void moveToBlocking(float angle);
    void moveToBlocking(float angle, uint32_t durationMs);

    /** Advance the current move. @return true while still moving. */
    bool update() override;

    /** @return true while a non-blocking move is in progress. */
    bool isMoving() const;

    /** Freeze the servo at its present position. */
    void stop();

    /** @return the angle the servo is heading to (or holding). */
    float getTarget() const;

    // ---- Configuration ---------------------------------------------------

    /**
     * Restrict commanded angles to a sub-range, e.g. to protect a mechanism.
     * Values outside [0, maxAngle] are clipped to that range.
     */
    void setLimits(float minAngle, float maxAngle);
    float getMinLimit() const;
    float getMaxLimit() const;

    /** @return the full mechanical travel set in attach(). */
    float getMaxAngle() const;

    /**
     * Mechanical trim in degrees added to every command, to correct a horn
     * that is not mounted exactly at center.
     */
    void setTrim(float degrees);
    float getTrim() const;

    /** Mirror the direction of rotation (angle -> maxAngle - angle). */
    void setReversed(bool reversed);
    bool isReversed() const;

private:
    float clampAngle(float angle) const;
    void applyAngle(float angle);
    void beginMove(float angle, uint32_t durationMs);
    static float ease(ServoEasing easing, float progress);

    float _maxAngle;
    float _minLimit;
    float _maxLimit;
    float _trim;
    bool _reversed;

    float _speed;
    ServoEasing _easing;

    float _currentAngle;
    float _startAngle;
    float _targetAngle;
    uint32_t _moveStartMs;
    uint32_t _moveDurationMs;
    bool _moving;
};

#endif  // POSITIONALSERVO_H

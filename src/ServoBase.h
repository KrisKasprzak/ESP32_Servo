/*
 * ServoBase.h - Common PWM plumbing shared by all ESP32_Servo servo types.
 *
 * Part of the ESP32_Servo library.
 *
 * ---------------------------------------------------------------------------
 * Revision History
 * ---------------------------------------------------------------------------
 * Version  Date        Author                Description
 * 1.2      2026-10-25  Kasprzak/Claude Code  Initial release: LEDC servo output for
 *                                            Arduino-ESP32 core 2.x and 3.x.
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
#ifndef SERVOBASE_H
#define SERVOBASE_H

#define SERVOBASE_LIBRARY_VERSION "1.2"

#include <Arduino.h>

#if !defined(ARDUINO_ARCH_ESP32)
#error "ESP32_Servo only supports ESP32-family boards (it uses the LEDC peripheral)."
#endif

// Default servo signal timing (standard RC hobby servo).
#define ESP32_SERVO_DEFAULT_FREQUENCY_HZ 50
#define ESP32_SERVO_DEFAULT_MIN_US 500
#define ESP32_SERVO_DEFAULT_MAX_US 2500

// LEDC duty resolution. 14 bits is supported by every ESP32 variant
// (ESP32, S2, S3, C3, C6, H2) and gives ~1.2 us steps at 50 Hz.
#define ESP32_SERVO_RESOLUTION_BITS 14

// Sentinel meaning "let the library pick an LEDC channel".
#define ESP32_SERVO_AUTO_CHANNEL -1

/**
 * Abstract base class handling pin attachment and pulse-width output via
 * the ESP32 LEDC peripheral. Works with Arduino-ESP32 core 2.x and 3.x.
 *
 * You normally use PositionalServo or ContinuousServo instead of this class
 * directly, but a ServoBase* can be used to manage mixed servo collections.
 */
class ServoBase {
public:
    ServoBase();
    virtual ~ServoBase();

    // Servos own a hardware channel, so copying makes no sense.
    ServoBase(const ServoBase&) = delete;
    ServoBase& operator=(const ServoBase&) = delete;

    /**
     * Release the pin and its LEDC channel. The servo stops receiving
     * pulses (most servos then go limp).
     */
    void detach();

    /** @return true if the servo is currently attached to a pin. */
    bool attached() const;

    /** @return the attached GPIO, or -1 when detached. */
    int pin() const;

    /**
     * Output a raw pulse width, clamped to the configured [min, max] range.
     * @param pulseUs pulse width in microseconds
     */
    virtual void writeMicroseconds(uint16_t pulseUs);

    /** @return the last pulse width written, in microseconds. */
    uint16_t readMicroseconds() const;

    /**
     * Change the allowed pulse range (calibration). The current output is
     * re-clamped into the new range.
     */
    void setPulseRange(uint16_t minUs, uint16_t maxUs);

    uint16_t minPulse() const;
    uint16_t maxPulse() const;
    uint32_t frequency() const;

    /**
     * Advance any time-based motion. Call this often from loop() when using
     * non-blocking moves/ramps.
     * @return true while a motion is still in progress
     */
    virtual bool update();

protected:
    /**
     * Attach to a pin and configure LEDC. Derived classes expose their own
     * public attach() with type-appropriate defaults.
     * @return true on success
     */
    bool attachPin(int pinNum, uint16_t minUs, uint16_t maxUs, uint32_t freqHz, int channel);

    /** Write a pulse without triggering derived-class overrides. */
    void outputPulse(uint16_t pulseUs);

    uint16_t _minUs;
    uint16_t _maxUs;
    uint16_t _pulseUs;

private:
    uint32_t usToDuty(uint16_t pulseUs) const;

    int _pin;
    int _channel;
    uint32_t _freqHz;
};

#endif  // SERVOBASE_H

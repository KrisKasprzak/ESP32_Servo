/*
 * ESP32_Servo.h - Servo control library for ESP32 boards (Arduino core).
 *
 * Include this single header to get:
 *   - PositionalServo  : standard angle-controlled servos (0..180/270 deg)
 *   - ContinuousServo  : 360-degree continuous rotation servos
 *
 * Compatible with Arduino-ESP32 core 2.x and 3.x on ESP32, ESP32-S2,
 * ESP32-S3, ESP32-C3, ESP32-C6 and ESP32-H2.
 *
 * ---------------------------------------------------------------------------
 * Revision History
 * ---------------------------------------------------------------------------
 * Version  Date        Author                Description
 * 1.2      2026-10-25  Kasprzak/Claude Code  Initial release of the ESP32_Servo
 *                                            library umbrella header.
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
#ifndef ESP32_SERVO_H
#define ESP32_SERVO_H

#define ESP32_SERVO_LIBRARY_VERSION "1.2"

#include "ServoBase.h"
#include "PositionalServo.h"
#include "ContinuousServo.h"

#endif  // ESP32_SERVO_H

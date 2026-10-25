/*
 * ContinuousServo.cpp - 360-degree continuous rotation servo.
 *
 * Part of the ESP32_Servo library.
 * MIT License - see ContinuousServo.h or the LICENSE file in the project root.
 */
#include "ContinuousServo.h"

ContinuousServo::ContinuousServo()
    : _neutralUs(ESP32_SERVO_CONTINUOUS_NEUTRAL_US),
      _deadband(0.0f),
      _reversed(false),
      _accel(0.0f),
      _currentSpeed(0.0f),
      _targetSpeed(0.0f),
      _lastUpdateMs(0),
      _timedRun(false),
      _runUntilMs(0) {}

bool ContinuousServo::attach(int pinNum, uint16_t minUs, uint16_t maxUs, uint16_t neutralUs,
                             uint32_t freqHz, int channel) {
    if (neutralUs <= minUs || neutralUs >= maxUs) {
        return false;
    }
    if (!attachPin(pinNum, minUs, maxUs, freqHz, channel)) {
        return false;
    }
    _neutralUs = neutralUs;
    stop();
    return true;
}

// ---- Speed control ------------------------------------------------------

void ContinuousServo::setSpeed(float percent) {
    percent = constrain(percent, -100.0f, 100.0f);
    if (_accel <= 0.0f) {
        _targetSpeed = percent;
        applySpeed(percent);
        return;
    }
    // Only restart the ramp clock when starting a new ramp, so that calling
    // setSpeed() every loop iteration does not stall an ongoing ramp.
    if (!isRamping()) {
        _lastUpdateMs = millis();
    }
    _targetSpeed = percent;
}

void ContinuousServo::setSpeedImmediate(float percent) {
    percent = constrain(percent, -100.0f, 100.0f);
    _targetSpeed = percent;
    applySpeed(percent);
}

float ContinuousServo::getSpeed() const {
    return _currentSpeed;
}

float ContinuousServo::getTargetSpeed() const {
    return _targetSpeed;
}

void ContinuousServo::forward(float percent) {
    setSpeed(fabsf(percent));
}

void ContinuousServo::backward(float percent) {
    setSpeed(-fabsf(percent));
}

void ContinuousServo::stop() {
    _timedRun = false;
    _targetSpeed = 0.0f;
    applySpeed(0.0f);
}

void ContinuousServo::softStop() {
    _timedRun = false;
    setSpeed(0.0f);
}

void ContinuousServo::runFor(float percent, uint32_t durationMs) {
    setSpeed(percent);
    _runUntilMs = millis() + durationMs;
    _timedRun = true;
}

bool ContinuousServo::update() {
    uint32_t now = millis();

    if (_timedRun && (int32_t)(now - _runUntilMs) >= 0) {
        _timedRun = false;
        setSpeed(0.0f);  // honors acceleration for a smooth stop
    }

    if (_currentSpeed != _targetSpeed) {
        if (_accel <= 0.0f) {
            applySpeed(_targetSpeed);
        } else {
            float maxStep = _accel * (float)(now - _lastUpdateMs) / 1000.0f;
            if (maxStep > 0.0f) {
                float diff = _targetSpeed - _currentSpeed;
                if (fabsf(diff) <= maxStep) {
                    applySpeed(_targetSpeed);
                } else {
                    applySpeed(_currentSpeed + (diff > 0.0f ? maxStep : -maxStep));
                }
                _lastUpdateMs = now;
            }
        }
    } else {
        _lastUpdateMs = now;
    }

    return _timedRun || _currentSpeed != _targetSpeed;
}

bool ContinuousServo::isRamping() const {
    return _currentSpeed != _targetSpeed;
}

bool ContinuousServo::isRunning() const {
    return fabsf(_currentSpeed) > _deadband;
}

// ---- Configuration ------------------------------------------------------

void ContinuousServo::setAcceleration(float percentPerSecond) {
    _accel = percentPerSecond > 0.0f ? percentPerSecond : 0.0f;
    _lastUpdateMs = millis();
}

float ContinuousServo::getAcceleration() const {
    return _accel;
}

void ContinuousServo::setNeutral(uint16_t pulseUs) {
    if (pulseUs <= _minUs || pulseUs >= _maxUs) {
        return;
    }
    _neutralUs = pulseUs;
    applySpeed(_currentSpeed);
}

uint16_t ContinuousServo::getNeutral() const {
    return _neutralUs;
}

void ContinuousServo::setDeadband(float percent) {
    _deadband = constrain(percent, 0.0f, 100.0f);
    applySpeed(_currentSpeed);
}

float ContinuousServo::getDeadband() const {
    return _deadband;
}

void ContinuousServo::setReversed(bool reversed) {
    _reversed = reversed;
    applySpeed(_currentSpeed);
}

bool ContinuousServo::isReversed() const {
    return _reversed;
}

// ---- Internals ----------------------------------------------------------

void ContinuousServo::applySpeed(float percent) {
    _currentSpeed = percent;
    float signedSpeed = _reversed ? -percent : percent;
    float pulseUs;
    if (fabsf(signedSpeed) <= _deadband) {
        pulseUs = _neutralUs;
    } else if (signedSpeed > 0.0f) {
        pulseUs = _neutralUs + signedSpeed / 100.0f * (float)(_maxUs - _neutralUs);
    } else {
        pulseUs = _neutralUs + signedSpeed / 100.0f * (float)(_neutralUs - _minUs);
    }
    outputPulse((uint16_t)(pulseUs + 0.5f));
}

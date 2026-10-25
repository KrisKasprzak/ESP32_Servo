/*
 * PositionalServo.cpp - Standard (angle-controlled) hobby servo.
 *
 * Part of the ESP32_Servo library.
 * MIT License - see PositionalServo.h or the LICENSE file in the project root.
 */
#include "PositionalServo.h"

PositionalServo::PositionalServo()
    : _maxAngle(180.0f),
      _minLimit(0.0f),
      _maxLimit(180.0f),
      _trim(0.0f),
      _reversed(false),
      _speed(0.0f),
      _easing(ServoEasing::Linear),
      _currentAngle(90.0f),
      _startAngle(90.0f),
      _targetAngle(90.0f),
      _moveStartMs(0),
      _moveDurationMs(0),
      _moving(false) {}

bool PositionalServo::attach(int pinNum, uint16_t minUs, uint16_t maxUs, float maxAngle,
                             uint32_t freqHz, int channel) {
    if (maxAngle <= 0.0f) {
        return false;
    }
    if (!attachPin(pinNum, minUs, maxUs, freqHz, channel)) {
        return false;
    }
    _maxAngle = maxAngle;
    _minLimit = 0.0f;
    _maxLimit = maxAngle;
    _moving = false;
    // Hold the last commanded position (center on first attach).
    write(constrain(_currentAngle, 0.0f, maxAngle));
    return true;
}

// ---- Immediate positioning ----------------------------------------------

void PositionalServo::write(float angle) {
    _moving = false;
    angle = clampAngle(angle);
    _targetAngle = angle;
    applyAngle(angle);
}

float PositionalServo::read() const {
    return _currentAngle;
}

void PositionalServo::writeMicroseconds(uint16_t pulseUs) {
    _moving = false;
    outputPulse(pulseUs);
    // Back-calculate the logical angle so read()/moveTo() stay consistent.
    float physical = (float)(_pulseUs - _minUs) * _maxAngle / (float)(_maxUs - _minUs);
    float logical = (_reversed ? _maxAngle - physical : physical) - _trim;
    _currentAngle = logical;
    _targetAngle = logical;
}

// ---- Non-blocking motion ------------------------------------------------

void PositionalServo::setSpeed(float degreesPerSecond) {
    _speed = degreesPerSecond > 0.0f ? degreesPerSecond : 0.0f;
}

float PositionalServo::getSpeed() const {
    return _speed;
}

void PositionalServo::setEasing(ServoEasing easing) {
    _easing = easing;
}

ServoEasing PositionalServo::getEasing() const {
    return _easing;
}

void PositionalServo::moveTo(float angle) {
    angle = clampAngle(angle);
    if (_speed <= 0.0f) {
        write(angle);
        return;
    }
    float distance = fabsf(angle - _currentAngle);
    beginMove(angle, (uint32_t)(distance / _speed * 1000.0f));
}

void PositionalServo::moveTo(float angle, uint32_t durationMs) {
    beginMove(clampAngle(angle), durationMs);
}

void PositionalServo::moveBy(float deltaDegrees) {
    moveTo(_targetAngle + deltaDegrees);
}

void PositionalServo::moveToBlocking(float angle) {
    moveTo(angle);
    while (update()) {
        delay(1);
    }
}

void PositionalServo::moveToBlocking(float angle, uint32_t durationMs) {
    moveTo(angle, durationMs);
    while (update()) {
        delay(1);
    }
}

void PositionalServo::beginMove(float angle, uint32_t durationMs) {
    if (durationMs == 0 || angle == _currentAngle) {
        write(angle);
        return;
    }
    _startAngle = _currentAngle;
    _targetAngle = angle;
    _moveStartMs = millis();
    _moveDurationMs = durationMs;
    _moving = true;
}

bool PositionalServo::update() {
    if (!_moving) {
        return false;
    }
    uint32_t elapsed = millis() - _moveStartMs;
    if (elapsed >= _moveDurationMs) {
        _moving = false;
        applyAngle(_targetAngle);
        return false;
    }
    float progress = ease(_easing, (float)elapsed / (float)_moveDurationMs);
    applyAngle(_startAngle + (_targetAngle - _startAngle) * progress);
    return true;
}

bool PositionalServo::isMoving() const {
    return _moving;
}

void PositionalServo::stop() {
    if (_moving) {
        update();
    }
    _moving = false;
    _targetAngle = _currentAngle;
}

float PositionalServo::getTarget() const {
    return _targetAngle;
}

// ---- Configuration ------------------------------------------------------

void PositionalServo::setLimits(float minAngle, float maxAngle) {
    if (minAngle > maxAngle) {
        float swapTmp = minAngle;
        minAngle = maxAngle;
        maxAngle = swapTmp;
    }
    _minLimit = constrain(minAngle, 0.0f, _maxAngle);
    _maxLimit = constrain(maxAngle, 0.0f, _maxAngle);
    if (_currentAngle < _minLimit || _currentAngle > _maxLimit) {
        write(_currentAngle);
    } else if (_moving) {
        _targetAngle = clampAngle(_targetAngle);
    }
}

float PositionalServo::getMinLimit() const {
    return _minLimit;
}

float PositionalServo::getMaxLimit() const {
    return _maxLimit;
}

float PositionalServo::getMaxAngle() const {
    return _maxAngle;
}

void PositionalServo::setTrim(float degrees) {
    _trim = degrees;
    applyAngle(_currentAngle);
}

float PositionalServo::getTrim() const {
    return _trim;
}

void PositionalServo::setReversed(bool reversed) {
    _reversed = reversed;
    applyAngle(_currentAngle);
}

bool PositionalServo::isReversed() const {
    return _reversed;
}

// ---- Internals ----------------------------------------------------------

float PositionalServo::clampAngle(float angle) const {
    return constrain(angle, _minLimit, _maxLimit);
}

void PositionalServo::applyAngle(float angle) {
    _currentAngle = angle;
    float physical = angle + _trim;
    if (_reversed) {
        physical = _maxAngle - physical;
    }
    physical = constrain(physical, 0.0f, _maxAngle);
    float pulseUs = _minUs + physical * (float)(_maxUs - _minUs) / _maxAngle;
    outputPulse((uint16_t)(pulseUs + 0.5f));
}

float PositionalServo::ease(ServoEasing easing, float progress) {
    switch (easing) {
        case ServoEasing::EaseIn:
            return progress * progress;
        case ServoEasing::EaseOut:
            return progress * (2.0f - progress);
        case ServoEasing::EaseInOut:
            return progress < 0.5f ? 2.0f * progress * progress
                                   : -1.0f + (4.0f - 2.0f * progress) * progress;
        case ServoEasing::Linear:
        default:
            return progress;
    }
}

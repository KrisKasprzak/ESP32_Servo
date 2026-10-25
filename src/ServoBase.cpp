/*
 * ServoBase.cpp - Common PWM plumbing shared by all ESP32_Servo servo types.
 *
 * Part of the ESP32_Servo library.
 * MIT License - see ServoBase.h or the LICENSE file in the project root.
 */
#include "ServoBase.h"

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#define ESP32_SERVO_LEDC_V3 1
#else
#define ESP32_SERVO_LEDC_V3 0
#include "soc/soc_caps.h"
// Core 2.x exposes both high- and low-speed LEDC groups as one channel range.
#ifdef SOC_LEDC_SUPPORT_HS_MODE
#define ESP32_SERVO_MAX_CHANNELS (SOC_LEDC_CHANNEL_NUM * 2)
#else
#define ESP32_SERVO_MAX_CHANNELS (SOC_LEDC_CHANNEL_NUM)
#endif
#endif

#if !ESP32_SERVO_LEDC_V3
// Core 2.x requires the caller to manage LEDC channels. Track which ones
// this library has handed out so multiple servos never collide.
static uint32_t s_usedChannels = 0;

static int allocateChannel(int requested) {
    if (requested >= 0) {
        if (requested >= ESP32_SERVO_MAX_CHANNELS || (s_usedChannels & (1UL << requested))) {
            return -1;
        }
        s_usedChannels |= (1UL << requested);
        return requested;
    }
    for (int chan = 0; chan < ESP32_SERVO_MAX_CHANNELS; chan++) {
        if (!(s_usedChannels & (1UL << chan))) {
            s_usedChannels |= (1UL << chan);
            return chan;
        }
    }
    return -1;
}

static void releaseChannel(int chan) {
    if (chan >= 0) {
        s_usedChannels &= ~(1UL << chan);
    }
}
#endif

ServoBase::ServoBase()
    : _minUs(ESP32_SERVO_DEFAULT_MIN_US),
      _maxUs(ESP32_SERVO_DEFAULT_MAX_US),
      _pulseUs(0),
      _pin(-1),
      _channel(-1),
      _freqHz(ESP32_SERVO_DEFAULT_FREQUENCY_HZ) {}

ServoBase::~ServoBase() {
    detach();
}

bool ServoBase::attachPin(int pinNum, uint16_t minUs, uint16_t maxUs, uint32_t freqHz, int channel) {
    if (pinNum < 0 || freqHz == 0 || minUs >= maxUs || maxUs >= 1000000UL / freqHz) {
        return false;
    }
    if (attached()) {
        detach();
    }

#if ESP32_SERVO_LEDC_V3
    bool attachOk = (channel >= 0)
                        ? ledcAttachChannel(pinNum, freqHz, ESP32_SERVO_RESOLUTION_BITS, channel)
                        : ledcAttach(pinNum, freqHz, ESP32_SERVO_RESOLUTION_BITS);
    if (!attachOk) {
        return false;
    }
#else
    int chan = allocateChannel(channel);
    if (chan < 0) {
        return false;
    }
    if (ledcSetup(chan, freqHz, ESP32_SERVO_RESOLUTION_BITS) == 0) {
        releaseChannel(chan);
        return false;
    }
    ledcAttachPin(pinNum, chan);
    _channel = chan;
#endif

    _pin = pinNum;
    _freqHz = freqHz;
    _minUs = minUs;
    _maxUs = maxUs;
    return true;
}

void ServoBase::detach() {
    if (!attached()) {
        return;
    }
#if ESP32_SERVO_LEDC_V3
    ledcDetach(_pin);
#else
    ledcWrite(_channel, 0);
    ledcDetachPin(_pin);
    releaseChannel(_channel);
#endif
    _pin = -1;
    _channel = -1;
}

bool ServoBase::attached() const {
    return _pin >= 0;
}

int ServoBase::pin() const {
    return _pin;
}

void ServoBase::writeMicroseconds(uint16_t pulseUs) {
    outputPulse(pulseUs);
}

uint16_t ServoBase::readMicroseconds() const {
    return _pulseUs;
}

void ServoBase::outputPulse(uint16_t pulseUs) {
    pulseUs = constrain(pulseUs, _minUs, _maxUs);
    _pulseUs = pulseUs;
    if (!attached()) {
        return;
    }
#if ESP32_SERVO_LEDC_V3
    ledcWrite(_pin, usToDuty(pulseUs));
#else
    ledcWrite(_channel, usToDuty(pulseUs));
#endif
}

void ServoBase::setPulseRange(uint16_t minUs, uint16_t maxUs) {
    if (minUs >= maxUs) {
        return;
    }
    _minUs = minUs;
    _maxUs = maxUs;
    if (_pulseUs != 0) {
        outputPulse(_pulseUs);
    }
}

uint16_t ServoBase::minPulse() const {
    return _minUs;
}

uint16_t ServoBase::maxPulse() const {
    return _maxUs;
}

uint32_t ServoBase::frequency() const {
    return _freqHz;
}

bool ServoBase::update() {
    return false;
}

uint32_t ServoBase::usToDuty(uint16_t pulseUs) const {
    const uint32_t maxDuty = (1UL << ESP32_SERVO_RESOLUTION_BITS) - 1;
    // duty = pulse / period * maxDuty, with period = 1e6 / freq.
    return (uint32_t)(((uint64_t)pulseUs * _freqHz * maxDuty) / 1000000ULL);
}

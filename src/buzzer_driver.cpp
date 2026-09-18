#include "buzzer_driver.h"
#include "c4_pins.h"
#include "c4_config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

BuzzerDriver::BuzzerDriver()
    : m_cadence(BeepCadence::OFF)
    , m_isOutputHigh(false)
    , m_lastToggleTime(0)
    , m_periodMs(1000)
    , m_pulseWidthMs(80)
    , m_frequencyHz(C4Config::BUZZER_FREQ_BOMB_ALERT)
    , m_singleBeepEndMs(0)
    , m_singleBeepActive(false)
{
}

void BuzzerDriver::init() {
#ifdef ARDUINO
    pinMode(C4Pins::PIN_BUZZER, OUTPUT);
    noTone(C4Pins::PIN_BUZZER);
#endif
    setCadence(BeepCadence::OFF);
}

void BuzzerDriver::startTone(uint32_t freqHz) {
    m_isOutputHigh = true;
#ifdef ARDUINO
    tone(C4Pins::PIN_BUZZER, freqHz);
#else
    (void)freqHz;
#endif
}

void BuzzerDriver::stopTone() {
    m_isOutputHigh = false;
#ifdef ARDUINO
    noTone(C4Pins::PIN_BUZZER);
#endif
}

void BuzzerDriver::setCadence(BeepCadence cadence) {
    if (m_cadence != cadence) {
        m_cadence = cadence;
        applyCadenceParams(cadence);
        m_lastToggleTime = 0;
        m_singleBeepActive = false;
    }
}

BeepCadence BuzzerDriver::getCadence() const {
    return m_cadence;
}

void BuzzerDriver::applyCadenceParams(BeepCadence cadence) {
    m_frequencyHz = C4Config::BUZZER_FREQ_BOMB_ALERT; // 4.200 Hz padrão de ressonância

    switch (cadence) {
        case BeepCadence::SLOW_1HZ:
            m_periodMs = 1000;
            m_pulseWidthMs = 80;
            break;
        case BeepCadence::MEDIUM_2HZ:
            m_periodMs = 500;
            m_pulseWidthMs = 80;
            break;
        case BeepCadence::FAST_4HZ:
            m_periodMs = 250;
            m_pulseWidthMs = 80;
            break;
        case BeepCadence::CONTINUOUS:
            m_periodMs = 100;
            m_pulseWidthMs = 100;
            break;
        case BeepCadence::ERROR_TONE:
            m_frequencyHz = C4Config::BUZZER_FREQ_ERROR; // 200 Hz grave
            m_periodMs = 1000;
            m_pulseWidthMs = 1000;
            break;
        case BeepCadence::SUCCESS_DOUBLE:
            m_frequencyHz = C4Config::BUZZER_FREQ_CONFIRM; // 2.000 Hz
            m_periodMs = 400;
            m_pulseWidthMs = 150;
            break;
        case BeepCadence::OFF:
        default:
            m_periodMs = 0;
            m_pulseWidthMs = 0;
            stopTone();
            break;
    }
}

void BuzzerDriver::triggerSingleBeep(uint32_t frequencyHz, uint32_t durationMs, uint32_t currentMillis) {
    m_singleBeepActive = true;
    m_singleBeepEndMs = currentMillis + durationMs;
    startTone(frequencyHz);
}

void BuzzerDriver::triggerKeyBeep(uint32_t currentMillis) {
    triggerSingleBeep(C4Config::BUZZER_FREQ_KEYPRESS, C4Config::BUZZER_KEYPRESS_DURATION_MS, currentMillis);
}

void BuzzerDriver::update(uint32_t currentMillis) {
    if (m_singleBeepActive) {
        if (currentMillis >= m_singleBeepEndMs) {
            m_singleBeepActive = false;
            stopTone();
        }
        return;
    }

    if (m_cadence == BeepCadence::OFF) {
        if (m_isOutputHigh) {
            stopTone();
        }
        return;
    }

    if (m_cadence == BeepCadence::CONTINUOUS) {
        if (!m_isOutputHigh) {
            startTone(m_frequencyHz);
        }
        return;
    }

    if (m_lastToggleTime == 0) {
        m_lastToggleTime = currentMillis;
        startTone(m_frequencyHz);
        return;
    }

    uint32_t elapsed = currentMillis - m_lastToggleTime;

    if (m_isOutputHigh) {
        if (elapsed >= m_pulseWidthMs) {
            stopTone();
        }
    } else {
        if (elapsed >= m_periodMs) {
            m_lastToggleTime = currentMillis;
            startTone(m_frequencyHz);
        }
    }
}

bool BuzzerDriver::isCurrentlyBeeping() const {
    return m_isOutputHigh;
}

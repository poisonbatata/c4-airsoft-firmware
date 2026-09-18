#include "button_driver.h"
#include "c4_pins.h"
#include "c4_config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

ButtonDriver::ButtonDriver()
    : m_rawState(false)
    , m_debouncedState(false)
    , m_lastDebouncedState(false)
    , m_lastTransitionTime(0)
{
}

void ButtonDriver::init() {
#ifdef ARDUINO
    pinMode(C4Pins::PIN_BUTTON_DEFUSE, INPUT_PULLUP);
    // Ativo em nível LOW: lido como true quando LOW
    m_debouncedState = (digitalRead(C4Pins::PIN_BUTTON_DEFUSE) == LOW);
#else
    m_debouncedState = false;
#endif
    m_lastDebouncedState = m_debouncedState;
    m_rawState = m_debouncedState;
}

void ButtonDriver::update(uint32_t currentMillis) {
    m_lastDebouncedState = m_debouncedState;

#ifdef ARDUINO
    bool currentReading = (digitalRead(C4Pins::PIN_BUTTON_DEFUSE) == LOW);
#else
    bool currentReading = m_rawState;
#endif

    if (currentReading != m_rawState) {
        m_rawState = currentReading;
        m_lastTransitionTime = currentMillis;
    }

    if ((currentMillis - m_lastTransitionTime) >= C4Config::BUTTON_DEBOUNCE_MS) {
        m_debouncedState = m_rawState;
    }
}

bool ButtonDriver::isPressed() const {
    return m_debouncedState;
}

void ButtonDriver::setMockPressed(bool pressed) {
    m_rawState = pressed;
    m_debouncedState = pressed;
    m_lastDebouncedState = pressed;
}

bool ButtonDriver::wasJustPressed() const {
    return (m_debouncedState && !m_lastDebouncedState);
}

bool ButtonDriver::wasJustReleased() const {
    return (!m_debouncedState && m_lastDebouncedState);
}

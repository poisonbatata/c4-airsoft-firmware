#include "keypad_driver.h"
#include "c4_pins.h"
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <Keypad.h>

static const byte ROWS = 4;
static const byte COLS = 4;

static char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static byte rowPins[ROWS] = {
    C4Pins::PIN_KEYPAD_ROW1,
    C4Pins::PIN_KEYPAD_ROW2,
    C4Pins::PIN_KEYPAD_ROW3,
    C4Pins::PIN_KEYPAD_ROW4
};

static byte colPins[COLS] = {
    C4Pins::PIN_KEYPAD_COL1,
    C4Pins::PIN_KEYPAD_COL2,
    C4Pins::PIN_KEYPAD_COL3,
    C4Pins::PIN_KEYPAD_COL4
};

static Keypad s_keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
#endif

static char s_injectedKey = '\0';

KeypadDriver::KeypadDriver() : m_length(0) {
    clearBuffer();
}

void KeypadDriver::init() {
#ifdef ARDUINO
    s_keypad.setDebounceTime(C4Config::KEYPAD_DEBOUNCE_MS);
#endif
    clearBuffer();
    s_injectedKey = '\0';
}

void KeypadDriver::injectKey(char c) {
    s_injectedKey = c;
}

char KeypadDriver::pollKey() {
    if (s_injectedKey != '\0') {
        char k = s_injectedKey;
        s_injectedKey = '\0';
        return k;
    }
#ifdef ARDUINO
    return s_keypad.getKey();
#else
    return '\0';
#endif
}

KeyAction KeypadDriver::handleInput(char c) {
    if (c == '\0') {
        return KeyAction::NONE;
    }

    // Tecla 'A' é o comando canônico de limpeza do buffer (AC-02, AC-14)
    if (c == 'A') {
        clearBuffer();
        return KeyAction::CLEARED;
    }

    // Tecla '#' é o comando canônico de submissão/Enter da senha digitada (AC-14)
    if (c == '#') {
        return KeyAction::SUBMIT;
    }

    // Dígitos numéricos '0' a '9' adicionados ao buffer até INPUT_BUFFER_MAX_LEN (sem truncamento silencioso)
    if (c >= '0' && c <= '9') {
        if (m_length < C4Config::INPUT_BUFFER_MAX_LEN) {
            m_buffer[m_length] = c;
            m_length++;
            m_buffer[m_length] = '\0';
            return KeyAction::DIGIT_ADDED;
        }
    }

    return KeyAction::NONE;
}

void KeypadDriver::clearBuffer() {
    m_length = 0;
    memset(m_buffer, 0, sizeof(m_buffer));
}

const char* KeypadDriver::getBuffer() const {
    return m_buffer;
}

uint8_t KeypadDriver::getLength() const {
    return m_length;
}

bool KeypadDriver::isComplete() const {
    return m_length == C4Config::PASSWORD_MAX_LEN;
}

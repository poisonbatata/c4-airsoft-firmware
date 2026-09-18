#include "led_driver.h"
#include "c4_pins.h"

#ifdef ARDUINO
#include <FastLED.h>
static CRGB s_leds[C4Config::LED_STRIP_COUNT];
#endif

LedDriver::LedDriver()
    : m_pattern(LedPattern::OFF)
    , m_needsRedraw(true)
    , m_lastUpdateMs(0)
{
}

void LedDriver::init() {
#ifdef ARDUINO
    FastLED.addLeds<WS2812B, C4Pins::PIN_LED_WS2812B, GRB>(s_leds, C4Config::LED_STRIP_COUNT);
    FastLED.setBrightness(C4Config::LED_BRIGHTNESS_LIMIT);
#endif
    clear();
}

void LedDriver::setPattern(LedPattern pattern) {
    if (m_pattern != pattern) {
        m_pattern = pattern;
        m_needsRedraw = true;
    }
}

LedPattern LedDriver::getPattern() const {
    return m_pattern;
}

void LedDriver::applyColor(uint8_t r, uint8_t g, uint8_t b) {
#ifdef ARDUINO
    for (uint16_t i = 0; i < C4Config::LED_STRIP_COUNT; ++i) {
        s_leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
#else
    (void)r; (void)g; (void)b;
#endif
}

void LedDriver::clear() {
    applyColor(0, 0, 0);
}

void LedDriver::update(uint32_t currentMillis, bool syncBuzzerPulse) {
    (void)currentMillis;

    switch (m_pattern) {
        case LedPattern::OFF:
            if (m_needsRedraw) {
                clear();
                m_needsRedraw = false;
            }
            break;

        case LedPattern::BLINK_RED:
            // Sincronizado perfeitamente com o pulso sonoro do buzzer (AC-05)
            if (syncBuzzerPulse) {
                applyColor(255, 0, 0);
            } else {
                clear();
            }
            break;

        case LedPattern::BLINK_BLUE:
            // Sincronizado perfeitamente com o pulso sonoro do buzzer (H&S Time Azul armado)
            if (syncBuzzerPulse) {
                applyColor(0, 0, 255);
            } else {
                clear();
            }
            break;

        case LedPattern::SOLID_BLUE:
            // Vitória CT / Time Azul: Todos os LEDs em azul puro (AC-11)
            if (m_needsRedraw) {
                applyColor(0, 0, 255);
                m_needsRedraw = false;
            }
            break;

        case LedPattern::SOLID_RED:
            // Vitória Time Vermelho: Todos os LEDs em vermelho puro
            if (m_needsRedraw) {
                applyColor(255, 0, 0);
                m_needsRedraw = false;
            }
            break;

        case LedPattern::SOLID_YELLOW:
            // Vitória TR: Todos os LEDs em amarelo puro (AC-12)
            if (m_needsRedraw) {
                applyColor(255, 200, 0);
                m_needsRedraw = false;
            }
            break;

        case LedPattern::MENU_IDLE:
            if (m_needsRedraw) {
                // Luz suave branca/âmbar para indicação de stand-by
                applyColor(20, 20, 20);
                m_needsRedraw = false;
            }
            break;

        case LedPattern::ERROR_FLASH:
            if ((currentMillis / 100) % 2 == 0) {
                applyColor(255, 0, 0);
            } else {
                clear();
            }
            break;
    }
}

#pragma once

#include <stdint.h>
#include "c4_types.h"
#include "c4_config.h"

/**
 * @file led_driver.h
 * @brief Controlador da fita de LEDs endereçáveis WS2812B
 * @ref C4A-ELE-0050, IF-005 e spec.md (AC-05, AC-11, AC-12)
 */

class LedDriver {
public:
    LedDriver();
    void init();

    /**
     * @brief Define o padrão visual ativo da fita.
     */
    void setPattern(LedPattern pattern);

    /**
     * @brief Retorna o padrão visual ativo.
     */
    LedPattern getPattern() const;

    /**
     * @brief Atualiza a animação dos LEDs. Não-bloqueante.
     * @param currentMillis Timestamp atual do sistema.
     * @param syncBuzzerPulse Informa se o buzzer está no pico de som para sincronizar a luz vermelha.
     */
    void update(uint32_t currentMillis, bool syncBuzzerPulse);

    /**
     * @brief Apaga todos os LEDs da fita.
     */
    void clear();

private:
    LedPattern m_pattern;
    bool m_needsRedraw;
    uint32_t m_lastUpdateMs;

    void applyColor(uint8_t r, uint8_t g, uint8_t b);
};

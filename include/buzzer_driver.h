#pragma once

#include <stdint.h>
#include "c4_types.h"
#include "c4_config.h"

/**
 * @file buzzer_driver.h
 * @brief Controlador assíncrono e não-bloqueante do buzzer com modulação de frequência (tone)
 * @ref C4A-ELE-0040, IF-004 e spec.md (AC-04, AC-05, AC-16)
 */

class BuzzerDriver {
public:
    BuzzerDriver();
    void init();

    /**
     * @brief Define a cadência sonora atual do buzzer durante a contagem regressiva.
     */
    void setCadence(BeepCadence cadence);

    /**
     * @brief Retorna a cadência atualmente configurada.
     */
    BeepCadence getCadence() const;

    /**
     * @brief Atualiza a oscilação do buzzer via temporização millis(). Não-bloqueante.
     * @param currentMillis Timestamp atual do sistema.
     */
    void update(uint32_t currentMillis);

    /**
     * @brief Informa se o buzzer está no estado ativo (emitindo som) neste instante.
     * Utilizado para sincronizar o piscar da fita LED.
     */
    bool isCurrentlyBeeping() const;

    /**
     * @brief Dispara um bip único com frequência e duração customizadas.
     */
    void triggerSingleBeep(uint32_t frequencyHz, uint32_t durationMs, uint32_t currentMillis);

    /**
     * @brief Dispara o bip de feedback ao pressionar uma tecla (3.000 Hz, 50 ms).
     */
    void triggerKeyBeep(uint32_t currentMillis);

private:
    BeepCadence m_cadence;
    bool m_isOutputHigh;
    uint32_t m_lastToggleTime;
    uint32_t m_periodMs;
    uint32_t m_pulseWidthMs;
    uint32_t m_frequencyHz;
    uint32_t m_singleBeepEndMs;
    bool m_singleBeepActive;

    void applyCadenceParams(BeepCadence cadence);
    void startTone(uint32_t freqHz);
    void stopTone();
};

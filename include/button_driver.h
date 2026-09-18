#pragma once

#include <stdint.h>

/**
 * @file button_driver.h
 * @brief Driver com debounce temporal do botão físico de defuse
 * @ref C4A-ELE-0030, IF-003 e spec.md (AC-07, AC-08)
 */

class ButtonDriver {
public:
    ButtonDriver();
    void init();

    /**
     * @brief Atualiza a leitura do pino com debounce temporal. Não-bloqueante.
     * @param currentMillis Timestamp atual do sistema.
     */
    void update(uint32_t currentMillis);

    /**
     * @brief Retorna se o botão está atualmente mantido pressionado (LOW estável).
     */
    bool isPressed() const;

    /**
     * @brief Força o estado do botão programaticamente (utilizado em testes unitários automatizados).
     */
    void setMockPressed(bool pressed);

    /**
     * @brief Retorna true apenas no ciclo em que o botão foi recém-pressionado.
     */
    bool wasJustPressed() const;

    /**
     * @brief Retorna true apenas no ciclo em que o botão foi recém-solto.
     */
    bool wasJustReleased() const;

private:
    bool m_rawState;
    bool m_debouncedState;
    bool m_lastDebouncedState;
    uint32_t m_lastTransitionTime;
};

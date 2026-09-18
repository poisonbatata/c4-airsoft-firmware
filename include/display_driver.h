#pragma once

#include <stdint.h>
#include "c4_config.h"

/**
 * @file display_driver.h
 * @brief Gerenciador do display LCD 16x2 I2C com shadow buffer anti-flicker
 * @ref C4A-ELE-0060, IF-006 e plan.md
 */

class DisplayDriver {
public:
    DisplayDriver();
    void init();

    /**
     * @brief Renderiza a tela do menu inicial.
     */
    void showMenu();

    /**
     * @brief Exibe prompt de digitação de senha de plantio (TR).
     * @param buffer Senha digitada até o momento.
     * @param maskPassword true para asteriscos (*), false para texto claro.
     */
    void showPlantPrompt(const char* buffer, bool maskPassword = true);

    /**
     * @brief Exibe prompt de digitação de senha de defuse (CT).
     * @param buffer Senha digitada até o momento.
     * @param maskPassword true para asteriscos (*), false para texto claro.
     */
    void showDefusePrompt(const char* buffer, bool maskPassword = true);

    /**
     * @brief Exibe a contagem regressiva da C4 plantada.
     * @param remainingMs Tempo restante em milissegundos.
     */
    void showCountdown(uint32_t remainingMs);

    /**
     * @brief Exibe o progresso de defuse e tempo restante.
     * @param remainingMs Tempo restante de rodada.
     * @param accumulatedDefuseMs Tempo já acumulado de botão mantido.
     * @param totalDefuseMs Tempo total necessário para defuse.
     * @param isPaused Indica se o botão foi solto e está em tolerância.
     */
    void showDefusing(uint32_t remainingMs, uint32_t accumulatedDefuseMs, uint32_t totalDefuseMs, bool isPaused);

    /**
     * @brief Exibe tela de vitória dos Contra-Terroristas (Defusada).
     */
    void showDefusedCT();

    /**
     * @brief Exibe tela de vitória dos Terroristas (Explosão cenográfica).
     */
    void showExplodedTR();

    /**
     * @brief Exibe prompt de digitação de senha de plantio no modo Hide and Seek.
     * @param buffer Senha digitada até o momento.
     * @param maskPassword true para asteriscos (*), false para texto claro.
     */
    void showPlantPromptHS(const char* buffer, bool maskPassword = true);

    /**
     * @brief Exibe prompt de digitação de senha de defuse no modo Hide and Seek.
     * @param buffer Senha digitada até o momento.
     * @param maskPassword true para asteriscos (*), false para texto claro.
     * @param defusingTeamIsRed true se o time que está defusando é Vermelho, false se Azul.
     */
    void showDefusePromptHS(const char* buffer, bool maskPassword = true, bool defusingTeamIsRed = true);

    /**
     * @brief Exibe tela de vitória do Time Azul (H&S).
     */
    void showVictoryBlue();

    /**
     * @brief Exibe tela de vitória do Time Vermelho (H&S).
     */
    void showVictoryRed();

    /**
     * @brief Exibe tela de aviso de senha inválida ou erro.
     * @param errorMsg Mensagem de erro para a linha 1.
     */
    void showError(const char* errorMsg);

    /**
     * @brief Atualiza a tela via I2C apenas se os caracteres mudaram. Não-bloqueante.
     */
    void update(uint32_t currentMillis);

private:
    char m_targetLine0[C4Config::LCD_COLS + 1];
    char m_targetLine1[C4Config::LCD_COLS + 1];
    char m_currentLine0[C4Config::LCD_COLS + 1];
    char m_currentLine1[C4Config::LCD_COLS + 1];
    uint32_t m_lastRefreshMs;

    void setLines(const char* line0, const char* line1);
};

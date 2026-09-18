#pragma once

#include <stdint.h>
#include "c4_types.h"
#include "c4_config.h"
#include "keypad_driver.h"
#include "button_driver.h"
#include "buzzer_driver.h"
#include "led_driver.h"
#include "display_driver.h"
#include "web_server_manager.h"

/**
 * @file game_fsm.h
 * @brief Controlador central da Máquina de Estados Finitos (FSM) do C4 Airsoft
 * @ref spec.md (Seção 4), plan.md (Seção 1) e C4A-ICD-001
 */

class C4GameFSM {
public:
    C4GameFSM(KeypadDriver& keypad,
              ButtonDriver& button,
              BuzzerDriver& buzzer,
              LedDriver& led,
              DisplayDriver& display,
              WebServerManager& webServer);

    /**
     * @brief Inicializa a máquina de estados no estado S1_INIT.
     */
    void init();

    /**
     * @brief Ciclo de execução periódica (tick) da FSM. Deve ser chamado no loop() do main.
     * @param currentMillis Timestamp atual em milissegundos.
     */
    void update(uint32_t currentMillis);

    /**
     * @brief Retorna o estado atual da FSM.
     */
    GameState getState() const;

    /**
     * @brief Retorna o tempo restante de contagem em milissegundos.
     */
    uint32_t getRemainingTimeMs() const;

    /**
     * @brief Retorna a estrutura de progresso de defuse.
     */
    const DefuseProgress& getDefuseProgress() const;

    /**
     * @brief Retorna o modo de jogo atual.
     */
    GameMode getGameMode() const;

    /**
     * @brief Retorna o time que armou a bomba (no modo Hide & Seek).
     */
    Team getPlantedTeam() const;

private:
    KeypadDriver&      m_keypad;
    ButtonDriver&      m_button;
    BuzzerDriver&      m_buzzer;
    LedDriver&         m_led;
    DisplayDriver&     m_display;
    WebServerManager&  m_webServer;

    GameState          m_state;
    GameMode           m_mode;
    Team               m_plantedTeam;

    uint32_t           m_plantedTimestampMs;
    uint32_t           m_remainingTimeMs;
    uint32_t           m_totalBombTimeMs;
    uint32_t           m_activeDefuseTargetMs;
    uint32_t           m_lastTickMs;

    DefuseProgress     m_defuse;

    // Transições de estado
    void transitionTo(GameState nextState, uint32_t currentMillis);

    // Manipuladores de estados específicos
    void handleStateInit(uint32_t currentMillis);
    void handleStateMenu(uint32_t currentMillis);
    void handleStateReady(uint32_t currentMillis);
    void handleStatePlanted(uint32_t currentMillis, uint32_t deltaMs);
    void handleStateDefusing(uint32_t currentMillis, uint32_t deltaMs);
    void handleStateDefused(uint32_t currentMillis);
    void handleStateExploded(uint32_t currentMillis);

    // Utilitários de lógica
    void updatePlantedCadence(uint32_t remainingMs);
};

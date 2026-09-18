#include "game_fsm.h"
#include <string.h>

C4GameFSM::C4GameFSM(KeypadDriver& keypad,
                     ButtonDriver& button,
                     BuzzerDriver& buzzer,
                     LedDriver& led,
                     DisplayDriver& display,
                     WebServerManager& webServer)
    : m_keypad(keypad)
    , m_button(button)
    , m_buzzer(buzzer)
    , m_led(led)
    , m_display(display)
    , m_webServer(webServer)
    , m_state(GameState::S1_INIT)
    , m_mode(GameMode::CS_GO)
    , m_plantedTeam(Team::NONE)
    , m_plantedTimestampMs(0)
    , m_remainingTimeMs(0)
    , m_totalBombTimeMs(C4Config::BOMB_COUNTDOWN_MS)
    , m_activeDefuseTargetMs(C4Config::DEFUSE_TOTAL_TIME_MS)
    , m_lastTickMs(0)
{
    m_defuse = {0, 0, false, false};
}

void C4GameFSM::init() {
    m_keypad.init();
    m_button.init();
    m_buzzer.init();
    m_led.init();
    m_display.init();

    m_state = GameState::S1_INIT;
    m_lastTickMs = 0;
    m_plantedTeam = Team::NONE;
    m_totalBombTimeMs = m_webServer.getConfig().bombTimeSeconds * 1000;
    m_activeDefuseTargetMs = m_webServer.getConfig().defuseTimeSeconds * 1000;
    m_defuse = {0, 0, false, false};
}

GameState C4GameFSM::getState() const {
    return m_state;
}

GameMode C4GameFSM::getGameMode() const {
    return m_mode;
}

Team C4GameFSM::getPlantedTeam() const {
    return m_plantedTeam;
}

uint32_t C4GameFSM::getRemainingTimeMs() const {
    return m_remainingTimeMs;
}

const DefuseProgress& C4GameFSM::getDefuseProgress() const {
    return m_defuse;
}

void C4GameFSM::transitionTo(GameState nextState, uint32_t currentMillis) {
    m_state = nextState;

    switch (m_state) {
        case GameState::S1_INIT:
            break;

        case GameState::S2_MENU:
            m_buzzer.setCadence(BeepCadence::OFF);
            m_led.setPattern(LedPattern::MENU_IDLE);
            m_display.showMenu();
            m_keypad.clearBuffer();
            m_plantedTeam = Team::NONE;
            m_defuse = {0, 0, false, false};
            m_remainingTimeMs = 0;
            m_plantedTimestampMs = 0;
            m_activeDefuseTargetMs = m_webServer.getConfig().defuseTimeSeconds * 1000;
            if (m_activeDefuseTargetMs < 1000) m_activeDefuseTargetMs = 10000;
            break;

        case GameState::S3_READY:
            m_buzzer.setCadence(BeepCadence::OFF);
            m_led.setPattern(LedPattern::OFF);
            m_keypad.clearBuffer();
            m_plantedTeam = Team::NONE;
            m_defuse = {0, 0, false, false};
            m_remainingTimeMs = 0;
            m_plantedTimestampMs = 0;
            m_activeDefuseTargetMs = m_webServer.getConfig().defuseTimeSeconds * 1000;
            if (m_activeDefuseTargetMs < 1000) m_activeDefuseTargetMs = 10000;
            if (m_mode == GameMode::HIDE_AND_SEEK) {
                m_display.showPlantPromptHS("", m_webServer.getConfig().maskPassword);
            } else {
                m_display.showPlantPrompt("", m_webServer.getConfig().maskPassword);
            }
            break;

        case GameState::S4_PLANTED:
            if (m_remainingTimeMs == 0) {
                // Primeira transição para plantada: inicia temporizador com valor configurado
                uint32_t bombSec = m_webServer.getConfig().bombTimeSeconds;
                if (bombSec < 5) bombSec = 45;
                m_totalBombTimeMs = bombSec * 1000;
                m_remainingTimeMs = m_totalBombTimeMs;
                m_plantedTimestampMs = currentMillis;
                m_defuse = {0, 0, false, false}; // Garante defuse limpo no plantio da bomba
                // Bip sonoro de confirmação de armação (2.000 Hz, 500 ms)
                m_buzzer.triggerSingleBeep(C4Config::BUZZER_FREQ_CONFIRM, 500, currentMillis);
            }
            m_keypad.clearBuffer();
            updatePlantedCadence(m_remainingTimeMs);
            if (m_mode == GameMode::HIDE_AND_SEEK && m_plantedTeam == Team::BLUE) {
                m_led.setPattern(LedPattern::BLINK_BLUE);
            } else {
                m_led.setPattern(LedPattern::BLINK_RED);
            }
            m_display.showCountdown(m_remainingTimeMs);
            break;

        case GameState::S5_DEFUSING:
            updatePlantedCadence(m_remainingTimeMs);
            if (m_mode == GameMode::HIDE_AND_SEEK && m_plantedTeam == Team::BLUE) {
                m_led.setPattern(LedPattern::BLINK_BLUE);
            } else {
                m_led.setPattern(LedPattern::BLINK_RED);
            }
            m_defuse.isReleasedInTolerance = false;
            m_defuse.buttonReleasedTimestamp = 0;
            m_defuse.isCompleted = false;
            if (m_activeDefuseTargetMs < 1000) {
                m_activeDefuseTargetMs = 10000;
            }
            if (m_defuse.accumulatedTimeMs >= m_activeDefuseTargetMs) {
                m_defuse.accumulatedTimeMs = 0;
            }
            m_display.showDefusing(m_remainingTimeMs, m_defuse.accumulatedTimeMs, m_activeDefuseTargetMs, false);
            break;

        case GameState::S6_DEFUSED:
            m_buzzer.setCadence(BeepCadence::SUCCESS_DOUBLE);
            if (m_mode == GameMode::HIDE_AND_SEEK) {
                if (m_plantedTeam == Team::BLUE) {
                    // Time Azul plantou e Time Vermelho desarmou -> Vitória Vermelho
                    m_led.setPattern(LedPattern::SOLID_RED);
                    m_display.showVictoryRed();
                } else {
                    // Time Vermelho plantou e Time Azul desarmou -> Vitória Azul
                    m_led.setPattern(LedPattern::SOLID_BLUE);
                    m_display.showVictoryBlue();
                }
            } else {
                // Vitória CT (AC-11)
                m_led.setPattern(LedPattern::SOLID_BLUE);
                m_display.showDefusedCT();
            }
            m_keypad.clearBuffer();
            m_defuse = {0, 0, false, false}; // Previne vazamento para a próxima rodada
            break;

        case GameState::S7_EXPLODED_TR:
            m_buzzer.setCadence(BeepCadence::CONTINUOUS);
            if (m_mode == GameMode::HIDE_AND_SEEK) {
                if (m_plantedTeam == Team::BLUE) {
                    // Time Azul plantou e protegeu até explodir -> Vitória Azul
                    m_led.setPattern(LedPattern::SOLID_BLUE);
                    m_display.showVictoryBlue();
                } else {
                    // Time Vermelho plantou e protegeu até explodir -> Vitória Vermelho
                    m_led.setPattern(LedPattern::SOLID_RED);
                    m_display.showVictoryRed();
                }
            } else {
                // Vitória TR (AC-12)
                m_led.setPattern(LedPattern::SOLID_YELLOW);
                m_display.showExplodedTR();
            }
            m_keypad.clearBuffer();
            m_defuse = {0, 0, false, false}; // Previne vazamento para a próxima rodada
            break;

        case GameState::S_ERROR:
        default:
            m_buzzer.setCadence(BeepCadence::ERROR_TONE);
            m_led.setPattern(LedPattern::ERROR_FLASH);
            m_display.showError("ERRO SISTEMA");
            break;
    }
}

void C4GameFSM::updatePlantedCadence(uint32_t remainingMs) {
    // Escalonamento em 3 terços de tempo (AC-04):
    // 1º terço: > 2/3 -> Lento (1 bip/s a 4.200 Hz)
    // 2º terço: 1/3 < t <= 2/3 -> Médio (2 bips/s a 4.200 Hz)
    // 3º terço: <= 1/3 -> Rápido (4 bips/s a 4.200 Hz)
    if (remainingMs > (m_totalBombTimeMs * 2) / 3) {
        m_buzzer.setCadence(BeepCadence::SLOW_1HZ);
    } else if (remainingMs > m_totalBombTimeMs / 3) {
        m_buzzer.setCadence(BeepCadence::MEDIUM_2HZ);
    } else {
        m_buzzer.setCadence(BeepCadence::FAST_4HZ);
    }
}

void C4GameFSM::update(uint32_t currentMillis) {
    if (m_lastTickMs == 0) {
        m_lastTickMs = currentMillis;
    }
    uint32_t deltaMs = currentMillis - m_lastTickMs;
    m_lastTickMs = currentMillis;

    // Atualização dos drivers periféricos
    m_button.update(currentMillis);
    m_buzzer.update(currentMillis);
    m_led.update(currentMillis, m_buzzer.isCurrentlyBeeping());
    m_display.update(currentMillis);

    // Execução da lógica por estado
    switch (m_state) {
        case GameState::S1_INIT:
            handleStateInit(currentMillis);
            break;
        case GameState::S2_MENU:
            handleStateMenu(currentMillis);
            break;
        case GameState::S3_READY:
            handleStateReady(currentMillis);
            break;
        case GameState::S4_PLANTED:
            handleStatePlanted(currentMillis, deltaMs);
            break;
        case GameState::S5_DEFUSING:
            handleStateDefusing(currentMillis, deltaMs);
            break;
        case GameState::S6_DEFUSED:
            handleStateDefused(currentMillis);
            break;
        case GameState::S7_EXPLODED_TR:
            handleStateExploded(currentMillis);
            break;
        case GameState::S_ERROR:
        default:
            char k = m_keypad.pollKey();
            if (k == 'A') {
                transitionTo(GameState::S2_MENU, currentMillis);
            }
            break;
    }
}

void C4GameFSM::handleStateInit(uint32_t currentMillis) {
    transitionTo(GameState::S2_MENU, currentMillis);
}

void C4GameFSM::handleStateMenu(uint32_t currentMillis) {
    char key = m_keypad.pollKey();
    if (key != '\0') {
        m_buzzer.triggerKeyBeep(currentMillis);

        if (key == 'C') {
            m_mode = GameMode::CS_GO;
            transitionTo(GameState::S3_READY, currentMillis);
        } else if (key == 'B') {
            m_mode = GameMode::HIDE_AND_SEEK;
            transitionTo(GameState::S3_READY, currentMillis);
        }
    }
}

void C4GameFSM::handleStateReady(uint32_t currentMillis) {
    char key = m_keypad.pollKey();
    if (key != '\0') {
        m_buzzer.triggerKeyBeep(currentMillis);
        KeyAction action = m_keypad.handleInput(key);

        if (action == KeyAction::DIGIT_ADDED || action == KeyAction::CLEARED) {
            if (m_mode == GameMode::HIDE_AND_SEEK) {
                m_display.showPlantPromptHS(m_keypad.getBuffer(), m_webServer.getConfig().maskPassword);
            } else {
                m_display.showPlantPrompt(m_keypad.getBuffer(), m_webServer.getConfig().maskPassword);
            }
        } else if (action == KeyAction::SUBMIT) {
            if (m_mode == GameMode::CS_GO) {
                // Validação com tecla '#' no modo CS:GO (AC-14)
                if (strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordTR) == 0) {
                    m_keypad.clearBuffer();
                    m_remainingTimeMs = 0; // Dispara inicialização de contagem em transitionTo
                    transitionTo(GameState::S4_PLANTED, currentMillis);
                } else {
                    m_buzzer.triggerSingleBeep(C4Config::BUZZER_FREQ_ERROR, 1000, currentMillis);
                    m_display.showError("SENHA INVALIDA");
                    m_keypad.clearBuffer();
                }
            } else {
                // Modo Hide and Seek: Time Azul ou Time Vermelho armam a C4 (AC-18)
                if (strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordBluePlant) == 0) {
                    m_plantedTeam = Team::BLUE;
                    m_keypad.clearBuffer();
                    m_remainingTimeMs = 0;
                    transitionTo(GameState::S4_PLANTED, currentMillis);
                } else if (strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordRedPlant) == 0) {
                    m_plantedTeam = Team::RED;
                    m_keypad.clearBuffer();
                    m_remainingTimeMs = 0;
                    transitionTo(GameState::S4_PLANTED, currentMillis);
                } else {
                    m_buzzer.triggerSingleBeep(C4Config::BUZZER_FREQ_ERROR, 1000, currentMillis);
                    m_display.showError("SENHA INVALIDA");
                    m_keypad.clearBuffer();
                }
            }
        }
    }
}

void C4GameFSM::handleStatePlanted(uint32_t currentMillis, uint32_t deltaMs) {
    // 1. Contagem regressiva da bomba com precedência máxima
    if (deltaMs >= m_remainingTimeMs) {
        m_remainingTimeMs = 0;
        transitionTo(GameState::S7_EXPLODED_TR, currentMillis);
        return;
    } else {
        m_remainingTimeMs -= deltaMs;
    }

    updatePlantedCadence(m_remainingTimeMs);

    // 2. Leitura de entrada de senha de defuse
    char key = m_keypad.pollKey();
    if (key != '\0') {
        m_buzzer.triggerKeyBeep(currentMillis);
        KeyAction action = m_keypad.handleInput(key);

        if (action == KeyAction::DIGIT_ADDED || action == KeyAction::CLEARED) {
            if (m_mode == GameMode::HIDE_AND_SEEK) {
                bool defusingIsRed = (m_plantedTeam == Team::BLUE);
                m_display.showDefusePromptHS(m_keypad.getBuffer(), m_webServer.getConfig().maskPassword, defusingIsRed);
            } else {
                m_display.showDefusePrompt(m_keypad.getBuffer(), m_webServer.getConfig().maskPassword);
            }
        } else if (action == KeyAction::SUBMIT) {
            if (m_mode == GameMode::CS_GO) {
                if (strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordCT) == 0) {
                    // Senha de desarme normal
                    uint32_t defSec = m_webServer.getConfig().defuseTimeSeconds;
                    if (defSec < 1) defSec = 10;
                    m_activeDefuseTargetMs = defSec * 1000;
                    m_keypad.clearBuffer();
                    transitionTo(GameState::S5_DEFUSING, currentMillis);
                    return;
                } else if (strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordKit) == 0) {
                    // Senha de desarme com Kit C4
                    uint32_t kitSec = m_webServer.getConfig().defuseKitSeconds;
                    if (kitSec < 1) kitSec = 5;
                    m_activeDefuseTargetMs = kitSec * 1000;
                    m_keypad.clearBuffer();
                    transitionTo(GameState::S5_DEFUSING, currentMillis);
                    return;
                } else {
                    m_buzzer.triggerSingleBeep(C4Config::BUZZER_FREQ_ERROR, 1000, currentMillis);
                    m_display.showError("SENHA CT ERRADA");
                    m_keypad.clearBuffer();
                }
            } else {
                // Modo Hide and Seek: Desarme cruzado (AC-19)
                // Se Azul armou, apenas Vermelho pode desarmar (passwordRedDefuse).
                // Se Vermelho armou, apenas Azul pode desarmar (passwordBlueDefuse).
                bool defuseAccepted = false;
                if (m_plantedTeam == Team::BLUE && strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordRedDefuse) == 0) {
                    defuseAccepted = true;
                } else if (m_plantedTeam == Team::RED && strcmp(m_keypad.getBuffer(), m_webServer.getConfig().passwordBlueDefuse) == 0) {
                    defuseAccepted = true;
                }

                if (defuseAccepted) {
                    uint32_t defSec = m_webServer.getConfig().defuseTimeSeconds;
                    if (defSec < 1) defSec = 10;
                    m_activeDefuseTargetMs = defSec * 1000;
                    m_keypad.clearBuffer();
                    transitionTo(GameState::S5_DEFUSING, currentMillis);
                    return;
                } else {
                    m_buzzer.triggerSingleBeep(C4Config::BUZZER_FREQ_ERROR, 1000, currentMillis);
                    m_display.showError("SENHA ERRADA");
                    m_keypad.clearBuffer();
                }
            }
        }
    } else {
        if (m_keypad.getLength() == 0) {
            m_display.showCountdown(m_remainingTimeMs);
        }
    }
}

void C4GameFSM::handleStateDefusing(uint32_t currentMillis, uint32_t deltaMs) {
    // 1. Precedência absoluta da explosão sobre o defuse (AC-10)
    if (deltaMs >= m_remainingTimeMs) {
        m_remainingTimeMs = 0;
        transitionTo(GameState::S7_EXPLODED_TR, currentMillis);
        return;
    } else {
        m_remainingTimeMs -= deltaMs;
    }

    updatePlantedCadence(m_remainingTimeMs);

    // 2. Acúmulo de progresso com botão pressionado (AC-07, AC-08, AC-09)
    if (m_button.isPressed()) {
        if (m_defuse.isReleasedInTolerance) {
            m_defuse.isReleasedInTolerance = false;
        }

        m_defuse.accumulatedTimeMs += deltaMs;

        if (m_defuse.accumulatedTimeMs >= m_activeDefuseTargetMs) {
            m_defuse.isCompleted = true;
            transitionTo(GameState::S6_DEFUSED, currentMillis);
            return;
        }

        m_display.showDefusing(m_remainingTimeMs, m_defuse.accumulatedTimeMs, m_activeDefuseTargetMs, false);

    } else {
        // Botão solto durante o defuse
        uint32_t toleranceMs = m_webServer.getConfig().defuseToleranceSeconds * 1000;
        if (m_defuse.accumulatedTimeMs > 0 && toleranceMs == 0) {
            // Tolerância zero: zera o progresso imediatamente e retorna para S4_PLANTED (AC-08, AC-09)
            m_defuse = {0, 0, false, false};
            transitionTo(GameState::S4_PLANTED, currentMillis);
            return;
        }

        // Janela de espera para pressionar/retomar botão
        uint32_t effectiveWaitMs = toleranceMs;
        if (m_defuse.accumulatedTimeMs == 0 && effectiveWaitMs < 3000) {
            effectiveWaitMs = 3000; // Janela inicial de 3s para pressionar botão pela 1ª vez
        }

        if (!m_defuse.isReleasedInTolerance) {
            m_defuse.isReleasedInTolerance = true;
            m_defuse.buttonReleasedTimestamp = currentMillis;
        } else {
            if (currentMillis - m_defuse.buttonReleasedTimestamp >= effectiveWaitMs) {
                // Tolerância expirou: zera o progresso para 0 e retorna para S4_PLANTED (AC-09)
                m_defuse = {0, 0, false, false};
                transitionTo(GameState::S4_PLANTED, currentMillis);
                return;
            }
        }

        m_display.showDefusing(m_remainingTimeMs, m_defuse.accumulatedTimeMs, m_activeDefuseTargetMs, true);
    }
}

void C4GameFSM::handleStateDefused(uint32_t currentMillis) {
    char key = m_keypad.pollKey();
    if (key == 'A') {
        transitionTo(GameState::S2_MENU, currentMillis);
    }
}

void C4GameFSM::handleStateExploded(uint32_t currentMillis) {
    char key = m_keypad.pollKey();
    if (key == 'A') {
        transitionTo(GameState::S2_MENU, currentMillis);
    }
}

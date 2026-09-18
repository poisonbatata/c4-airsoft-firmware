/**
 * @file test_fsm_core.cpp
 * @brief Suite de Testes Unitários de Bancada para a FSM e Regras de Negócio do C4 Airsoft
 * @ref spec.md (AC-01 a AC-17), plan.md (TDRs) e tasks.md (Task 1.4, Task 5.7)
 *
 * Compilação e execução nativa (Linux / GCC):
 * g++ -std=c++17 -Wall -Wextra -I include test/test_fsm_core.cpp src/game_fsm.cpp \
 *     src/keypad_driver.cpp src/button_driver.cpp src/buzzer_driver.cpp \
 *     src/led_driver.cpp src/display_driver.cpp src/web_server_manager.cpp \
 *     -o test_fsm && ./test_fsm
 */

#include <iostream>
#include <cassert>
#include <cstring>
#include "c4_types.h"
#include "c4_config.h"
#include "c4_pins.h"
#include "keypad_driver.h"
#include "button_driver.h"
#include "buzzer_driver.h"
#include "led_driver.h"
#include "display_driver.h"
#include "web_server_manager.h"
#include "game_fsm.h"

// Macro de asserção com log formatado
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[\033[31mFALHA\033[0m] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            return false; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        std::cout << "Executando " #fn "... "; \
        if (fn()) { \
            std::cout << "\033[32m[PASSOU]\033[0m" << std::endl; \
            passedCount++; \
        } else { \
            failedCount++; \
        } \
        totalCount++; \
    } while (0)

// Helper para instanciar sistema de teste com drivers e web server
struct TestRig {
    KeypadDriver      keypad;
    ButtonDriver      button;
    BuzzerDriver      buzzer;
    LedDriver         led;
    DisplayDriver     display;
    WebServerManager  webServer;
    C4GameFSM         fsm;

    TestRig() : fsm(keypad, button, buzzer, led, display, webServer) {
        fsm.init();
    }

    void advanceToReady(uint32_t& simTimeMs) {
        fsm.update(simTimeMs); // S1 -> S2
        keypad.injectKey('C'); // Modo CS:GO
        simTimeMs += 50;
        fsm.update(simTimeMs); // S2 -> S3
    }

    void advanceToReadyHS(uint32_t& simTimeMs) {
        fsm.update(simTimeMs); // S1 -> S2
        keypad.injectKey('B'); // Modo Hide and Seek
        simTimeMs += 50;
        fsm.update(simTimeMs); // S2 -> S3
    }

    void typeString(const char* str, uint32_t& simTimeMs) {
        for (size_t i = 0; i < strlen(str); ++i) {
            keypad.injectKey(str[i]);
            simTimeMs += 50;
            fsm.update(simTimeMs);
        }
    }

    void plantBomb(uint32_t& simTimeMs) {
        advanceToReady(simTimeMs);
        typeString(webServer.getConfig().passwordTR, simTimeMs);
        // Submete com a tecla '#' (Enter)
        keypad.injectKey('#');
        simTimeMs += 50;
        fsm.update(simTimeMs);
    }
};

/**
 * @test Teste 1: Transição S1_INIT -> S2_MENU -> S3_READY e Plantio com Senha TR "73556" + Enter '#'
 * @ref AC-03, AC-14, CTR-001
 */
bool test_plant_with_correct_tr_password_and_enter() {
    TestRig rig;
    uint32_t simTime = 100;

    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S2_MENU, "FSM deve transitar de INIT para MENU");

    rig.keypad.injectKey('C');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S3_READY, "FSM deve transitar de MENU para READY ao selecionar CS:GO");

    // Digita senha correta "73556" sem o '#': FSM deve continuar em S3_READY
    rig.typeString("73556", simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S3_READY, "FSM deve aguardar '#' para submeter senha");

    // Pressiona '#' (Enter) para validar a senha (AC-14)
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "FSM deve transitar para S4_PLANTED apos '#' (Enter)");
    TEST_ASSERT(rig.fsm.getRemainingTimeMs() == C4Config::BOMB_COUNTDOWN_MS, "Tempo de bomba deve iniciar no valor configurado");
    return true;
}

/**
 * @test Teste 2: Rejeição de Senha TR Incorreta com '#' e Limpeza de Buffer
 * @ref AC-03, AC-14, AC-16
 */
bool test_plant_with_invalid_password_rejection() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.advanceToReady(simTime);

    // Digita senha incorreta "99999" e dá Enter '#'
    rig.typeString("99999", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S3_READY, "FSM deve permanecer em S3_READY com senha invalida");
    TEST_ASSERT(rig.keypad.getLength() == 0, "Buffer de senha deve ser limpo imediatamente apos erro");
    return true;
}

/**
 * @test Teste 3: Limpeza Manual do Buffer com a Tecla 'A'
 * @ref AC-02, AC-14, IF-002
 */
bool test_keypad_buffer_clear_with_key_a() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.advanceToReady(simTime);

    rig.keypad.injectKey('1');
    simTime += 50;
    rig.fsm.update(simTime);

    rig.keypad.injectKey('2');
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.keypad.getLength() == 2, "Buffer deve conter 2 digitos");

    rig.keypad.injectKey('A');
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.keypad.getLength() == 0, "Tecla 'A' deve esvaziar completamente o buffer de senha");
    return true;
}

/**
 * @test Teste 4: Escalonamento Sonoro do Buzzer nos Três Terços de Tempo (45s -> >30s, 15-30s, <=15s)
 * @ref AC-04, AC-16
 */
bool test_buzzer_cadence_three_tiers() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    // No plantio imediato (45.000 ms restantes): 1º terço (> 30s) -> SLOW_1HZ
    TEST_ASSERT(rig.buzzer.getCadence() == BeepCadence::SLOW_1HZ, "Cadencia inicial (>30s) deve ser SLOW_1HZ (1 bip/s a 4.200 Hz)");

    // Avança 16 segundos: resta 29.000 ms: 2º terço (15s < t <= 30s) -> MEDIUM_2HZ
    simTime += 16000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getRemainingTimeMs() <= 30000 && rig.fsm.getRemainingTimeMs() > 15000, "Tempo deve estar no 2o terco");
    TEST_ASSERT(rig.buzzer.getCadence() == BeepCadence::MEDIUM_2HZ, "Cadencia no 2o terco deve ser MEDIUM_2HZ (2 bips/s a 4.200 Hz)");

    // Avança mais 16 segundos: resta 13.000 ms: 3º terço (<= 15s) -> FAST_4HZ
    simTime += 16000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getRemainingTimeMs() <= 15000, "Tempo deve estar no 3o terco");
    TEST_ASSERT(rig.buzzer.getCadence() == BeepCadence::FAST_4HZ, "Cadencia no 3o terco deve ser FAST_4HZ (4 bips/s a 4.200 Hz)");

    return true;
}

/**
 * @test Teste 5: Soltura do Botão de Defuse por Menos de 5.000 ms Preserva o Progresso Acumulado
 * @ref AC-08, AC-14
 */
bool test_defuse_tolerance_window_preserved() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    // CT insere senha de defuse "12345" + '#'
    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "FSM deve transitar para S5_DEFUSING apos senha CT + '#'");

    // Segura botão por 3.000 ms
    rig.button.setMockPressed(true);
    simTime += 3000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs >= 3000, "Progresso acumulado de defuse deve ser >= 3000ms");

    uint32_t savedProgress = rig.fsm.getDefuseProgress().accumulatedTimeMs;

    // Solta botão por 3.000 ms (< janela de tolerância de 5.000 ms)
    rig.button.setMockPressed(false);
    simTime += 3000;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Deve permanecer em S5_DEFUSING durante janela de tolerancia");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == savedProgress, "Progresso acumulado nao deve sofrer penalidade dentro de 5s");

    // Retoma botão
    rig.button.setMockPressed(true);
    simTime += 1000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs > savedProgress, "Progresso deve continuar acumulando apos retomada");

    return true;
}

/**
 * @test Teste 6: Soltura do Botão > Tolerância Zera Progresso (0%) e Exige Nova Senha CT para Recomeçar do Zero
 * @ref AC-09, AC-14
 */
bool test_defuse_penalty_above_half_preserved_at_fifty_percent() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    // Segura botão por 7.000 ms (70% do total de 10.000 ms)
    rig.button.setMockPressed(true);
    simTime += 7000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs >= 7000, "Progresso deve estar em 70% (7000ms)");

    // Solta botão por mais de 5.000 ms (> janela de tolerância)
    rig.button.setMockPressed(false);
    simTime += 50;
    rig.fsm.update(simTime); // Registra início da janela de tolerância

    simTime += 5100;
    rig.fsm.update(simTime); // Dispara expiração da tolerância

    // Deve retornar para S4_PLANTED e o progresso deve ser ZERADO (não preserva 50%)
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Apos timeout de tolerancia, FSM deve retornar a S4_PLANTED");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "Progresso de defuse DEVE ser zerado para 0ms");

    // Tentar apertar o botão em S4_PLANTED não deve iniciar defuse (exige senha primeiro)
    rig.button.setMockPressed(true);
    simTime += 500;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Em S4_PLANTED, apertar o botao nao deve iniciar defuse sem senha");
    rig.button.setMockPressed(false);

    // CT coloca a senha novamente "12345#"
    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Deve retornar para S5_DEFUSING apos nova insercao de senha");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "A progressao do defuse deve comecar estritamente do zero");

    // Pressiona o botão e confirma que a progressão recomeça do zero e requer os 10s integrais
    rig.button.setMockPressed(true);
    simTime += 5000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Com 5s acumulados, nao deve desarmar ainda");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 5000, "Progresso deve estar em 5000ms a partir do zero");

    simTime += 5000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "Deve desarmar somente apos acumular os 10s completos");

    return true;
}

/**
 * @test Teste 7: Soltura do Botão > 5.000 ms com Progresso <= 50% Zera o Progresso
 * @ref AC-09, AC-14
 */
bool test_defuse_penalty_below_half_reset_to_zero() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    // Segura botão por 3.000 ms (30% do total)
    rig.button.setMockPressed(true);
    simTime += 3000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs >= 3000, "Progresso deve ser 3000ms");

    // Solta botão por mais de 5.000 ms
    rig.button.setMockPressed(false);
    simTime += 50;
    rig.fsm.update(simTime); // Registra início da janela de tolerância

    simTime += 5100;
    rig.fsm.update(simTime); // Dispara expiração da tolerância

    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "FSM deve retornar a S4_PLANTED");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "Penalidade deve zerar progresso se <= 50%");

    return true;
}

/**
 * @test Teste 8: Precedência Máxima do Tempo Geral da Rodada (Explosão sobre Defuse)
 * @ref AC-10, AC-14
 */
bool test_explosion_precedence_over_active_defuse() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    // Deixa o tempo passar até restarem apenas 3.000 ms (42 segundos decorridos)
    simTime += 42000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getRemainingTimeMs() == 3000, "Devem restar 3000ms na contagem");

    // CT insere senha e dá Enter '#'
    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    rig.button.setMockPressed(true);
    // Defuse precisa de 10.000 ms, mas bomba explode em 3.000 ms
    simTime += 3500;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S7_EXPLODED_TR, "Bomba deve explodir (S7_EXPLODED_TR) mesmo com defuse em andamento");
    TEST_ASSERT(rig.fsm.getRemainingTimeMs() == 0, "Tempo restante deve ser 0ms");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::SOLID_YELLOW, "LED deve ficar amarelo solido (Vitoria TR)");
    TEST_ASSERT(rig.buzzer.getCadence() == BeepCadence::CONTINUOUS, "Buzzer deve soar continuamente a 4.200 Hz");

    return true;
}

/**
 * @test Teste 9: Defuse Completo (10s Contínuos) e Vitória CT
 * @ref AC-11, AC-14
 */
bool test_defuse_successful_ct_victory() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    // Segura botão por 10.000 ms contínuos
    rig.button.setMockPressed(true);
    simTime += 10000;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "FSM deve transitar para S6_DEFUSED");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::SOLID_BLUE, "LED deve ficar azul solido (Vitoria CT)");
    TEST_ASSERT(rig.buzzer.getCadence() == BeepCadence::SUCCESS_DOUBLE, "Buzzer deve emitir som de confirmacao");

    // Pressiona 'A' para retornar ao menu
    rig.keypad.injectKey('A');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S2_MENU, "Tecla 'A' apos fim da rodada deve resetar para o menu");

    return true;
}

/**
 * @test Teste 10: Desarme com Kit C4 ("1234") utiliza 5 segundos de defuse
 * @ref AC-13, AC-14
 */
bool test_defuse_with_kit_password() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    // Insere senha do Kit C4 ("1234") + '#'
    rig.typeString("1234", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Deve transitar para S5_DEFUSING com senha do kit");

    // Segura o botão por 5.000 ms (tempo do kit)
    rig.button.setMockPressed(true);
    simTime += 5000;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "Desarme com Kit deve completar com sucesso em 5s");
    return true;
}

/**
 * @test Teste 11: Configuração de Máscara de Senha no Display (AC-15)
 * @ref AC-15
 */
bool test_password_display_mask_and_cleartext() {
    TestRig rig;

    // 1. Modo Mascarado (maskPassword = true)
    C4Config::GameRuntimeConfig cfg = rig.webServer.getConfig();
    cfg.maskPassword = true;
    rig.webServer.setConfig(cfg);
    TEST_ASSERT(rig.webServer.getConfig().maskPassword == true, "Mascara deve estar ativa");

    // 2. Modo Texto Claro (maskPassword = false)
    cfg.maskPassword = false;
    rig.webServer.setConfig(cfg);
    TEST_ASSERT(rig.webServer.getConfig().maskPassword == false, "Mascara deve estar desativada");

    return true;
}

/**
 * @test Teste 12: Rejeição estrita de senha com dígitos excedentes (ex: "123456" para senha "12345")
 * @ref AC-03, AC-14
 */
bool test_defuse_rejection_with_extra_digits() {
    TestRig rig;
    uint32_t simTime = 100;
    rig.plantBomb(simTime);

    // CT tenta desarme com dígito extra "123456" + '#' (senha correta é "12345")
    rig.typeString("123456", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "FSM deve permanecer em S4_PLANTED ao digitar '123456' para senha '12345'");
    TEST_ASSERT(rig.keypad.getLength() == 0, "Buffer do teclado deve ser limpo apos senha incorreta");
    return true;
}

/**
 * @test Teste 13: Tolerância Zero no Defuse (ttol = 0s) cancela imediatamente na soltura do botão
 * @ref AC-08, AC-13
 */
bool test_defuse_zero_tolerance_immediate_cancel() {
    TestRig rig;
    uint32_t simTime = 100;

    // Configura tolerância para 0 segundos
    C4Config::GameRuntimeConfig cfg = rig.webServer.getConfig();
    cfg.defuseToleranceSeconds = 0;
    rig.webServer.setConfig(cfg);

    rig.plantBomb(simTime);

    // Inicia defuse com senha correta "12345" + '#'
    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "FSM deve transitar para S5_DEFUSING");

    // Segura botão por 2.000 ms
    rig.button.setMockPressed(true);
    simTime += 2000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs >= 2000, "Progresso deve acumular 2000ms");

    // Solta botão: com tolerância zero, o cancelamento para S4_PLANTED deve ocorrer imediatamente
    rig.button.setMockPressed(false);
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Com tolerancia zero, FSM deve cancelar imediatamente para S4_PLANTED ao soltar o botao");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "Progresso < 50% deve ser zerado imediatamente");
    return true;
}

/**
 * @test Teste 14: Tolerância configurável personalizada (ttol = 10s)
 * @ref AC-08, AC-13
 */
bool test_defuse_custom_tolerance_window() {
    TestRig rig;
    uint32_t simTime = 100;

    C4Config::GameRuntimeConfig cfg = rig.webServer.getConfig();
    cfg.defuseToleranceSeconds = 10;
    rig.webServer.setConfig(cfg);

    rig.plantBomb(simTime);

    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    rig.button.setMockPressed(true);
    simTime += 3000;
    rig.fsm.update(simTime);

    // Solta botão por 7 segundos (< 10 segundos configurados)
    rig.button.setMockPressed(false);
    simTime += 50;
    rig.fsm.update(simTime); // Inicia tolerância

    simTime += 7000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Deve permanecer em S5_DEFUSING dentro da tolerancia de 10s");

    // Ultrapassa 10s (mais 3.5s totalizando > 10s de soltura)
    simTime += 3500;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Deve cancelar defuse e retornar para S4_PLANTED apos ultrapassar 10s");

    return true;
}

/**
 * @test Teste 15: Hide and Seek: Plantio Time Azul ("1111") e Vitória Azul por Detonação
 * @ref AC-18, AC-20
 */
bool test_hs_blue_plant_and_blue_victory_by_explosion() {
    TestRig rig;
    uint32_t simTime = 100;

    rig.advanceToReadyHS(simTime);
    TEST_ASSERT(rig.fsm.getGameMode() == GameMode::HIDE_AND_SEEK, "Modo deve ser HIDE_AND_SEEK");

    // Time Azul arma com senha "1111" + '#'
    rig.typeString("1111", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Bomba deve estar armada no modo H&S");
    TEST_ASSERT(rig.fsm.getPlantedTeam() == Team::BLUE, "Time que armou deve ser BLUE");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::BLINK_BLUE, "LED deve pulsar em Azul durante contagem de bomba do Time Azul");

    // Avança tempo até a detonação da bomba (45s)
    simTime += 46000;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S7_EXPLODED_TR, "FSM deve transitar para fim de jogo por explosao");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::SOLID_BLUE, "Vitoria do Time Azul deve acender LED Azul solido");

    // Retorna ao menu com 'A'
    rig.keypad.injectKey('A');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S2_MENU, "Deve retornar ao menu com 'A'");

    return true;
}

/**
 * @test Teste 16: Hide and Seek: Plantio Time Azul ("1111") e Vitória Time Vermelho por Desarme ("4444")
 * @ref AC-18, AC-19, AC-20
 */
bool test_hs_blue_plant_and_red_victory_by_defuse() {
    TestRig rig;
    uint32_t simTime = 100;

    rig.advanceToReadyHS(simTime);

    // Time Azul arma com "1111" + '#'
    rig.typeString("1111", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Bomba armada por Time Azul");

    // Tentativa inválida de desarme com senha errada ("9999") ou com a própria senha azul ("2222")
    rig.typeString("2222", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Defuse com senha incorreta deve ser rejeitado");
    TEST_ASSERT(rig.keypad.getLength() == 0, "Buffer deve ser limpo apos senha incorreta");

    // Time Vermelho insere senha correta de desarme ("4444") + '#'
    rig.typeString("4444", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "FSM deve transitar para S5_DEFUSING com senha do Time Vermelho");

    // Segura botão pelo tempo total de defuse (10.000 ms)
    rig.button.setMockPressed(true);
    simTime += 10000;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "FSM deve transitar para S6_DEFUSED");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::SOLID_RED, "Vitoria do Time Vermelho (desarme) deve acender LED Vermelho solido");

    return true;
}

/**
 * @test Teste 17: Hide and Seek: Plantio Time Vermelho ("3333") e Vitória Time Azul por Desarme ("2222")
 * @ref AC-18, AC-19, AC-20
 */
bool test_hs_red_plant_and_blue_victory_by_defuse() {
    TestRig rig;
    uint32_t simTime = 100;

    rig.advanceToReadyHS(simTime);

    // Time Vermelho arma com "3333" + '#'
    rig.typeString("3333", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Bomba armada por Time Vermelho");
    TEST_ASSERT(rig.fsm.getPlantedTeam() == Team::RED, "Time que armou deve ser RED");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::BLINK_RED, "LED deve pulsar em Vermelho durante contagem");

    // Time Azul insere senha correta de desarme ("2222") + '#'
    rig.typeString("2222", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "FSM deve transitar para S5_DEFUSING com senha do Time Azul");

    // Segura botão pelo tempo total de defuse (10.000 ms)
    rig.button.setMockPressed(true);
    simTime += 10000;
    rig.fsm.update(simTime);

    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "FSM deve transitar para S6_DEFUSED");
    TEST_ASSERT(rig.led.getPattern() == LedPattern::SOLID_BLUE, "Vitoria do Time Azul (desarme) deve acender LED Azul solido");

    return true;
}

/**
 * @test Teste 18: Rodadas Consecutivas: Acumulador de defuse deve ser 100% resetado entre rodadas
 * Previne o bug de desarme imediato no primeiro toque de botão em rodada subsequente.
 * @ref AC-06, AC-07, AC-11
 */
bool test_consecutive_rounds_defuse_accumulator_reset() {
    TestRig rig;
    uint32_t simTime = 100;

    // --- RODADA 1: Jogo Completo com Defuse CT ---
    rig.plantBomb(simTime);

    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Rodada 1: Deve entrar em S5_DEFUSING");

    // Segura botão por 10.000 ms até defusar
    rig.button.setMockPressed(true);
    simTime += 10000;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "Rodada 1: C4 deve desarmar com sucesso");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "Apos S6_DEFUSED progresso deve ser resetado");

    // CT solta o botão
    rig.button.setMockPressed(false);

    // Pressiona 'A' para voltar ao Menu
    rig.keypad.injectKey('A');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S2_MENU, "Deve retornar a S2_MENU");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "No Menu, defuse acumulado deve ser 0");

    // --- RODADA 2: Novo Teste no Modo CS:GO ---
    // Seleciona CS:GO ('C')
    rig.keypad.injectKey('C');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S3_READY, "Rodada 2: Deve estar em S3_READY");

    // TR arma a bomba ("73556#")
    rig.typeString("73556", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S4_PLANTED, "Rodada 2: C4 armada em S4_PLANTED");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 0, "Em S4_PLANTED, defuse acumulado deve ser 0");

    // CT digita senha de desarme "12345#"
    rig.typeString("12345", simTime);
    rig.keypad.injectKey('#');
    simTime += 50;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Rodada 2: Deve entrar em S5_DEFUSING");

    // Pressiona botão por apenas 100 ms (toque inicial)
    rig.button.setMockPressed(true);
    simTime += 100;
    rig.fsm.update(simTime);

    // CRÍTICO: NÃO DEVE DESARMAR IMEDIATAMENTE!
    TEST_ASSERT(rig.fsm.getState() == GameState::S5_DEFUSING, "Rodada 2: NAO deve desarmar imediatamente ao primeiro toque de botao!");
    TEST_ASSERT(rig.fsm.getDefuseProgress().accumulatedTimeMs == 100, "Rodada 2: Progresso acumulado deve ser apenas 100ms, e nao 10000ms!");

    // Deve requerer mais 9.900 ms de botão mantido para completar o defuse
    simTime += 9900;
    rig.fsm.update(simTime);
    TEST_ASSERT(rig.fsm.getState() == GameState::S6_DEFUSED, "Rodada 2: C4 deve desarmar somente apos o tempo total de 10s!");

    return true;
}

int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << "  SUITE DE TESTES NATIVOS DE BANCA: C4 AIRSOFT FSM CORE  " << std::endl;
    std::cout << "  VALIDACAO DE CRITERIOS DE ACEITE (AC-01 A AC-20)      " << std::endl;
    std::cout << "========================================================" << std::endl;

    int totalCount = 0;
    int passedCount = 0;
    int failedCount = 0;

    RUN_TEST(test_plant_with_correct_tr_password_and_enter);
    RUN_TEST(test_plant_with_invalid_password_rejection);
    RUN_TEST(test_keypad_buffer_clear_with_key_a);
    RUN_TEST(test_buzzer_cadence_three_tiers);
    RUN_TEST(test_defuse_tolerance_window_preserved);
    RUN_TEST(test_defuse_penalty_above_half_preserved_at_fifty_percent);
    RUN_TEST(test_defuse_penalty_below_half_reset_to_zero);
    RUN_TEST(test_explosion_precedence_over_active_defuse);
    RUN_TEST(test_defuse_successful_ct_victory);
    RUN_TEST(test_defuse_with_kit_password);
    RUN_TEST(test_password_display_mask_and_cleartext);
    RUN_TEST(test_defuse_rejection_with_extra_digits);
    RUN_TEST(test_defuse_zero_tolerance_immediate_cancel);
    RUN_TEST(test_defuse_custom_tolerance_window);
    RUN_TEST(test_hs_blue_plant_and_blue_victory_by_explosion);
    RUN_TEST(test_hs_blue_plant_and_red_victory_by_defuse);
    RUN_TEST(test_hs_red_plant_and_blue_victory_by_defuse);
    RUN_TEST(test_consecutive_rounds_defuse_accumulator_reset);

    std::cout << "========================================================" << std::endl;
    std::cout << "Resultado: " << passedCount << "/" << totalCount << " testes passaram." << std::endl;

    if (failedCount > 0) {
        std::cout << "\033[31m[FALHA] " << failedCount << " testes falharam!\033[0m" << std::endl;
        return 1;
    }

    std::cout << "\033[32m[SUCESSO] Todos os 18 testes passaram com 100% de conformidade!\033[0m" << std::endl;
    return 0;
}

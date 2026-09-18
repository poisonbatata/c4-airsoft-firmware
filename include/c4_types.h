#pragma once

#include <stdint.h>

/**
 * @file c4_types.h
 * @brief Definição de tipos, enums fortemente tipados e estruturas de estado
 * @ref constitution.md (EDR-001), spec.md (Seção 4) e plan.md
 */

// --- Modos de Jogo Selecionáveis ---
enum class GameMode : uint8_t {
    CS_GO,          // Modo clássico (Plantio, Contagem, Defuse)
    HIDE_AND_SEEK   // Modo alternativo Achar e Demolir
};

// --- Estados Finitos Operacionais (FSM) ---
enum class GameState : uint8_t {
    S0_OFF,         // Alimentação desligada
    S1_INIT,        // Inicialização e autoteste de periféricos
    S2_MENU,        // Menu de seleção de modo (B = H&S, C = CS:GO)
    S3_READY,       // Modo selecionado, aguardando senha de plantio TR
    S4_PLANTED,     // C4 plantada, contagem regressiva em andamento
    S5_DEFUSING,    // Defuse ativo (senha CT aceita e botão mantido)
    S6_DEFUSED,     // Vitória CT (C4 desarmada com sucesso)
    S7_EXPLODED_TR, // Vitória TR (tempo de explosão esgotado)
    S_ERROR         // Falha de hardware ou timeout crítico
};

// --- Cadências Sonoras do Buzzer ---
enum class BeepCadence : uint8_t {
    OFF,            // Silêncio
    SLOW_1HZ,       // 1 bip por segundo (1º terço: tempo > 30s)
    MEDIUM_2HZ,     // 2 bips por segundo (2º terço: 15s < tempo <= 30s)
    FAST_4HZ,       // 4 bips por segundo (3º terço final: tempo <= 15s)
    CONTINUOUS,     // Tom contínuo de fim de rodada TR
    SUCCESS_DOUBLE, // Bip duplo agudo de confirmação
    ERROR_TONE      // Tom grave curto para senha incorreta
};

// --- Times em Jogo (Especialmente Hide & Seek) ---
enum class Team : uint8_t {
    NONE = 0,
    BLUE,
    RED
};

// --- Padrões Visuais da Fita de LEDs WS2812B ---
enum class LedPattern : uint8_t {
    OFF,            // Fita apagada
    MENU_IDLE,      // Respiração leve ou indicação neutra
    BLINK_RED,      // Piscada vermelha sincronizada com o buzzer
    BLINK_BLUE,     // Piscada azul sincronizada com o buzzer (H&S Time Azul armado)
    SOLID_BLUE,     // Fita totalmente acesa em Azul (Vitória CT / Time Azul)
    SOLID_RED,      // Fita totalmente acesa em Vermelho (Vitória Time Vermelho)
    SOLID_YELLOW,   // Fita totalmente acesa em Amarelo (Vitória TR)
    ERROR_FLASH     // Piscada rápida vermelha de alerta
};

// --- Estrutura de Controle de Progresso do Defuse ---
struct DefuseProgress {
    uint32_t accumulatedTimeMs;     // Tempo já acumulado de botão pressionado
    uint32_t buttonReleasedTimestamp; // Timestamp em millis() no momento da soltura
    bool     isReleasedInTolerance; // Indica se está dentro da janela de 5s
    bool     isCompleted;           // Atingiu 100% (DEFUSE_TOTAL_TIME_MS)
};

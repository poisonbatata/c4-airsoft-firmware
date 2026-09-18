#pragma once

#include <stdint.h>

/**
 * @file c4_pins.h
 * @brief Pinagem oficial imutável do C4 Airsoft Firmware (ESP32 DevKit v1)
 * @ref C4A-ICD-001 (Versão 2.0 / Tabela 5.1) e constitution.md (Seção 2.2)
 */

namespace C4Pins {

    // --- Subsistema de Saída Sonora (Buzzer) ---
    constexpr uint8_t PIN_BUZZER          = 4;   // D4  - Acionamento sonoro (IF-004)

    // --- Subsistema de Saída Visual (LEDs Endereçáveis) ---
    constexpr uint8_t PIN_LED_WS2812B     = 13;  // D13 - Sinal de dados WS2812B (IF-005)

    // --- Subsistema de Entrada do Jogador (Botão de Defuse) ---
    constexpr uint8_t PIN_BUTTON_DEFUSE   = 14;  // D14 - Entrada digital c/ Pull-up interno (IF-003)

    // --- Subsistema de Entrada (Teclado Matricial 4x4) ---
    // Linhas (Saídas ativas em nível baixo durante varredura)
    constexpr uint8_t PIN_KEYPAD_ROW1     = 16;  // RX2 - Linha 1 da matriz (IF-002)
    constexpr uint8_t PIN_KEYPAD_ROW2     = 17;  // TX2 - Linha 2 da matriz (IF-002)
    constexpr uint8_t PIN_KEYPAD_ROW3     = 18;  // D18 - Linha 3 da matriz (IF-002)
    constexpr uint8_t PIN_KEYPAD_ROW4     = 19;  // D19 - Linha 4 da matriz (IF-002)

    // Colunas (Entradas com Pull-up interno)
    constexpr uint8_t PIN_KEYPAD_COL1     = 23;  // D23 - Coluna 1 da matriz (IF-002)
    constexpr uint8_t PIN_KEYPAD_COL2     = 25;  // D25 - Coluna 2 da matriz (IF-002)
    constexpr uint8_t PIN_KEYPAD_COL3     = 26;  // D26 - Coluna 3 da matriz (IF-002)
    constexpr uint8_t PIN_KEYPAD_COL4     = 27;  // D27 - Coluna 4 da matriz (IF-002)

    // --- Subsistema de Comunicação I2C (Display LCD 16x2) ---
    constexpr uint8_t PIN_I2C_SDA         = 21;  // D21 - I2C Serial Data (IF-006)
    constexpr uint8_t PIN_I2C_SCL         = 22;  // D22 - I2C Serial Clock (IF-006)

    // --- Pinos Livres / Reservados para Expansão Futura (V2) ---
    constexpr uint8_t PIN_RESERVED_32     = 32;  // D32 - Livre (RFID / Sensor de queda)
    constexpr uint8_t PIN_RESERVED_33     = 33;  // D33 - Livre (2º Buzzer / Sensor de queda)

} // namespace C4Pins

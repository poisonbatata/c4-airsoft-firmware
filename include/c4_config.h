#pragma once

#include <stdint.h>

/**
 * @file c4_config.h
 * @brief Configurações globais, temporizadores e parâmetros de hardware do C4 Airsoft
 * @ref spec.md (AC-01 a AC-17), plan.md (TDRs) e C4A-SW-0020 a C4A-SW-0050
 */

namespace C4Config {

    // --- Temporizadores Padrão do Modo Clássico CS:GO ---
    constexpr uint32_t BOMB_COUNTDOWN_MS          = 45000; // 45 segundos padrão para explosão cenográfica
    constexpr uint32_t DEFUSE_TOTAL_TIME_MS       = 10000; // 10 segundos padrão de botão mantido para defuse
    constexpr uint32_t DEFUSE_TOLERANCE_WINDOW_MS = 5000;  // 5 segundos de janela de tolerância ao soltar botão

    // --- Parâmetros de Senha e Buffer de Entrada ---
    constexpr uint8_t  PASSWORD_MAX_LEN           = 16;    // Até 16 caracteres para senha configurada
    constexpr uint8_t  INPUT_BUFFER_MAX_LEN       = 16;    // Até 16 dígitos digitados (evita truncamento silencioso)
    constexpr const char* DEFAULT_PASSWORD_TR     = "73556"; // Senha padrão Terrorista
    constexpr const char* DEFAULT_PASSWORD_CT     = "12345"; // Senha padrão Contra-Terrorista
    constexpr const char* DEFAULT_PASSWORD_KIT    = "1234";  // Senha padrão com Kit de Defuse
    constexpr const char* DEFAULT_PASSWORD_BLUE_PLANT  = "1111"; // H&S: Time Azul - Armar (taa)
    constexpr const char* DEFAULT_PASSWORD_BLUE_DEFUSE = "2222"; // H&S: Time Azul - Defusar (tad)
    constexpr const char* DEFAULT_PASSWORD_RED_PLANT   = "3333"; // H&S: Time Vermelho - Armar (tva)
    constexpr const char* DEFAULT_PASSWORD_RED_DEFUSE  = "4444"; // H&S: Time Vermelho - Defusar (tvd)

    // --- Parâmetros Acústicos do Buzzer (Frequências do Código Legado) ---
    constexpr uint32_t BUZZER_FREQ_BOMB_ALERT     = 4200;  // 4.200 Hz: ressonância máxima do piezo (bips de alerta)
    constexpr uint32_t BUZZER_FREQ_KEYPRESS       = 3000;  // 3.000 Hz: clique de feedback a cada tecla digitada
    constexpr uint32_t BUZZER_FREQ_CONFIRM        = 2000;  // 2.000 Hz: confirmação de armação / desarme
    constexpr uint32_t BUZZER_FREQ_ERROR          = 200;   // 200 Hz: tom grave de senha incorreta ou erro
    constexpr uint32_t BUZZER_KEYPRESS_DURATION_MS= 50;    // 50 ms por clique de tecla

    // --- Ponto de Acesso Wi-Fi e Web Server ---
    constexpr const char* WIFI_AP_SSID            = "C4_BOMB_CONFIG";
    constexpr const char* WIFI_AP_PASS            = "12345678"; // Mínimo 8 caracteres para WPA2
    constexpr uint16_t    WEB_SERVER_PORT         = 80;

    // --- Parâmetros do Display LCD 16x2 I2C ---
    constexpr uint8_t  LCD_I2C_ADDR               = 0x27;  // Endereço padrão PCF8574
    constexpr uint8_t  LCD_I2C_ADDR_FALLBACK      = 0x3F;  // Endereço alternativo
    constexpr uint8_t  LCD_COLS                   = 16;
    constexpr uint8_t  LCD_ROWS                   = 2;
    constexpr uint32_t LCD_REFRESH_INTERVAL_MS    = 100;   // Intervalo de amostragem de tela

    // --- Parâmetros da Fita LED WS2812B ---
    constexpr uint16_t LED_STRIP_COUNT            = 30;    // 30 LEDs (1 metro)
    constexpr uint8_t  LED_BRIGHTNESS_LIMIT       = 255;   // 100% de brilho total para visibilidade em campo aberto

    // --- Temporização de Debounce de Entradas ---
    constexpr uint32_t KEYPAD_DEBOUNCE_MS         = 50;    // Debounce matricial
    constexpr uint32_t BUTTON_DEBOUNCE_MS         = 30;    // Debounce do botão de defuse

    // --- Estrutura de Configuração em Tempo de Execução (Persistida na NVS) ---
    struct GameRuntimeConfig {
        char passwordTR[PASSWORD_MAX_LEN + 1];
        char passwordCT[PASSWORD_MAX_LEN + 1];
        char passwordKit[PASSWORD_MAX_LEN + 1];
        char passwordBluePlant[PASSWORD_MAX_LEN + 1];   // H&S Time Azul Armar (taa)
        char passwordBlueDefuse[PASSWORD_MAX_LEN + 1];  // H&S Time Azul Defusar (tad)
        char passwordRedPlant[PASSWORD_MAX_LEN + 1];    // H&S Time Vermelho Armar (tva)
        char passwordRedDefuse[PASSWORD_MAX_LEN + 1];   // H&S Time Vermelho Defusar (tvd)
        uint32_t bombTimeSeconds;
        uint32_t defuseTimeSeconds;
        uint32_t defuseKitSeconds;
        uint32_t defuseToleranceSeconds; // Tempo de tolerância ao soltar botão (0 a 30s)
        bool maskPassword;               // true: exibe '*', false: exibe caracteres claros
    };

} // namespace C4Config

#include <stdint.h>
#include "c4_pins.h"
#include "c4_config.h"
#include "c4_types.h"
#include "keypad_driver.h"
#include "button_driver.h"
#include "buzzer_driver.h"
#include "led_driver.h"
#include "display_driver.h"
#include "web_server_manager.h"
#include "game_fsm.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Instanciação dos módulos, HAL e Gerenciador Web
static KeypadDriver      s_keypad;
static ButtonDriver      s_button;
static BuzzerDriver      s_buzzer;
static LedDriver         s_led;
static DisplayDriver     s_display;
static WebServerManager  s_webServer;

// Instanciação da Máquina de Estados Finitos
static C4GameFSM         s_fsm(s_keypad, s_button, s_buzzer, s_led, s_display, s_webServer);

#ifdef ARDUINO
void setup() {
    // Inicialização da porta serial para depuração e diagnósticos
    Serial.begin(115200);
    Serial.println(F("========================================"));
    Serial.println(F("  C4 AIRSOFT - PRODUTO INTENSIVO"));
    Serial.println(F("  FIRMWARE v2.0 (SPEC-DRIVEN DEV)"));
    Serial.println(F("========================================"));

    // Inicialização da interface Web AP e NVS
    s_webServer.init();
    Serial.println(F("[OK] SoftAP Wi-Fi (C4_BOMB_CONFIG) e WebServer iniciados."));

    // Inicialização do hardware e da FSM
    s_fsm.init();
    Serial.println(F("[OK] Perifericos e FSM inicializados com sucesso."));
}

void loop() {
    // Captura o timestamp de amostragem em milissegundos
    uint32_t currentMillis = millis();

    // Atende requisições de clientes web conectados ao SoftAP
    s_webServer.handleClient();

    // Executa a iteração da máquina de estados (totalmente não-bloqueante)
    s_fsm.update(currentMillis);
}
#else
// Ponto de entrada padrão para ambientes sem Arduino (validação / build de bancada)
int main() {
    s_webServer.init();
    s_fsm.init();
    return 0;
}
#endif

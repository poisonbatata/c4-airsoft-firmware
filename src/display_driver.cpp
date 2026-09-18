#include "display_driver.h"
#include "c4_pins.h"
#include <stdio.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C s_lcd(C4Config::LCD_I2C_ADDR, C4Config::LCD_COLS, C4Config::LCD_ROWS);
#endif

DisplayDriver::DisplayDriver()
    : m_lastRefreshMs(0)
{
    memset(m_targetLine0, ' ', C4Config::LCD_COLS);
    m_targetLine0[C4Config::LCD_COLS] = '\0';
    memset(m_targetLine1, ' ', C4Config::LCD_COLS);
    m_targetLine1[C4Config::LCD_COLS] = '\0';

    memset(m_currentLine0, 0, sizeof(m_currentLine0));
    memset(m_currentLine1, 0, sizeof(m_currentLine1));
}

void DisplayDriver::init() {
#ifdef ARDUINO
    Wire.begin(C4Pins::PIN_I2C_SDA, C4Pins::PIN_I2C_SCL);
    s_lcd.init();
    s_lcd.backlight();
    s_lcd.clear();
#endif
    showMenu();
}

void DisplayDriver::setLines(const char* line0, const char* line1) {
    snprintf(m_targetLine0, sizeof(m_targetLine0), "%-16s", line0);
    snprintf(m_targetLine1, sizeof(m_targetLine1), "%-16s", line1);
}

void DisplayDriver::showMenu() {
    setLines(" C4 AIRSOFT v1.0", "[B]H&S   [C]CSGO");
}

void DisplayDriver::showPlantPrompt(const char* buffer, bool maskPassword) {
    char passLine[17];
    char displayStr[17] = {0};
    size_t len = strlen(buffer);
    for (size_t i = 0; i < len && i < 16; ++i) {
        displayStr[i] = maskPassword ? '*' : buffer[i];
    }
    if (len <= 6) {
        snprintf(passLine, sizeof(passLine), "SENHA:%-6.6s [#]", displayStr);
    } else {
        snprintf(passLine, sizeof(passLine), "%-12.12s [#]", displayStr);
    }
    setLines("PLANTAR C4 (TR):", passLine);
}

void DisplayDriver::showDefusePrompt(const char* buffer, bool maskPassword) {
    char passLine[17];
    char displayStr[17] = {0};
    size_t len = strlen(buffer);
    for (size_t i = 0; i < len && i < 16; ++i) {
        displayStr[i] = maskPassword ? '*' : buffer[i];
    }
    if (len <= 6) {
        snprintf(passLine, sizeof(passLine), "SENHA:%-6.6s [#]", displayStr);
    } else {
        snprintf(passLine, sizeof(passLine), "%-12.12s [#]", displayStr);
    }
    setLines("DEFUSAR C4 (CT):", passLine);
}

void DisplayDriver::showCountdown(uint32_t remainingMs) {
    uint32_t totalSec = (remainingMs + 999) / 1000;
    uint32_t min = totalSec / 60;
    uint32_t sec = totalSec % 60;

    char line1[17];
    snprintf(line1, sizeof(line1), "TEMPO:  %02u:%02u", min, sec);
    setLines(" * BOMB ARMED * ", line1);
}

void DisplayDriver::showDefusing(uint32_t remainingMs, uint32_t accumulatedDefuseMs, uint32_t totalDefuseMs, bool isPaused) {
    uint32_t pct = (accumulatedDefuseMs * 100) / (totalDefuseMs > 0 ? totalDefuseMs : 1);
    if (pct > 100) pct = 100;

    uint32_t totalSec = (remainingMs + 999) / 1000;
    char line0[17];
    char line1[17];

    if (isPaused) {
        snprintf(line0, sizeof(line0), "PAUSA! SEGURE...");
    } else {
        snprintf(line0, sizeof(line0), "DEFUSANDO: %3u%%", pct);
    }
    uint32_t displaySec = totalSec > 99 ? 99 : totalSec;
    snprintf(line1, sizeof(line1), "T:%02us [BOTAO]", (unsigned)displaySec);

    setLines(line0, line1);
}

void DisplayDriver::showPlantPromptHS(const char* buffer, bool maskPassword) {
    char passLine[17];
    char displayStr[17] = {0};
    size_t len = strlen(buffer);
    for (size_t i = 0; i < len && i < 16; ++i) {
        displayStr[i] = maskPassword ? '*' : buffer[i];
    }
    if (len <= 6) {
        snprintf(passLine, sizeof(passLine), "SENHA:%-6.6s [#]", displayStr);
    } else {
        snprintf(passLine, sizeof(passLine), "%-12.12s [#]", displayStr);
    }
    setLines("PLANTAR H&S:", passLine);
}

void DisplayDriver::showDefusePromptHS(const char* buffer, bool maskPassword, bool defusingTeamIsRed) {
    char passLine[17];
    char displayStr[17] = {0};
    size_t len = strlen(buffer);
    for (size_t i = 0; i < len && i < 16; ++i) {
        displayStr[i] = maskPassword ? '*' : buffer[i];
    }
    if (len <= 6) {
        snprintf(passLine, sizeof(passLine), "SENHA:%-6.6s [#]", displayStr);
    } else {
        snprintf(passLine, sizeof(passLine), "%-12.12s [#]", displayStr);
    }
    setLines(defusingTeamIsRed ? "DEFUSAR (VERM):" : "DEFUSAR (AZUL):", passLine);
}

void DisplayDriver::showDefusedCT() {
    setLines("  BOMB DEFUSED  ", "  CTS WIN! [A]  ");
}

void DisplayDriver::showExplodedTR() {
    setLines(" * EXPLOSION! * ", "TERRORISTS WIN[A]");
}

void DisplayDriver::showVictoryBlue() {
    setLines("  VITORIA AZUL! ", " TIME AZUL!  [A]");
}

void DisplayDriver::showVictoryRed() {
    setLines(" VITORIA VERM.! ", "TIME VERMELHO[A]");
}

void DisplayDriver::showError(const char* errorMsg) {
    setLines("ATENCAO:", errorMsg);
}

void DisplayDriver::update(uint32_t currentMillis) {
    if (currentMillis - m_lastRefreshMs < C4Config::LCD_REFRESH_INTERVAL_MS) {
        return;
    }
    m_lastRefreshMs = currentMillis;

#ifdef ARDUINO
    if (strncmp(m_currentLine0, m_targetLine0, C4Config::LCD_COLS) != 0) {
        s_lcd.setCursor(0, 0);
        s_lcd.print(m_targetLine0);
        memcpy(m_currentLine0, m_targetLine0, sizeof(m_currentLine0));
    }

    if (strncmp(m_currentLine1, m_targetLine1, C4Config::LCD_COLS) != 0) {
        s_lcd.setCursor(0, 1);
        s_lcd.print(m_targetLine1);
        memcpy(m_currentLine1, m_targetLine1, sizeof(m_currentLine1));
    }
#else
    memcpy(m_currentLine0, m_targetLine0, sizeof(m_currentLine0));
    memcpy(m_currentLine1, m_targetLine1, sizeof(m_currentLine1));
#endif
}

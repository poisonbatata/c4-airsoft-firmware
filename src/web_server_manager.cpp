#include "web_server_manager.h"
#include <string.h>
#include <stdio.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

static WebServer s_server(C4Config::WEB_SERVER_PORT);
static Preferences s_preferences;

// HTML embutido via PROGMEM extraído e otimizado do Versao2.4.ino
static const char HTML_CONFIG_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>C4 Airsoft - Painel Tatico</title>
    <style>
        :root {
            --bg-color: #121214;
            --card-bg: #1a1a1e;
            --accent: #de9b35;
            --text: #e1e1e6;
            --text-dark: #a8a8b3;
            --input-bg: #202024;
            --border: #29292e;
            --success: #04d361;
            --danger: #e25858;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', sans-serif; }
        body { background-color: var(--bg-color); color: var(--text); padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; }
        .container { width: 100%; max-width: 500px; background: var(--card-bg); border: 1px solid var(--border); border-radius: 8px; padding: 25px; box-shadow: 0 8px 24px rgba(0,0,0,0.5); }
        h1 { font-size: 1.4rem; text-align: center; margin-bottom: 20px; color: var(--accent); text-transform: uppercase; letter-spacing: 2px; border-bottom: 2px solid var(--border); padding-bottom: 10px; }
        .section-title { font-size: 0.85rem; color: var(--text-dark); text-transform: uppercase; letter-spacing: 1px; margin: 18px 0 8px 0; display: block; font-weight: bold; }
        .form-group { margin-bottom: 14px; }
        label { display: block; font-size: 0.85rem; margin-bottom: 5px; color: var(--text); }
        input[type="text"], input[type="number"], select { width: 100%; background: var(--input-bg); border: 1px solid var(--border); padding: 11px; border-radius: 4px; color: var(--text); font-size: 1rem; transition: border-color 0.2s; }
        input:focus, select:focus { outline: none; border-color: var(--accent); }
        .checkbox-group { display: flex; align-items: center; gap: 10px; margin-top: 10px; }
        .checkbox-group input { width: 18px; height: 18px; accent-color: var(--accent); cursor: pointer; }
        .checkbox-group label { margin-bottom: 0; cursor: pointer; }
        button { width: 100%; padding: 14px; font-size: 1rem; font-weight: bold; border-radius: 4px; cursor: pointer; text-transform: uppercase; transition: all 0.2s; }
        .btn-save { background: var(--accent); color: #000; border: none; margin-top: 20px; }
        .btn-save:hover { opacity: 0.9; }
        .btn-reset { background: transparent; color: var(--danger); border: 1px solid var(--danger); margin-top: 12px; }
        .btn-reset:hover { background: rgba(226, 88, 88, 0.1); }
        .alert { background: rgba(4, 211, 97, 0.15); border: 1px solid var(--success); color: var(--success); padding: 12px; border-radius: 4px; text-align: center; margin-bottom: 18px; font-size: 0.9rem; display: none; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Configuracao C4 Airsoft</h1>
        <div id="statusAlert" class="alert">Configuracoes salvas com sucesso!</div>
        <form action="/save" method="POST" id="configForm">
            <span class="section-title">Senhas Taticas</span>
            <div class="form-group">
                <label for="sa">Senha de Armacao (TR)</label>
                <input type="text" id="sa" name="sa" maxlength="16" required placeholder="Ex: 73556">
            </div>
            <div class="form-group">
                <label for="sd">Senha de Desarme Normal (CT)</label>
                <input type="text" id="sd" name="sd" maxlength="16" required placeholder="Ex: 12345">
            </div>
            <div class="form-group">
                <label for="sk">Senha de Desarme com Kit (CT)</label>
                <input type="text" id="sk" name="sk" maxlength="16" required placeholder="Ex: 1234">
            </div>

            <span class="section-title">Hide and Seek</span>
            <div class="form-group">
                <label for="taa">Time Azul - Armar</label>
                <input type="text" id="taa" name="taa" maxlength="16" required placeholder="Ex: 1111">
            </div>
            <div class="form-group">
                <label for="tad">Time Azul - Defusar</label>
                <input type="text" id="tad" name="tad" maxlength="16" required placeholder="Ex: 2222">
            </div>
            <div class="form-group">
                <label for="tva">Time Vermelho - Armar</label>
                <input type="text" id="tva" name="tva" maxlength="16" required placeholder="Ex: 3333">
            </div>
            <div class="form-group">
                <label for="tvd">Time Vermelho - Defusar</label>
                <input type="text" id="tvd" name="tvd" maxlength="16" required placeholder="Ex: 4444">
            </div>

            <span class="section-title">Tempos de Jogo (Segundos)</span>
            <div class="form-group">
                <label for="tb">Tempo de Contagem da Bomba (s)</label>
                <input type="number" id="tb" name="tb" min="5" max="999" required placeholder="Ex: 45">
            </div>
            <div class="form-group">
                <label for="tdn">Tempo de Desarme Normal (s)</label>
                <input type="number" id="tdn" name="tdn" min="1" max="99" required placeholder="Ex: 10">
            </div>
            <div class="form-group">
                <label for="tdk">Tempo de Desarme com Kit (s)</label>
                <input type="number" id="tdk" name="tdk" min="1" max="99" required placeholder="Ex: 5">
            </div>
            <div class="form-group">
                <label for="ttol">Tolerancia ao Soltar Botao de Defuse (s)</label>
                <input type="number" id="ttol" name="ttol" min="0" max="30" required placeholder="0 para desativar carência">
            </div>

            <span class="section-title">Preferencia de Exibicao</span>
            <div class="form-group">
                <div class="checkbox-group">
                    <input type="checkbox" id="mask" name="mask" value="1">
                    <label for="mask">Mascarar senha com asteriscos (*) no display</label>
                </div>
            </div>

            <button type="submit" class="btn-save">Gravar na Memoria Flash</button>
            <button type="button" class="btn-reset" onclick="resetarFabrica()">Restaurar Padrao de Fabrica</button>
        </form>
    </div>
    <script>
        window.onload = function() {
            fetch('/current-values').then(r => r.json()).then(data => {
                document.getElementById('sa').value = data.sa;
                document.getElementById('sd').value = data.sd;
                document.getElementById('sk').value = data.sk;
                document.getElementById('taa').value = data.taa;
                document.getElementById('tad').value = data.tad;
                document.getElementById('tva').value = data.tva;
                document.getElementById('tvd').value = data.tvd;
                document.getElementById('tb').value = data.tb;
                document.getElementById('tdn').value = data.tdn;
                document.getElementById('tdk').value = data.tdk;
                document.getElementById('ttol').value = data.ttol;
                document.getElementById('mask').checked = data.mask;
            });
            if (window.location.search.includes('success=true')) {
                const a = document.getElementById('statusAlert');
                a.style.display = 'block';
                setTimeout(() => { a.style.display = 'none'; }, 4000);
            }
        };

        function resetarFabrica() {
            if (confirm("Deseja restaurar as senhas e tempos originais?")) {
                document.getElementById('sa').value = "73556";
                document.getElementById('sd').value = "12345";
                document.getElementById('sk').value = "1234";
                document.getElementById('taa').value = "1111";
                document.getElementById('tad').value = "2222";
                document.getElementById('tva').value = "3333";
                document.getElementById('tvd').value = "4444";
                document.getElementById('tb').value = "45";
                document.getElementById('tdn').value = "10";
                document.getElementById('tdk').value = "5";
                document.getElementById('ttol').value = "5";
                document.getElementById('mask').checked = true;
                document.getElementById('configForm').submit();
            }
        }
    </script>
</body>
</html>
)=====";
#endif

WebServerManager::WebServerManager() {
    resetToDefaults();
}

void WebServerManager::resetToDefaults() {
    strncpy(m_config.passwordTR, C4Config::DEFAULT_PASSWORD_TR, sizeof(m_config.passwordTR));
    m_config.passwordTR[sizeof(m_config.passwordTR) - 1] = '\0';

    strncpy(m_config.passwordCT, C4Config::DEFAULT_PASSWORD_CT, sizeof(m_config.passwordCT));
    m_config.passwordCT[sizeof(m_config.passwordCT) - 1] = '\0';

    strncpy(m_config.passwordKit, C4Config::DEFAULT_PASSWORD_KIT, sizeof(m_config.passwordKit));
    m_config.passwordKit[sizeof(m_config.passwordKit) - 1] = '\0';

    strncpy(m_config.passwordBluePlant, C4Config::DEFAULT_PASSWORD_BLUE_PLANT, sizeof(m_config.passwordBluePlant));
    m_config.passwordBluePlant[sizeof(m_config.passwordBluePlant) - 1] = '\0';

    strncpy(m_config.passwordBlueDefuse, C4Config::DEFAULT_PASSWORD_BLUE_DEFUSE, sizeof(m_config.passwordBlueDefuse));
    m_config.passwordBlueDefuse[sizeof(m_config.passwordBlueDefuse) - 1] = '\0';

    strncpy(m_config.passwordRedPlant, C4Config::DEFAULT_PASSWORD_RED_PLANT, sizeof(m_config.passwordRedPlant));
    m_config.passwordRedPlant[sizeof(m_config.passwordRedPlant) - 1] = '\0';

    strncpy(m_config.passwordRedDefuse, C4Config::DEFAULT_PASSWORD_RED_DEFUSE, sizeof(m_config.passwordRedDefuse));
    m_config.passwordRedDefuse[sizeof(m_config.passwordRedDefuse) - 1] = '\0';

    m_config.bombTimeSeconds = C4Config::BOMB_COUNTDOWN_MS / 1000;
    m_config.defuseTimeSeconds = C4Config::DEFUSE_TOTAL_TIME_MS / 1000;
    m_config.defuseKitSeconds = 5;
    m_config.defuseToleranceSeconds = 5;
    m_config.maskPassword = true;
}

void WebServerManager::init() {
#ifdef ARDUINO
    loadFromNVS();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(C4Config::WIFI_AP_SSID, C4Config::WIFI_AP_PASS);

    setupRoutes();
    s_server.begin();
#endif
}

void WebServerManager::handleClient() {
#ifdef ARDUINO
    s_server.handleClient();
#endif
}

const C4Config::GameRuntimeConfig& WebServerManager::getConfig() const {
    return m_config;
}

void WebServerManager::setConfig(const C4Config::GameRuntimeConfig& config) {
    m_config = config;
    saveToNVS();
}

void WebServerManager::loadFromNVS() {
#ifdef ARDUINO
    s_preferences.begin("c4_game_props", true);

    String sa = s_preferences.getString("sa", C4Config::DEFAULT_PASSWORD_TR);
    String sd = s_preferences.getString("sd", C4Config::DEFAULT_PASSWORD_CT);
    String sk = s_preferences.getString("sk", C4Config::DEFAULT_PASSWORD_KIT);
    String taa = s_preferences.getString("taa", C4Config::DEFAULT_PASSWORD_BLUE_PLANT);
    String tad = s_preferences.getString("tad", C4Config::DEFAULT_PASSWORD_BLUE_DEFUSE);
    String tva = s_preferences.getString("tva", C4Config::DEFAULT_PASSWORD_RED_PLANT);
    String tvd = s_preferences.getString("tvd", C4Config::DEFAULT_PASSWORD_RED_DEFUSE);

    strncpy(m_config.passwordTR, sa.c_str(), sizeof(m_config.passwordTR));
    m_config.passwordTR[sizeof(m_config.passwordTR) - 1] = '\0';

    strncpy(m_config.passwordCT, sd.c_str(), sizeof(m_config.passwordCT));
    m_config.passwordCT[sizeof(m_config.passwordCT) - 1] = '\0';

    strncpy(m_config.passwordKit, sk.c_str(), sizeof(m_config.passwordKit));
    m_config.passwordKit[sizeof(m_config.passwordKit) - 1] = '\0';

    strncpy(m_config.passwordBluePlant, taa.c_str(), sizeof(m_config.passwordBluePlant));
    m_config.passwordBluePlant[sizeof(m_config.passwordBluePlant) - 1] = '\0';

    strncpy(m_config.passwordBlueDefuse, tad.c_str(), sizeof(m_config.passwordBlueDefuse));
    m_config.passwordBlueDefuse[sizeof(m_config.passwordBlueDefuse) - 1] = '\0';

    strncpy(m_config.passwordRedPlant, tva.c_str(), sizeof(m_config.passwordRedPlant));
    m_config.passwordRedPlant[sizeof(m_config.passwordRedPlant) - 1] = '\0';

    strncpy(m_config.passwordRedDefuse, tvd.c_str(), sizeof(m_config.passwordRedDefuse));
    m_config.passwordRedDefuse[sizeof(m_config.passwordRedDefuse) - 1] = '\0';

    m_config.bombTimeSeconds = s_preferences.getInt("tb", 45);
    if (m_config.bombTimeSeconds < 5) m_config.bombTimeSeconds = 45;

    m_config.defuseTimeSeconds = s_preferences.getInt("tdn", 10);
    if (m_config.defuseTimeSeconds < 1) m_config.defuseTimeSeconds = 10;

    m_config.defuseKitSeconds = s_preferences.getInt("tdk", 5);
    if (m_config.defuseKitSeconds < 1) m_config.defuseKitSeconds = 5;

    m_config.defuseToleranceSeconds = s_preferences.getInt("ttol", 5);
    if (m_config.defuseToleranceSeconds > 30) m_config.defuseToleranceSeconds = 5;

    m_config.maskPassword = s_preferences.getBool("mask", true);

    s_preferences.end();
#endif
}

void WebServerManager::saveToNVS() {
#ifdef ARDUINO
    s_preferences.begin("c4_game_props", false);

    s_preferences.putString("sa", m_config.passwordTR);
    s_preferences.putString("sd", m_config.passwordCT);
    s_preferences.putString("sk", m_config.passwordKit);
    s_preferences.putString("taa", m_config.passwordBluePlant);
    s_preferences.putString("tad", m_config.passwordBlueDefuse);
    s_preferences.putString("tva", m_config.passwordRedPlant);
    s_preferences.putString("tvd", m_config.passwordRedDefuse);
    s_preferences.putInt("tb", m_config.bombTimeSeconds);
    s_preferences.putInt("tdn", m_config.defuseTimeSeconds);
    s_preferences.putInt("tdk", m_config.defuseKitSeconds);
    s_preferences.putInt("ttol", m_config.defuseToleranceSeconds);
    s_preferences.putBool("mask", m_config.maskPassword);

    s_preferences.end();
#endif
}

void WebServerManager::setupRoutes() {
#ifdef ARDUINO
    s_server.on("/", HTTP_GET, []() {
        s_server.send_P(200, "text/html", HTML_CONFIG_PAGE);
    });

    s_server.on("/current-values", HTTP_GET, [this]() {
        char json[512];
        snprintf(json, sizeof(json),
                 "{\"sa\":\"%s\",\"sd\":\"%s\",\"sk\":\"%s\",\"taa\":\"%s\",\"tad\":\"%s\",\"tva\":\"%s\",\"tvd\":\"%s\","
                 "\"tb\":%u,\"tdn\":%u,\"tdk\":%u,\"ttol\":%u,\"mask\":%s}",
                 m_config.passwordTR,
                 m_config.passwordCT,
                 m_config.passwordKit,
                 m_config.passwordBluePlant,
                 m_config.passwordBlueDefuse,
                 m_config.passwordRedPlant,
                 m_config.passwordRedDefuse,
                 m_config.bombTimeSeconds,
                 m_config.defuseTimeSeconds,
                 m_config.defuseKitSeconds,
                 m_config.defuseToleranceSeconds,
                 m_config.maskPassword ? "true" : "false");
        s_server.send(200, "application/json", json);
    });

    s_server.on("/save", HTTP_POST, [this]() {
        if (s_server.hasArg("sa")) {
            String sa = s_server.arg("sa");
            strncpy(m_config.passwordTR, sa.c_str(), sizeof(m_config.passwordTR));
            m_config.passwordTR[sizeof(m_config.passwordTR) - 1] = '\0';
        }
        if (s_server.hasArg("sd")) {
            String sd = s_server.arg("sd");
            strncpy(m_config.passwordCT, sd.c_str(), sizeof(m_config.passwordCT));
            m_config.passwordCT[sizeof(m_config.passwordCT) - 1] = '\0';
        }
        if (s_server.hasArg("sk")) {
            String sk = s_server.arg("sk");
            strncpy(m_config.passwordKit, sk.c_str(), sizeof(m_config.passwordKit));
            m_config.passwordKit[sizeof(m_config.passwordKit) - 1] = '\0';
        }
        if (s_server.hasArg("taa")) {
            String taa = s_server.arg("taa");
            strncpy(m_config.passwordBluePlant, taa.c_str(), sizeof(m_config.passwordBluePlant));
            m_config.passwordBluePlant[sizeof(m_config.passwordBluePlant) - 1] = '\0';
        }
        if (s_server.hasArg("tad")) {
            String tad = s_server.arg("tad");
            strncpy(m_config.passwordBlueDefuse, tad.c_str(), sizeof(m_config.passwordBlueDefuse));
            m_config.passwordBlueDefuse[sizeof(m_config.passwordBlueDefuse) - 1] = '\0';
        }
        if (s_server.hasArg("tva")) {
            String tva = s_server.arg("tva");
            strncpy(m_config.passwordRedPlant, tva.c_str(), sizeof(m_config.passwordRedPlant));
            m_config.passwordRedPlant[sizeof(m_config.passwordRedPlant) - 1] = '\0';
        }
        if (s_server.hasArg("tvd")) {
            String tvd = s_server.arg("tvd");
            strncpy(m_config.passwordRedDefuse, tvd.c_str(), sizeof(m_config.passwordRedDefuse));
            m_config.passwordRedDefuse[sizeof(m_config.passwordRedDefuse) - 1] = '\0';
        }
        if (s_server.hasArg("tb")) {
            m_config.bombTimeSeconds = s_server.arg("tb").toInt();
            if (m_config.bombTimeSeconds < 5) m_config.bombTimeSeconds = 5;
        }
        if (s_server.hasArg("tdn")) {
            m_config.defuseTimeSeconds = s_server.arg("tdn").toInt();
            if (m_config.defuseTimeSeconds < 1) m_config.defuseTimeSeconds = 1;
        }
        if (s_server.hasArg("tdk")) {
            m_config.defuseKitSeconds = s_server.arg("tdk").toInt();
            if (m_config.defuseKitSeconds < 1) m_config.defuseKitSeconds = 1;
        }
        if (s_server.hasArg("ttol")) {
            m_config.defuseToleranceSeconds = s_server.arg("ttol").toInt();
            if (m_config.defuseToleranceSeconds > 30) m_config.defuseToleranceSeconds = 30;
        }

        m_config.maskPassword = s_server.hasArg("mask");

        saveToNVS();

        s_server.sendHeader("Location", "/?success=true");
        s_server.send(303);
    });
#endif
}

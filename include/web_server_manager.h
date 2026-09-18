#pragma once

#include <stdint.h>
#include "c4_config.h"

/**
 * @file web_server_manager.h
 * @brief Gerenciador de Ponto de Acesso Wi-Fi, Servidor Web e Persistência NVS (Preferences)
 * @ref spec.md (AC-13, AC-15), plan.md (ADR-002) e Versao2.4.ino
 */

class WebServerManager {
public:
    WebServerManager();

    /**
     * @brief Inicializa o Ponto de Acesso Wi-Fi, rotas HTTP e carrega configurações da NVS.
     */
    void init();

    /**
     * @brief Processa requisições HTTP de clientes conectados. Não-bloqueante.
     */
    void handleClient();

    /**
     * @brief Retorna as configurações ativas do jogo.
     */
    const C4Config::GameRuntimeConfig& getConfig() const;

    /**
     * @brief Atualiza a configuração diretamente (útil para testes unitários ou reset).
     */
    void setConfig(const C4Config::GameRuntimeConfig& config);

    /**
     * @brief Restaura as configurações de fábrica e persiste na NVS.
     */
    void resetToDefaults();

private:
    C4Config::GameRuntimeConfig m_config;

    void loadFromNVS();
    void saveToNVS();
    void setupRoutes();
};

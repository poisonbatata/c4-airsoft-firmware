#pragma once

#include <stdint.h>
#include "c4_config.h"

/**
 * @file keypad_driver.h
 * @brief Driver de leitura não-bloqueante do teclado matricial 4x4
 * @ref C4A-ELE-0030, IF-002, spec.md (AC-02, AC-14) e constitution.md (CWE-120 mitigação)
 */

enum class KeyAction {
    NONE,        // Nenhuma ação / tecla não relevante
    DIGIT_ADDED, // Dígito numérico '0'-'9' adicionado ao buffer
    CLEARED,     // Tecla 'A' pressionada: buffer limpo imediatamente
    SUBMIT       // Tecla '#' pressionada: comando de Enter / validação de senha
};

class KeypadDriver {
public:
    KeypadDriver();
    void init();

    /**
     * @brief Varre o teclado por teclas pressionadas. Não-bloqueante.
     * @return Caractere da tecla ('0'-'9', 'A', 'B', 'C', 'D', '*', '#') ou '\0' se nada foi pressionado.
     */
    char pollKey();

    /**
     * @brief Injeta tecla programaticamente (utilizado para testes de unidade automatizados).
     */
    void injectKey(char c);

    /**
     * @brief Processa o caractere recebido, gerenciando buffer, limpeza com 'A' e submissão com '#'.
     * @param c Caractere recebido.
     * @return KeyAction correspondente à ação executada.
     */
    KeyAction handleInput(char c);

    /**
     * @brief Esvazia imediatamente o buffer de senha.
     */
    void clearBuffer();

    /**
     * @brief Retorna o conteúdo atual do buffer de senha (sempre terminado em '\0').
     */
    const char* getBuffer() const;

    /**
     * @brief Retorna a quantidade de dígitos preenchidos no buffer.
     */
    uint8_t getLength() const;

    /**
     * @brief Verifica se o buffer atingiu exatamente o tamanho nominal de senha.
     */
    bool isComplete() const;

private:
    char m_buffer[C4Config::INPUT_BUFFER_MAX_LEN + 1];
    uint8_t m_length;
};

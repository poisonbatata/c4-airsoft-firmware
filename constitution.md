# Constitution: C4 Airsoft Firmware

**Projeto:** C4 Airsoft — Produto Intensivo de Software (PIS)  
**Documento:** `constitution.md` (Constituição do Sistema)  
**Autor:** Human Architect  
**Fase SDD:** 1 de 6 (Security & Foundation)  
**Impacto a Jusante:** Restrição Global Obrigatória (*Global Constraint*)  
**Referência Canônica:** `C4A-ICD-001` (ICD v1.0), `C4A-DES-001` (Descritivo v1.0), `C4A-PB-001` (Backlog v2.0)

---

## 1. Foundational Principles

A Constituição situa-se no ápice da hierarquia do *Spec-Driven Development* (SDD). Ela estabelece as leis imutáveis de engenharia de software e hardware, impedindo que agentes autônomos de IA ou desenvolvedores humanos introduzam bibliotecas proibidas, padrões vulneráveis ou premissas físicas inexistentes.

### 1.1 Níveis de Exigência Normativa (RFC 2119 Compliance)
As declarações normativas deste ecossistema utilizam rigorosamente a terminologia da **RFC 2119**:
- **MUST / SHALL / OBRIGATÓRIO:** Define uma exigência incondicional e absoluta para segurança, estabilidade física ou conformidade de interface. A violação reprova imediatamente o build.
- **SHOULD / RECOMENDADO:** Define uma boa prática de engenharia que deve ser seguida, a menos que haja justificativa técnica documentada e aprovada em TDR (*Technical Decision Record*).
- **MAY / OPCIONAL:** Define itens de conveniência, recursos cosméticos ou expansões futuras do roadmap.

### 1.2 Segurança por Construção (*Security by Construction*)
O dispositivo C4 Airsoft é classificado como um produto cenográfico eletrônico de entretenimento esportivo. As seguintes regras de segurança física e cibernética são irrevogáveis:

1. **Invariante Cenográfica (Segurança Física):**
   - O sistema **MUST NOT** controlar, acionar ou intermediar nenhuma carga explosiva, pirotécnica, agente químico, mecanismo incendiário ou atuador de disparo balístico.
   - Qualquer simulação de "explosão" **MUST** ser estritamente audiovisual (som no buzzer, cores e animações na fita LED e texto no display LCD).
2. **Mitigação de Vulnerabilidades Mapeadas (CWE Compliance):**
   - **CWE-120 (Buffer Copy without Checking Size of Input):** Todos os buffers de leitura de senha (teclado 4x4) e strings de exibição **MUST** possuir tamanhos fixos pré-alocados com verificação estrita de limites de índice. É terminantemente proibido ler dados além do tamanho nominal do buffer de senha (5 dígitos numéricos).
   - **CWE-676 (Use of Potentially Dangerous Function):** Funções inseguras de manipulação de memória e strings da biblioteca padrão C (como `strcpy`, `strcat`, `sprintf` sem controle de tamanho) **MUST NOT** ser utilizadas. Utilizar equivalentes seguros com limite explícito (`strncpy`, `snprintf`) ou estruturas de tamanho fixo.
   - **CWE-835 (Loop with Unreachable Exit Condition / Watchdog Timeout):** Nenhum loop de polling de teclado, leitura de sensores ou efeito de animação **MUST** executar em modo bloqueante infinito. O watchdog timer (WDT) do ESP32 deve ser alimentado regularmente por meio de execução não-bloqueante cooperativa.
   - **CWE-400 (Uncontrolled Resource Consumption):** O uso do heap (alocação dinâmica contínua via `malloc`, `free` ou instanciação frequente de objetos `String`) **MUST NOT** ser executado durante o ciclo de vida da partida para evitar fragmentação de memória e travamentos em campo (*OutOfMemory*).

---

## 2. Architectural Baselines

### 2.1 Fronteiras do Stack Tecnológico (*Tech Stack Boundaries*)
- **Plataforma de Execução:** Microcontrolador Espressif ESP32 DevKit v1 (32-bit dual-core Xtensa LX6, 520 KB SRAM).
- **Linguagem & Framework:** C++17 compilado sob o ecossistema Arduino Core para ESP32 (compatível com PlatformIO e Arduino IDE).
- **Paradigma de Concorrência:** Loop único orientado a Máquina de Estados Finitos (FSM) determinística com temporização baseada em delta de tempo (`millis()`). O uso da instrução bloqueante `delay()` **MUST NOT** ser tolerado em nenhuma rotina da máquina de estados do jogo.
- **Bibliotecas Homologadas (*Approved Libraries Only*):**
  - Leitura de Teclado Matricial: `Keypad` (Mark Stanley / Alexander Brevig).
  - Controle de Display LCD I2C: `LiquidCrystal_I2C` (endereço `0x27` ou `0x3F`, 16 colunas x 2 linhas).
  - Controle de LEDs Endereçáveis: `FastLED` ou `Adafruit_NeoPixel` (configurado para barramento WS2812B, ordem de cor GRB, 800 kHz).
  - Rede Local e Persistência NVS (Nativas ESP32): `WiFi.h`, `WebServer.h` e `Preferences.h` para o portal de configuração e armazenamento de parâmetros de jogo.
  - **Regra de Bloqueio:** Nenhuma outra biblioteca de terceiros poderá ser adicionada sem a criação prévia de um registro de decisão técnica (`EDR`).

### 2.2 Pinagem de Hardware Imutável (Baseada em `C4A-ICD-001` v2.0)
Todos os identificadores de pinos GPIO **MUST** ser centralizados no arquivo `include/c4_pins.h`. Qualquer divergência entre o código e esta tabela constitui falha de contrato:

| Rótulo | GPIO ESP32 | Função no Subsistema | Tipo Elétrico | Contrato ICD |
| :--- | :--- | :--- | :--- | :--- |
| **D4** | `GPIO 4` | BUZZER | Saída Digital / PWM | IF-004 / `C4A-ELE-0040` |
| **D13** | `GPIO 13` | FITA DE LED (WS2812B) | Saída Digital (Dados 800kHz) | IF-005 / `C4A-ELE-0050` |
| **D14** | `GPIO 14` | BOTÃO DE DEFUSE | Entrada Digital (Pull-up interno) | IF-003 / `C4A-ELE-0030` |
| **RX2** | `GPIO 16` | LINHA 1 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **TX2** | `GPIO 17` | LINHA 2 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D18** | `GPIO 18` | LINHA 3 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D19** | `GPIO 19` | LINHA 4 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D21** | `GPIO 21` | SDA (I2C DATA) | I2C Bidirecional (Pull-up) | IF-006 / `C4A-ELE-0060` |
| **D22** | `GPIO 22` | SCL (I2C CLOCK) | I2C Clock (Pull-up) | IF-006 / `C4A-ELE-0060` |
| **D23** | `GPIO 23` | COLUNA 1 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D25** | `GPIO 25` | COLUNA 2 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D26** | `GPIO 26` | COLUNA 3 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D27** | `GPIO 27` | COLUNA 4 (TECLADO 4x4) | Entrada/Saída Matricial | IF-002 / `C4A-ELE-0030` |
| **D32** | `GPIO 32` | RESERVADO (LIVRE) | Expansão Futura (RFID/Queda) | IF-009 / `C4A-FUT-0010` |
| **D33** | `GPIO 33` | RESERVADO (LIVRE) | Expansão Futura (2º Buzzer) | `C4A-ELE-0040` |

### 2.3 Restrições Elétricas e de Alimentação (PWR Compliance)
- **GND Comum (PWR-005):** O GND do microcontrolador, display I2C, fita LED, buzzer e chave **MUST** ser compartilhado sem loops de terra.
- **Tensão de Barramento (PWR-003):** O display LCD e a fita WS2812B operam na linha de 5V suprida pelo conversor Step-up regulado. As linhas GPIO de dados operam no nível lógico nativo de 3.3V do ESP32.
- **Prevenção de Brownout:** Em momentos de máxima atividade combinada (LEDs brancos no pico + buzzer contínuo), o firmware **SHOULD** limitar o brilho global da fita LED (FastLED brightness máx. 128 de 255) para manter o consumo sob a margem de segurança da bateria e do step-up.

---

## 3. Directory Conventions

A base de código de firmware **MUST** estruturar-se rigorosamente de acordo com o seguinte layout físico:

```text
c4-airsoft-firmware/
├── constitution.md       # Este documento (Apex / Leis Imutáveis)
├── spec.md               # Especificação Funcional (O quê e Porquê)
├── plan.md               # Plano de Implementação Técnica & TDRs (O Como)
├── tasks.md              # Decomposição Atômica de Tarefas & DAG de Ondas
├── include/              # Cabeçalhos e definições públicas
│   ├── c4_config.h       # Parâmetros de tempos, senhas padrão e flags
│   ├── c4_pins.h         # Constantes de pinagem GPIO do ESP32
│   ├── c4_types.h        # Enums de estado (GameState), eventos e structs
│   ├── buzzer_driver.h   # Interface do subsistema sonoro
│   ├── display_driver.h  # Interface do LCD 16x2 I2C
│   ├── keypad_driver.h   # Interface do teclado matricial 4x4
│   ├── led_driver.h      # Interface da fita WS2812B
│   ├── button_driver.h   # Interface do botão de defuse
│   └── game_fsm.h        # Controlador da Máquina de Estados
├── src/                  # Implementações em C++
│   ├── main.cpp          # Ponto de entrada (setup e loop não-bloqueante)
│   ├── buzzer_driver.cpp # Implementação de bips e frequências por terços
│   ├── display_driver.cpp# Renderização de mensagens e telas do LCD
│   ├── keypad_driver.cpp # Varredura com debounce e suporte à tecla 'A'
│   ├── led_driver.cpp    # Padrões visuais (Vermelho, Azul, Amarelo, Pulso)
│   ├── button_driver.cpp # Leitura do botão com janela de tolerância de 5s
│   └── game_fsm.cpp      # Transições de estado CS:GO / Hide-and-Seek
└── test/                 # Testes unitários de lógica da FSM e contratos
    └── test_fsm_core.cpp # Simulação pura das regras de transição de estado
```

### 3.1 Convenções de Nomenclatura
- **Arquivos:** `snake_case.h`, `snake_case.cpp`
- **Tipos e Classes:** `PascalCase` (ex: `C4GameFSM`, `KeypadDriver`)
- **Constantes e Enums:** `UPPER_SNAKE_CASE` (ex: `STATE_PLANTED`, `PIN_BUZZER`)
- **Variáveis e Funções:** `camelCase` (ex: `remainingTimeMs`, `updateBuzzer()`)

---

## 4. Anti-Patterns & Prohibited Practices

1. **Uso de `delay()`:** Proibido no loop principal e nos módulos de driver. Toda temporização deve calcular `currentMillis - previousMillis >= interval`.
2. **Hardcoding de Pinos:** Proibido em qualquer arquivo de `src/`. Todas as referências a pinos de hardware devem vir exclusivamente de `include/c4_pins.h`.
3. **Senhas com Tamanho Variável sem Validação:** Proibido buffer dinâmico. O buffer de entrada aceita exatamente 5 caracteres numéricos; a tecla `A` limpa o buffer.
4. **Acoplamento Direto Hardware-Regra:** A lógica do jogo (`game_fsm.cpp`) **MUST NOT** invocar chamadas diretas de GPIO como `digitalWrite()`; deve sempre interagir através das classes/funções da camada de abstração de drivers (`buzzer_driver`, `led_driver`, etc.).

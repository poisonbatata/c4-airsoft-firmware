# Technical Implementation Plan: C4 Airsoft Firmware

**Projeto:** C4 Airsoft — Produto Intensivo de Software (PIS)  
**Documento:** `plan.md` (Plano de Implementação Técnica e Decisões de Engenharia)  
**Autor:** AI Architected / Human Approved  
**Fase SDD:** 3 de 6 (Plan: The How — Architecture & Decisions)  
**Impacto a Jusante:** Contexto Primário de Engenharia para Geração de Código  
**Documentos de Origem:** `constitution.md`, `spec.md`, `C4A-ICD-001` (ICD v1.0)

---

## 1. Arquitetura Modular e Estrutura de Componentes

A arquitetura do firmware é projetada segundo o padrão **Separation of Concerns (SoC)** e isolamento de hardware por meio de uma camada de abstração de periféricos (**HAL - Hardware Abstraction Layer**), separando rigorosamente os drivers físicos da máquina de estados do jogo.

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                                     main.cpp                                            │
│                         (setup() & loop() não-bloqueante)                               │
└───────────────────────────────────────────┬─────────────────────────────────────────────┘
                                            │ Ciclo de Tick (a cada iteração)
                                            ▼
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                                    C4GameFSM                                            │
│                            (Núcleo da Máquina de Estados)                               │
│  - Gerencia estados: S1_INIT, S2_MENU, S3_READY, S4_PLANTED, S5_DEFUSING, S6, S7        │
│  - Controla temporizadores de rodada (45s) e defuse (10s)                               │
│  - Avalia regras de transição de estado e penalidade de tolerância de 5s                │
└───────────────────────▲───────────────────────────────────────┬─────────────────────────┘
                        │ Eventos                               │ Ações de Saída
         ┌──────────────┴──────────────┐         ┌──────────────┴──────────────┐
         ▼                             ▼         ▼              ▼              ▼
┌─────────────────┐   ┌─────────────────┐ ┌─────────────┐ ┌────────────┐ ┌────────────────┐
│  KeypadDriver   │   │  ButtonDriver   │ │BuzzerDriver │ │ LedDriver  │ │ DisplayDriver  │
│  (Teclado 4x4)  │   │ (Botão Defuse)  │ │   (Buzzer)  │ │ (WS2812B)  │ │ (LCD 16x2 I2C) │
└────────┬────────┘   └────────┬────────┘ └──────┬──────┘ └─────┬──────┘ └───────┬────────┘
         │ GPIO 16-27          │ GPIO 14         │ GPIO 4       │ GPIO 13        │ GPIO 21, 22
         ▼                     ▼                 ▼              ▼                ▼
   [Hardware Físico: Teclado, Botão, Buzzer, Fita LED Endereçável e LCD I2C 16x2]
```

### 1.1 Responsabilidades dos Módulos

1. **`KeypadDriver` (`include/keypad_driver.h`, `src/keypad_driver.cpp`):**
   - Abstrai a matriz 4x4 usando a biblioteca `Keypad`.
   - Executa varredura com debounce interno (50 ms).
   - Gerencia o buffer estático de senha de até 16 caracteres (`char buffer[17]`), evitando truncamento prematuro para garantir validação estrita de senhas.
   - Processa a tecla `A` como limpeza do buffer e `#` como confirmação (Enter).
   - Emite bip sonoro de feedback (3.000 Hz, 50ms) a cada tecla pressionada.
2. **`ButtonDriver` (`include/button_driver.h`, `src/button_driver.cpp`):**
   - Monitora o pino do botão de defuse (`GPIO 14`) em `INPUT_PULLUP` (ativo em `LOW`).
   - Implementa debounce temporal por software (30 ms).
   - Detecta transições: Pressionado, Mantido e Solto.
   - Computa janela de tolerância de soltura configurável (0 a 30 segundos, default 5s) e regra de penalidade de 50% (com cancelamento imediato caso tolerância = 0s).
3. **`BuzzerDriver` (`include/buzzer_driver.h`, `src/buzzer_driver.cpp`):**
   - Controla o `GPIO 4` gerando onda quadrada em frequência ressonante via `tone()` e `noTone()`.
   - Emite bips de contagem a **4.200 Hz** (pulso de 80ms).
   - Emite feedback de teclas a **3.000 Hz** (50ms).
   - Emite confirmação a **2.000 Hz** e som de erro a **200 Hz**.
4. **`LedDriver` (`include/led_driver.h`, `src/led_driver.cpp`):**
   - Controla os 30 LEDs WS2812B no pino `GPIO 13` com brilho total (255 / 100%).
   - Sincroniza o pulso luminoso vermelho ou azul com o estado ativo do buzzer no estado `S4` (conforme time que armou no modo H&S ou padrão TR).
   - Projeta iluminação contínua Azul (Vitória CT / Time Azul), Vermelha (Vitória Time Vermelho) e Amarela (Vitória TR).
5. **`DisplayDriver` (`include/display_driver.h`, `src/display_driver.cpp`):**
   - Comunica com o LCD 16x2 via barramento I2C com shadow buffer anti-flicker.
   - Suporta exibição configurável de senha: modo mascarado (`*`) ou texto claro, telas táticas CS:GO e prompts de equipe H&S.
6. **`WebServerManager` (`include/web_server_manager.h`, `src/web_server_manager.cpp`):**
   - Inicializa o SoftAP Wi-Fi (`C4_BOMB_CONFIG`, IP `192.168.4.1`) e WebServer na porta 80.
   - Serve a página tática responsiva embutida em `PROGMEM` e rotas `/current-values` e `/save` com suporte a senhas CS:GO e Hide & Seek.
   - Gerencia a persistência NVS com a classe `Preferences`.
7. **`C4GameFSM` (`include/game_fsm.h`, `src/game_fsm.cpp`):**
   - Coordena o ciclo de vida completo do jogo em ambos os modos (CS:GO e Hide and Seek), avaliando entradas, senhas cruzadas e atuadores.
   - Valida senhas apenas ao receber a tecla `#` e limpa o buffer com a tecla `A`.

---

## 2. Mapeamento Tecnológico e Protocolos de Comunicação

### 2.1 Mapeamento I2C (Display LCD)
- **Barramento:** Hardware I2C 0 nativo do ESP32.
- **Pinos:** `SDA = GPIO 21`, `SCL = GPIO 22`.
- **Frequência de Clock:** 100 kHz (Modo Padrão I2C).
- **Endereço do Módulo:** `0x27` (com fallback para `0x3F` configurável em `c4_config.h`).
- **Padrão de Transmissão:** O display é atualizado com intervalo mínimo de 150 ms para economizar ciclos de CPU e evitar saturação do barramento I2C.

### 2.2 Matriz do Teclado 4x4
- **Linhas (Saídas ativas em nível baixo durante varredura):**
  - Linha 1: `GPIO 16`
  - Linha 2: `GPIO 17`
  - Linha 3: `GPIO 18`
  - Linha 4: `GPIO 19`
- **Colunas (Entradas com resistores de Pull-up internos ativos):**
  - Coluna 1: `GPIO 23`
  - Coluna 2: `GPIO 25`
  - Coluna 3: `GPIO 26`
  - Coluna 4: `GPIO 27`
- **Mapeamento de Caracteres da Matriz:**
  ```text
  [1] [2] [3] [A]   <- Linha 1
  [4] [5] [6] [B]   <- Linha 2
  [7] [8] [9] [C]   <- Linha 3
  [*] [0] [#] [D]   <- Linha 4
  ```
  *Convenção de Comandos:* Digitação numérica (`0-9`), Limpeza de buffer (`A`), Hide-and-Seek no menu (`B`), CS:GO no menu (`C`).

### 2.3 Barramento WS2812B (Fita de LEDs)
- **Linha de Dados:** `GPIO 13`.
- **Protocolo:** NRZ de 1 fio (*single-wire NZR*), frequência de 800 kHz (taxa de bit 1.25 µs).
- **Ordem de Cores:** `GRB` (padrão físico da fita WS2812B).
- **Brilho e Potência:** O brilho é configurado no patamar pleno de 100% (`FastLED.setBrightness(255)`), garantindo alta visibilidade luminosa em campos abertos de jogo.

---

## 3. Technical Decision Records (TDRs)

```yaml
---
tdr_id: MDD-001
type: Major Design Decision
status: APPROVED
date: 2026-09-18
title: Adoção de Máquina de Estados Não-Bloqueante Monolítica vs. FreeRTOS Preemptivo
context: O ESP32 possui suporte nativo ao FreeRTOS com múltiplas threads e filas. No entanto, o sistema C4 Airsoft possui baixa complexidade computacional, estado centralizado compartilhado e interfaces interdependentes (ex: buzzer e LED devem sincronizar precisamente durante a contagem).
decision: Implementar uma arquitetura de loop cooperativo único baseado em Máquina de Estados Finitos (FSM) orientada a delta de tempo (millis()), sem tarefas preemptivas separadas.
consequences:
  positive:
    - Elimina condições de corrida (race conditions) e necessidade de mutexes/semáforos.
    - Facilidade extrema de testes unitários isolados da lógica de jogo.
    - Determinismo temporal garantido e menor consumo de memória RAM (sem pilhas de tarefas separadas).
  negative:
    - Nenhuma função ou driver pode executar chamadas bloqueantes (exige disciplina rigorosa de não-bloqueio).
---
```

```yaml
---
tdr_id: ADR-001
type: Architectural Decision Record
status: APPROVED
date: 2026-09-18
title: Temporização Assíncrona via millis() Delta vs. Interrupções de Timer por Hardware (ISR)
context: O controle de bips sonoros, piscadas de LED e tempos de rodada requer precisão na escala de centenas de milissegundos.
decision: Utilizar temporização orientada a amostragem com uint32_t currentMillis = millis() no loop principal, calculando deltas para cada subsistema.
consequences:
  positive:
    - Evita execução de código complexo (como chamadas I2C ou atualização de LEDs) dentro de rotinas de interrupção (ISRs), o que causaria travamento no ESP32.
    - Código legível, portátil e fácil de simular em testes de software puro.
  negative:
    - A resolução temporal fica vinculada à duração da iteração mais longa do loop (mantida abaixo de 5ms).
---
```

```yaml
---
tdr_id: ADR-002
type: Architectural Decision Record
status: APPROVED
date: 2026-09-18
title: Portal Web em Wi-Fi SoftAP e Persistência NVS via Preferences
context: O operador do jogo necessita configurar senhas, tempos e modo de exibição sem necessidade de recompilar o firmware ou conectar o microcontrolador via USB.
decision: Criar um Ponto de Acesso Wi-Fi autônomo (C4_BOMB_CONFIG) e hospedar um WebServer HTTP com página tática responsiva embutida em PROGMEM, persistindo dados na partição NVS com Preferences.
consequences:
  positive:
    - Configuração imediata via smartphone ou laptop em campo sem depender de internet externa.
    - Persistência não-volátil robusta entre reinicializações.
    - Processamento assíncrono via handleClient() perfeitamente integrado ao loop cooperativo.
  negative:
    - Pequeno acréscimo de consumo de corrente de RF do rádio Wi-Fi durante o funcionamento.
---
```

```yaml
---
tdr_id: EDR-001
type: Engineering Decision Record
status: APPROVED
date: 2026-09-18
title: Alocação de Memória Estática e Tipagem Primitiva Fixa
context: Sistemas embarcados sujeitos a reinicializações em campo frequentemente sofrem de fragmentação do heap causada pela classe String do Arduino e alocações dinâmicas repetitivas.
decision: Proibir o uso de String dinâmica e malloc/new no ciclo de vida do jogo. Todas as strings e buffers utilizam arrays estáticos de char (char buffer[N]) e tipos primitivos da stdint.h (uint8_t, uint16_t, uint32_t).
consequences:
  positive:
    - Uso de memória RAM 100% determinístico e previsível em tempo de compilação.
    - Zero fragmentação de heap e imunidade a falhas de falta de memória (OOM).
  negative:
    - Manipulação de strings requer funções com controle de tamanho fixo (snprintf).
---
```

```yaml
---
tdr_id: CDR-001
type: Contract Decision Record
status: APPROVED
date: 2026-09-18
title: Contratos de Interface Hardware-Software (CTR-001 a CTR-006 do ICD)
context: O documento C4A-ICD-001 estabelece contratos formais de interface entre subsistemas físicos e o firmware.
decision: Implementar os contratos de interface exatamente como definidos:
  - CTR-001: Validação estrita de senha (comprimento exato, caracteres excedentes invalidam a senha sem truncamento).
  - CTR-002: Escalonamento automático de frequência de alerta a cada terço de tempo de explosão.
  - CTR-003: Acúmulo de progresso de defuse estritamente condicionado ao botão mantido em nível LOW.
  - CTR-004: Janela de tolerância de soltura de botão configurável dinamicamente (0 a 30 segundos, default 5s) com penalidade proporcional de 50%.
  - CTR-005: Isolamento de parâmetros operacionais no arquivo c4_config.h e persistência NVS.
  - CTR-006: Feedback audiovisual de vitória imediato e imutável até reset via tecla 'A'.
consequences:
  positive:
    - Conformidade 100% auditável com a documentação do projeto e aprovação nos testes de integração física.
---
```

```yaml
---
tdr_id: TDM-001
type: Tech Decision Memo (Matriz de Tecnologias Rejeitadas)
status: REJECTED_ALTERNATIVES_RECORDED
date: 2026-09-18
title: Registro de Padrões e Tecnologias Rejeitadas para Evitar Retrabalho de IA
rejected_items:
  - item: "delay() no loop do jogo"
    reason: "Bloqueia o microcontrolador, impede a varredura do teclado e desativa a detecção de soltura do botão de defuse."
  - item: "Arduino String Object"
    reason: "Fragmenta a SRAM do ESP32 por alocações repetidas de tamanho variável, podendo causar travamento após rodadas sucessivas."
  - item: "Display SPI Gráfico no MVP"
    reason: "Aumenta a complexidade de fiação, consome pinos adicionais de GPIO que seriam necessários para o teclado e eleva o custo."
  - item: "Wi-Fi e WebServer ativo durante a partida no MVP"
    reason: "Consumo excessivo de corrente da bateria e risco de instabilidade na temporização crítica do jogo; postergado para a versão 2."
---
```

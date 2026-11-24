# 🧃 Dollynho Toy - Robô Assistente Inteligente com ESP32-S3

> *"Oi, eu sou o Dollynho Toy, seu amiguinho inteligente de bancada!"*

O **Dollynho Toy** é um robô assistente de mesa interativo baseado no microcontrolador **ESP32-S3 (N16R8)**. Ele combina animações de olhos ultra-fluidas com a biblioteca **FluxGarage RoboEyes**, integração de inteligência artificial com o **Google Gemini 2.5 Flash**, monitoramento da **Bolsa de Valores B3 (BBAS3)**, sintetizador de **músicas clássicas 8-bits (Star Wars, Mario, Tetris)** e um **painel web completo com reconhecimento de voz**!

---

## 🚀 Recursos Principais

- **🎭 Expressões Faciais Orgânicas (FluxGarage RoboEyes):**
  - Animação a 50 FPS de olhinhos no estilo *Cozmo / WALL-E*.
  - Piscamento natural automático (`autoblinker`) e olhar curioso em direções aleatórias (`idleMode`).
  - Estados emocionais: **Feliz (`HAPPY`)**, **Bravo (`ANGRY`)**, **Sonolento/Preocupado (`TIRED`)**, **Normal (`DEFAULT`)**, **Gargalhada (`anim_laugh`)**, **Dúvida/Curiosidade (`anim_confused`)** e suor na testa (`setSweat`).
- **💡 Sistema Inteligente de LEDs Pulsantes Conforme o Sentimento:**
  - **Pulsação Orgânica Contínua nos LEDs Externos e On-Board RGB (GPIO 48):**
    - 💚 **Feliz (`HAPPY`):** Pulsação respiratória alegre e suave em Verde Esmeralda (~1.3s).
    - 🔴 **Bravo (`ANGRY`):** Pulsação rápida, tensa e agressiva em Vermelho Vivo (~0.5s).
    - 🟠 **Cansado / Triste (`TIRED` / `SAD`):** Respiração lenta, suave e fading em Âmbar/Vermelho fraco (~3.5s).
    - 🔵 **Normal / Foco (`DEFAULT`):** Pulsação zen rítmica e calma em Ciano (~2.3s).
    - 💖 **Amor (`LOVE` no Dashboard):** Batimento duplo cardíaco (*thump-thump*) em Rosa Vibrante (~1.1s).
  - **LED RGB On-Board (GPIO 48) - Alertas do Sistema:**
    - 💚 **3 piscadas verdes:** Wi-Fi conectado com sucesso!
    - 💙 **1 piscada azul:** Resposta recebida da API Gemini IA (OK)!
    - 🪙 **Brilho Dourado:** Disparado ao coletar moedas do Mario (+200 PTS)!
  - **Ajuste Físico:** Controle de teto de brilho analógico em tempo real via potenciômetro (GPIO 1).
- **📈 Monitor da Bolsa B3 (BBAS3 - Banco do Brasil):**
  - Consulta automática via Wi-Fi a cada 5 minutos.
  - Se a ação subir: **LED Verde pisca** + comemoração 8-bit + olhos felizes.
  - Se a ação cair: **LED Vermelho pisca** + tom grave + olhos tristes com gota de suor.
- **🎼 Jukebox 8-Bits Chiptune com Animação Visual no OLED:**
  - 🏎️ **Ayrton Senna - Tema da Vitória (F1):** Tema épico completo com o **Logo oficial "S" do Ayrton Senna**, bandeira quadriculada, carro de F1 acelerando pela pista com zebras e **show de luzes Verde e Amarelo do Brasil**!
  - 🍄 **Super Mario Bros:** Tema completo com **animação do Mario no OLED** (Mario correndo pela fase World 1-1, pulo no bloco [?] e moeda saltando)!
  - 🪙 **Notificação Especial da Moeda do Mario (`coin` / `moeda`):** Card retrô NES no OLED com bloco `[?]`, salto parabólico de moeda brilhante rotativa, estrelas de brilho, pontuação flutuante `+200 PTS`, som icônico de moeda, piscada dourada no LED RGB on-board e contador acumulado de moedas!
  - ⚔️ **Star Wars - Marcha Imperial:** Tema completo com **logo Star Wars estilizado + Hyperspace** (campo estelar 3D na velocidade da luz)!
  - 🚀 **Star Wars - Tema Principal:** Tema completo com **logo Star Wars + Hyperspace**!
  - 🧱 **Tetris:** *Korobeiniki* completo!
  - Show de luzes sincronizado nos LEDs Verde, Amarelo e Vermelho durante a reprodução.
  - Sincronização visual em tempo real no simulador OLED e **Toast flutuante no Painel Web**!
- **🧠 Inteligência Artificial com Google Gemini:**
  - Responde a perguntas em português enviadas pelo Monitor Serial ou pelo Microfone do Painel Web.
  - Escolhe autonomamente a expressão e cor dos LEDs com base no sentimento da resposta.
- **🎙️ Painel Web Interativo (Dashboard):**
  - Hospedado localmente em `http://localhost:8080`.
  - Simulador físico do visor OLED e LEDs em tempo real.
  - Reconhecimento de voz em português via **Web Speech API**.
  - Diagrama de fiação interativo e sintetizador sonoro Web Audio API.

---

## 🔌 Diagrama de Montagem em Protoboard de 60 Linhas (830 Pontos)

Utilize uma protoboard padrão de **60 linhas (830 furos)** com canal central e barramentos laterais de alimentação.

### 📋 Mapeamento Linha a Linha:

| Faixa de Linhas | Componente | Conexões Físicas |
| :--- | :--- | :--- |
| **Linhas 1 a 22** | **ESP32-S3 DevKitC-1** | Encaixado no divisor central.<br>• **Linha 1 (Esq):** 3V3 ➔ Fio Vermelho para trilho **(+)**<br>• **Linha 3 (Esq):** GPIO 4 ➔ Linha 33 (LED Verde)<br>• **Linha 5 (Esq):** GPIO 6 ➔ Linha 38 (LED Vermelho)<br>• **Linha 7 (Esq):** GPIO 15 ➔ Linha 50 (Buzzer +)<br>• **Linha 11 (Esq):** GPIO 8 ➔ Linha 29 (OLED SDA)<br>• **Linha 14 (Esq):** GPIO 9 ➔ Linha 28 (OLED SCL)<br>• **Linha 21 (Esq):** GND ➔ Fio Preto para trilho **(-)**<br>• **Linha 4 (Dir):** GPIO 1 ➔ Linha 45 (Potenciômetro centro) |
| **Linhas 26 a 29** | **Tela OLED 0.96" SSD1306** | • **Linha 26:** VCC ➔ Trilho **(+)** (3.3V)<br>• **Linha 27:** GND ➔ Trilho **(-)**<br>• **Linha 28:** SCL ➔ Jumper Amarelo para Linha 14 (GPIO 9)<br>• **Linha 29:** SDA ➔ Jumper Azul para Linha 11 (GPIO 8) |
| **Linhas 33 a 35** | **LED Verde (Alta / Feliz)** | • **Linha 33:** Resistor 220Ω até Linha 34 + Jumper vindo da Linha 3 (GPIO 4)<br>• **Linha 34:** Perna longa (+ Ânodo) do LED<br>• **Linha 35:** Perna curta (- Cátodo) ➔ Trilho **(-)** |
| **Linhas 38 a 40** | **LED Vermelho (Queda / Triste)**| • **Linha 38:** Resistor 220Ω até Linha 39 + Jumper vindo da Linha 5 (GPIO 6)<br>• **Linha 39:** Perna longa (+ Ânodo) do LED<br>• **Linha 40:** Perna curta (- Cátodo) ➔ Trilho **(-)** |
| **Linhas 44 a 46** | **Potenciômetro 10k (Brilho)** | • **Linha 44:** Terminal 1 ➔ Trilho **(+)** (3.3V)<br>• **Linha 45:** Terminal Central ➔ Jumper Roxo para Linha 4 (GPIO 1)<br>• **Linha 46:** Terminal 3 ➔ Trilho **(-)** (GND) |
| **Linhas 50 a 52** | **Buzzer Piezoelétrico** | • **Linha 50:** Terminal (+) ➔ Jumper Branco para Linha 7 (GPIO 15)<br>• **Linha 52:** Terminal (-) ➔ Trilho **(-)** (GND) |
| **Linhas 53 a 60** | **Área Livre** | Espaço reservado para expansões futuras (ex: microfone I2S INMP441 ou servomotores). |

> ⚠️ **Aviso de Segurança (ESP32-S3 N16R8):** Os pinos GPIO 26 a 37 são conectados internamente à memória PSRAM Octal de 8MB e Flash de 16MB. Mantenha-os sempre livres.

---

## 🖨️ Gabinete e Modelagem 3D

Na raiz do projeto estão incluídos os arquivos 3D paramétricos para impressão:
- `gabinete_robo.scad` / `gabinete.stl`: Chassi com encaixe para a tela OLED 0.96", bochechas de LED 5mm, abertura acústica para o buzzer e porta traseira USB-C.
- Modelos retrô adicionais: `pc_retro_win95.scad`, `monitor_crt.scad`, `teclado_base_esp32.scad`.

---

## 💻 Como Compilar e Gravar

O projeto utiliza o **PlatformIO** integrado ao VS Code:

1. Clone o repositório:
   ```bash
   git clone https://github.com/hugozumkerberg-ux/dollynho-toy.git
   cd dollynho-toy
   ```
2. Abra a pasta no VS Code com a extensão do PlatformIO instalada.
3. Conecte o ESP32-S3 na porta USB.
4. Para compilar e fazer upload direto:
   ```bash
   pio run -t upload
   ```
5. Para iniciar o painel web local:
   ```bash
   node server.js
   ```
   Acesse no navegador: **`http://localhost:8080`**

---

## 🕹️ Comandos via Monitor Serial (115200 baud)

- `imperial` ou `vader`: Toca a Marcha Imperial Completa do Star Wars!
- `starwars` ou `sw`: Toca o Tema Principal Completo do Star Wars!
- `mario`: Toca Super Mario Bros Completo!
- `senna`, `vitoria` ou `f1`: Toca o Tema da Vitória do Ayrton Senna com Logo Senna no OLED!
- `coin` ou `moeda`: Dispara a Notificação da Moeda do Mario no OLED (+200 PTS)!
- `tetris`: Toca Tetris Completo!
- `bbas3`: Atualiza a cotação da BBAS3 na B3 e reage com LEDs e som.
- `laugh`: Executa a animação de gargalhada do RoboEyes.
- `confused`: Executa a animação de dúvida/confusão do RoboEyes.
- `happy` / `angry` / `tired` / `default`: Altera o humor dos olhos.
- *Qualquer texto*: Envia a mensagem diretamente para o **Google Gemini AI**!

---

Desenvolvido com carinho para o seu assistente de bancada definitivo! 🤖✨

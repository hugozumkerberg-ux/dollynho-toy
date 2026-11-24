#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include <time.h>
#include "config_robot.h"
#include "pitches.h"
#include "songs_8bit.h"
#include "graphics_oled.h"

// Desativa macros conflitantes do RoboEyes com templates
#ifdef N
#undef N
#endif
#ifdef S
#undef S
#endif
#ifdef E
#undef E
#endif
#ifdef W
#undef W
#endif
#ifdef DEFAULT
#undef DEFAULT
#define DEFAULT 0
#endif

// Display OLED SSD1306
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Inicialização do FluxGarage RoboEyes
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// Conexão Wi-Fi Multi (Hugo / Apto 1412) e Servidor Web Local
WiFiMulti wifiMulti;
WebServer localServer(80);

// LED RGB On-board do ESP32-S3 (GPIO 48 no DevKitC-1)
#ifndef RGB_BUILTIN
#define RGB_BUILTIN 48
#endif

// Estados de Exibição e Humor
bool showingText = false;
unsigned long textStartTime = 0;
unsigned long textDuration = 6000;
String currentMoodStr = "DEFAULT";

// Declarações antecipadas de funções
void executeRobotCommand(String cmd);
void checkRemoteCloudApi();

// Variáveis de Cotação da B3 (BBAS3)
float bbas3Price = 28.45;
float bbas3Change = 1.82;
unsigned long lastStockCheckTime = 0;

// Variáveis e Controle de Horário e Status Bar OLED
char manualTimeStr[10] = "";
bool hasManualTime = false;
unsigned long lastStatusBarTime = 0;

const char* getTimeString() {
    static char buf[10] = "12:00";
    static unsigned long lastCheck = 0;
    static bool ntpSynced = false;
    unsigned long now = millis();
    if (now - lastCheck < 1000 && lastCheck != 0) {
        return buf;
    }
    lastCheck = now;

    struct tm timeinfo;
    // O RTC interno do ESP32 mantem a hora precisa com 0 delay apos a sincronizacao
    if (getLocalTime(&timeinfo, 0)) {
        if (timeinfo.tm_year > (2020 - 1900)) {
            ntpSynced = true;
            strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
            return buf;
        }
    }
    if (hasManualTime && strlen(manualTimeStr) >= 5) {
        return manualTimeStr;
    }
    // Fallback: calcula minutos desde o boot
    unsigned long totalSec = millis() / 1000;
    unsigned long min = (totalSec / 60) % 60;
    unsigned long hr = (totalSec / 3600) % 24;
    snprintf(buf, sizeof(buf), "%02lu:%02lu", hr, min);
    return buf;
}

// Ajuste fino dos LEDs (externos e interno do ESP32)
void setLedsColor(uint8_t r, uint8_t g, uint8_t b, uint8_t scale = 255) {
    analogWrite(PIN_LED_VERMELHO, (r * scale) / 255);
    analogWrite(PIN_LED_AMARELO,  (((g > 0 && r > 0 ? 180 : 0)) * scale) / 255);
    analogWrite(PIN_LED_VERDE,    (g * scale) / 255);
    // Ativa o LED RGB On-board do ESP32-S3 (GPIO 48)
    rgbLedWrite(RGB_BUILTIN, (r * scale) / 255, (g * scale) / 255, (b * scale) / 255);
}

// Desliga todos os LEDs da placa e externos
void ledsOff() {
    setLedsColor(0, 0, 0, 0);
    analogWrite(PIN_LED_VERDE, 0);
    analogWrite(PIN_LED_AMARELO, 0);
    analogWrite(PIN_LED_VERMELHO, 0);
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
}

// ----------------------------------------------------
// Controle de Áudio Estéreo / Polifônico (Dual Buzzer LEDC Hardware)
// Buzzer 1 = GPIO 15 (Melodia Principal / Canal Esquerdo)
// Buzzer 2 = GPIO 16 (Harmonia / Baixo / Canal Direito)
// ----------------------------------------------------
void buzzerTone1(unsigned int freq) {
    if (freq > 0) {
        ledcWriteTone(PIN_BUZZER_1, freq);
    } else {
        ledcWrite(PIN_BUZZER_1, 0);
    }
}

void buzzerTone2(unsigned int freq) {
    if (freq > 0) {
        ledcWriteTone(PIN_BUZZER_2, freq);
    } else {
        ledcWrite(PIN_BUZZER_2, 0);
    }
}

// Toca frequências distintas simultaneamente (Polifonia real a 2 vozes!)
void buzzerToneDual(unsigned int freq1, unsigned int freq2) {
    buzzerTone1(freq1);
    buzzerTone2(freq2);
}

// Toca nos dois buzzers em fase (Modo Volume Duplo Boost)
void buzzerTone(unsigned int freq) {
    buzzerTone1(freq);
    buzzerTone2(freq);
}

void buzzerNoTone1() {
    ledcWrite(PIN_BUZZER_1, 0);
}

void buzzerNoTone2() {
    ledcWrite(PIN_BUZZER_2, 0);
}

void buzzerNoTone() {
    buzzerNoTone1();
    buzzerNoTone2();
}

// 3 Piscadas Verdes: Wi-Fi Conectado com sucesso
void blinkWifiConnected() {
    Serial.println("[LED ESP32] 3 Piscadas Verdes: Wi-Fi Conectado!");
    for (int i = 0; i < 3; i++) {
        rgbLedWrite(RGB_BUILTIN, 0, 220, 0); // Verde forte no ESP32
        analogWrite(PIN_LED_VERDE, 255);       // Verde externo
        delay(180);
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
        analogWrite(PIN_LED_VERDE, 0);
        delay(180);
    }
}

// 1 Piscada Azul: Resposta da API IA recebida com sucesso (OK)
void blinkApiOk() {
    Serial.println("[LED ESP32] 1 Piscada Azul: API Gemini IA OK!");
    rgbLedWrite(RGB_BUILTIN, 0, 0, 255); // Azul forte no ESP32
    delay(450);
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
}

void flashLeds(int count, int delayMs, bool isGreen) {
    for (int i = 0; i < count; i++) {
        analogWrite(isGreen ? PIN_LED_VERDE : PIN_LED_VERMELHO, 255);
        analogWrite(isGreen ? PIN_LED_VERMELHO : PIN_LED_VERDE, 0);
        rgbLedWrite(RGB_BUILTIN, isGreen ? 0 : 255, isGreen ? 255 : 0, 0);
        delay(delayMs);
        analogWrite(PIN_LED_VERDE, 0);
        analogWrite(PIN_LED_VERMELHO, 0);
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
        delay(delayMs);
    }
}

// Modos de Exibição Visual durante a Música
enum VisualSongTheme {
    THEME_NONE,
    THEME_MARIO,
    THEME_STAR_WARS_IMPERIAL,
    THEME_STAR_WARS_MAIN,
    THEME_SENNA,
    THEME_PACMAN,
    THEME_TOP_GUN,
    THEME_ARCADE_SPACE,
    THEME_80S_SYNTH
};

// ----------------------------------------------------
// Tocador de Músicas 8-Bits com Animação Visual no OLED
// ----------------------------------------------------
void playSong(const Note* song, int length, VisualSongTheme theme = THEME_NONE) {
    showingText = true; // Pausa o RoboEyes para exibir a cena visual da música
    for (int i = 0; i < length; i++) {
        Note n;
        memcpy_P(&n, &song[i], sizeof(Note));

        int pitch = n.pitch;
        int duration = n.durationMs;

        // Leitura dinâmica do potenciômetro para brilho durante a música
        int potValue = analogRead(PIN_POTENCIOMETRO);
        uint8_t brightness = map(potValue, 0, 4095, 40, 255);

        // Atualiza a cena visual no OLED a cada nota
        if (theme == THEME_MARIO) {
            drawMarioScene(i, display);
        } else if (theme == THEME_STAR_WARS_IMPERIAL) {
            drawStarWarsScene(i, "IMPERIAL MARCH", display);
        } else if (theme == THEME_STAR_WARS_MAIN) {
            drawStarWarsScene(i, "MAIN THEME", display);
        } else if (theme == THEME_SENNA) {
            drawSennaScene(i, display);
        } else if (theme == THEME_PACMAN) {
            drawPacmanScene(i, display);
        } else if (theme == THEME_TOP_GUN) {
            drawTopGunScene(i, display);
        } else if (theme == THEME_ARCADE_SPACE) {
            drawArcadeSpaceScene(i, display);
        } else if (theme == THEME_80S_SYNTH) {
            drawTakeOnMeScene(i, display);
        }

        if (pitch > 0) {
            buzzerTone(pitch);

            if (theme == THEME_SENNA) {
                // Alterna LEDs em Verde e Amarelo (Cores do Brasil e do Capacete do Ayrton Senna!)
                if (i % 2 == 0) {
                    analogWrite(PIN_LED_VERDE, brightness);
                    analogWrite(PIN_LED_AMARELO, 0);
                    analogWrite(PIN_LED_VERMELHO, 0);
                    rgbLedWrite(RGB_BUILTIN, 0, 255, 30); // Verde Esmeralda
                } else {
                    analogWrite(PIN_LED_VERDE, (brightness * 160) / 255);
                    analogWrite(PIN_LED_AMARELO, brightness);
                    analogWrite(PIN_LED_VERMELHO, (brightness * 40) / 255);
                    rgbLedWrite(RGB_BUILTIN, 255, 215, 0); // Amarelo Dourado
                }
            } else if (theme == THEME_PACMAN) {
                // Amarelo Pac-Man e Vermelho Fantasma Blinky
                if (i % 2 == 0) {
                    analogWrite(PIN_LED_AMARELO, brightness);
                    analogWrite(PIN_LED_VERMELHO, 0);
                    analogWrite(PIN_LED_VERDE, (brightness * 140) / 255);
                    rgbLedWrite(RGB_BUILTIN, 255, 220, 0);
                } else {
                    analogWrite(PIN_LED_VERMELHO, brightness);
                    analogWrite(PIN_LED_AMARELO, 0);
                    analogWrite(PIN_LED_VERDE, 0);
                    rgbLedWrite(RGB_BUILTIN, 255, 20, 20);
                }
            } else if (theme == THEME_TOP_GUN) {
                // Caça supersônico: Vermelho e Azul de pós-combustão
                if (i % 2 == 0) {
                    analogWrite(PIN_LED_VERMELHO, brightness);
                    analogWrite(PIN_LED_AMARELO, (brightness * 50) / 255);
                    analogWrite(PIN_LED_VERDE, 0);
                    rgbLedWrite(RGB_BUILTIN, 255, 60, 0);
                } else {
                    analogWrite(PIN_LED_VERMELHO, 0);
                    analogWrite(PIN_LED_AMARELO, 0);
                    analogWrite(PIN_LED_VERDE, (brightness * 80) / 255);
                    rgbLedWrite(RGB_BUILTIN, 0, 180, 255);
                }
            } else if (theme == THEME_ARCADE_SPACE) {
                // Verde arcade e ciano espacial
                if (i % 2 == 0) {
                    analogWrite(PIN_LED_VERDE, brightness);
                    analogWrite(PIN_LED_AMARELO, 0);
                    analogWrite(PIN_LED_VERMELHO, 0);
                    rgbLedWrite(RGB_BUILTIN, 0, 255, 120);
                } else {
                    analogWrite(PIN_LED_VERDE, (brightness * 180) / 255);
                    analogWrite(PIN_LED_AMARELO, 0);
                    analogWrite(PIN_LED_VERMELHO, (brightness * 80) / 255);
                    rgbLedWrite(RGB_BUILTIN, 180, 0, 255);
                }
            } else if (theme == THEME_80S_SYNTH) {
                // Neon Anos 80 (Magenta e Dourado)
                if (i % 2 == 0) {
                    analogWrite(PIN_LED_VERMELHO, brightness);
                    analogWrite(PIN_LED_AMARELO, 0);
                    analogWrite(PIN_LED_VERDE, 0);
                    rgbLedWrite(RGB_BUILTIN, 255, 0, 180);
                } else {
                    analogWrite(PIN_LED_AMARELO, brightness);
                    analogWrite(PIN_LED_VERDE, (brightness * 120) / 255);
                    analogWrite(PIN_LED_VERMELHO, (brightness * 60) / 255);
                    rgbLedWrite(RGB_BUILTIN, 255, 230, 0);
                }
            } else if (pitch > 550) {
                setLedsColor(0, 255, 0, brightness);
            } else {
                setLedsColor(255, 0, 0, brightness);
            }
        } else {
            buzzerNoTone();
            ledsOff();
        }

        int elapsed = 0;
        while (elapsed < duration) {
            int step = min(25, duration - elapsed);
            delay(step);
            elapsed += step;
            if (theme == THEME_NONE) {
                roboEyes.update();
            }
        }

        buzzerNoTone();
        ledsOff();
        delay(25); // Breve pausa entre as notas
    }
    buzzerNoTone();
    ledsOff(); // Garante que tudo apaga ao final da música
    showingText = false; // Retorna aos olhinhos orgânicos do RoboEyes
    roboEyes.update();
}

void playLaserSfx() {
    // Efeito Laser Estéreo: o som se divide entre o Buzzer 1 (agudo) e o Buzzer 2 (grave harmônico)
    for (int f = 1600; f >= 200; f -= 75) {
        buzzerToneDual(f, f / 2);
        delay(6);
    }
    buzzerNoTone();
    ledsOff();
}

void playStereoLaserSfx() {
    // Laser Ping-Pong Estéreo: Dispara no Buzzer 1 e ricocheteia no Buzzer 2
    for (int f = 1800; f >= 400; f -= 90) {
        buzzerTone1(f);
        delay(6);
    }
    buzzerNoTone1();
    for (int f = 1200; f >= 250; f -= 80) {
        buzzerTone2(f);
        delay(7);
    }
    buzzerNoTone2();
    ledsOff();
}

void playDualChordSfx() {
    // Acorde Polifônico Triunfante a 2 Vozes (Buzzer 1 + Buzzer 2 simultâneos)
    // Dó4 + Sol4 (Quinta Perfeita)
    buzzerToneDual(NOTE_C4, NOTE_G4);
    analogWrite(PIN_LED_VERMELHO, 150);
    delay(140);
    // Mi4 + Si4 (Terça Maior harmônica)
    buzzerToneDual(NOTE_E4, NOTE_B4);
    analogWrite(PIN_LED_AMARELO, 200);
    delay(140);
    // Sol4 + Ré5
    buzzerToneDual(NOTE_G4, NOTE_D5);
    analogWrite(PIN_LED_VERDE, 200);
    delay(140);
    // Dó5 + Sol5 (Clímax brilhante duplo)
    buzzerToneDual(NOTE_C5, NOTE_G5);
    rgbLedWrite(RGB_BUILTIN, 0, 255, 255);
    delay(350);
    buzzerNoTone();
    ledsOff();
}

void play1UpSfx() {
    int notes1[] = {NOTE_E5, NOTE_G5, NOTE_E6, NOTE_C6, NOTE_D6, NOTE_G6};
    int notes2[] = {NOTE_C5, NOTE_E5, NOTE_C6, NOTE_G5, NOTE_B5, NOTE_D6}; // Harmonia em terças no Buzzer 2!
    int durs[]   = {100,     100,     100,     100,     100,     250};
    for (int i = 0; i < 6; i++) {
        buzzerToneDual(notes1[i], notes2[i]);
        setLedsColor(0, 255, 0, 200);
        delay(durs[i]);
        buzzerNoTone();
        ledsOff();
        delay(20);
    }
}

int totalCoinsCollected = 0;

void notifyMarioCoin() {
    totalCoinsCollected++;
    showingText = true;
    textStartTime = millis();
    textDuration = 2200; // Mantém a notificação na tela por 2.2 segundos

    Serial.printf("[Mario] 🪙 Moeda Coletada! Total: %d (+200 PTS)\n", totalCoinsCollected);
    roboEyes.setMood(HAPPY);

    // Primeiro tom: Si5 (B5) no Buzzer 1 + Sol#5 (GS5) no Buzzer 2
    buzzerToneDual(NOTE_B5, NOTE_GS5);
    analogWrite(PIN_LED_VERDE, 255);
    rgbLedWrite(RGB_BUILTIN, 255, 215, 0); // Dourado brilhante no ESP32

    for (int f = 0; f < 8; f++) {
        drawMarioCoinNotification(f, totalCoinsCollected, display);
        delay(28);
    }

    // Segundo tom: Mi6 (E6) no Buzzer 1 + Si5 (B5) no Buzzer 2 (Acorde completo brilhante!)
    buzzerToneDual(NOTE_E6, NOTE_B5);
    analogWrite(PIN_LED_VERDE, 200);
    rgbLedWrite(RGB_BUILTIN, 50, 255, 100);

    for (int f = 8; f <= 15; f++) {
        drawMarioCoinNotification(f, totalCoinsCollected, display);
        delay(28);
    }

    buzzerNoTone();
    ledsOff();
}

void play8BitCoin() {
    notifyMarioCoin();
}

// ----------------------------------------------------
// Cotação da B3 (BBAS3)
// ----------------------------------------------------
void drawStockCard() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 2);
    display.println("--- B3 / BBAS3 ---");
    
    display.setTextSize(2);
    display.setCursor(12, 16);
    display.println("BBAS3");

    display.setTextSize(1);
    display.setCursor(12, 36);
    display.printf("Cotacao: R$ %.2f\n", bbas3Price);
    display.setCursor(12, 48);
    if (bbas3Change >= 0) {
        display.printf("Hoje: +%.2f%% [ALTA]", bbas3Change);
    } else {
        display.printf("Hoje: %.2f%% [BAIXA]", bbas3Change);
    }
    display.display();
}

void updateStockBBAS3() {
    Serial.println("\n[B3] Consultando cotação da BBAS3...");
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient https;
        if (https.begin(client, "https://brapi.dev/api/quote/BBAS3")) {
            int httpCode = https.GET();
            if (httpCode == HTTP_CODE_OK) {
                String payload = https.getString();
                JsonDocument doc;
                if (!deserializeJson(doc, payload)) {
                    bbas3Price = doc["results"][0]["regularMarketPrice"] | bbas3Price;
                    bbas3Change = doc["results"][0]["regularMarketChangePercent"] | bbas3Change;
                    Serial.printf("[B3] BBAS3 Atualizada: R$ %.2f (%.2f%%)\n", bbas3Price, bbas3Change);
                }
            }
            https.end();
        }
    }

    if (bbas3Change >= 0) {
        roboEyes.setMood(HAPPY);
        roboEyes.anim_laugh();
        play8BitCoin();
    } else {
        roboEyes.setMood(TIRED);
        roboEyes.setSweat(ON);
        buzzerTone(NOTE_FS4);
        delay(300);
        buzzerNoTone();
        ledsOff();
    }

    showingText = true;
    textStartTime = millis();
    textDuration = 5000;
    drawStockCard();
}

// ----------------------------------------------------
// Google Gemini AI
// ----------------------------------------------------
void askGemini(String userQuery) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERRO] Wi-Fi desconectado!");
        return;
    }

    Serial.println("\n[Gemini] Enviando pergunta: " + userQuery);
    
    roboEyes.anim_confused();
    buzzerTone(NOTE_A5);
    delay(60);
    buzzerNoTone();

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    String url = "https://generativelanguage.googleapis.com/v1beta/models/" + String(GEMINI_MODEL) + ":generateContent";

    if (https.begin(client, url)) {
        https.addHeader("Content-Type", "application/json");
        https.addHeader("x-goog-api-key", GEMINI_API_KEY);

        JsonDocument docReq;
        JsonObject contents = docReq["contents"].to<JsonArray>().add<JsonObject>();
        JsonObject parts = contents["parts"].to<JsonArray>().add<JsonObject>();
        
        String systemPrompt = "Voce e o robo assistente 'Dollynho Toy'. Responda em portugues em ate 14 palavras. "
                              "No final inclua obrigatoriamente um humor: [HAPPY], [ANGRY], [TIRED] ou [DEFAULT]. "
                              "Mensagem: " + userQuery;
        parts["text"] = systemPrompt;

        String reqBody;
        serializeJson(docReq, reqBody);

        int httpCode = https.POST(reqBody);
        if (httpCode == HTTP_CODE_OK) {
            String respStr = https.getString();
            JsonDocument docResp;
            if (!deserializeJson(docResp, respStr)) {
                const char* replyText = docResp["candidates"][0]["content"]["parts"][0]["text"];
                if (replyText) {
                    String fullReply = String(replyText);
                    fullReply.trim();
                    Serial.println("\n[Dollynho]: " + fullReply);

                    if (fullReply.indexOf("[ANGRY]") >= 0) {
                        currentMoodStr = "ANGRY";
                        roboEyes.setMood(ANGRY);
                        roboEyes.setSweat(OFF);
                        fullReply.replace("[ANGRY]", "");
                        buzzerTone(135); delay(100); buzzerTone(90); delay(130); buzzerNoTone();
                    } else if (fullReply.indexOf("[TIRED]") >= 0) {
                        currentMoodStr = "TIRED";
                        roboEyes.setMood(TIRED);
                        fullReply.replace("[TIRED]", "");
                        buzzerTone(NOTE_FS4); delay(250); buzzerNoTone();
                        ledsOff();
                    } else if (fullReply.indexOf("[HAPPY]") >= 0) {
                        currentMoodStr = "HAPPY";
                        roboEyes.setMood(HAPPY);
                        roboEyes.anim_laugh();
                        fullReply.replace("[HAPPY]", "");
                        play8BitCoin();
                    } else {
                        currentMoodStr = "DEFAULT";
                        roboEyes.setMood(DEFAULT);
                        fullReply.replace("[DEFAULT]", "");
                        play8BitCoin();
                    }

                    fullReply.trim();

                    display.clearDisplay();
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 0);
                    display.println("--- Dollynho diz: ---");
                    display.println();
                    display.println(fullReply);
                    display.display();

                    showingText = true;
                    textStartTime = millis();
                    textDuration = 6000;
                }
            }
        }
        https.end();
    }
}

// ----------------------------------------------------
// Despachante Unificado de Comandos do Robô
// Executa comandos vindos do Web Serial USB, Servidor Web Local e Nuvem Hostinger
// ----------------------------------------------------
void executeRobotCommand(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) return;

    Serial.println("[Dollynho] Processando comando: " + cmd);

    if (cmd.startsWith("time:")) {
        String t = cmd.substring(5);
        t.trim();
        if (t.length() >= 5) {
            strncpy(manualTimeStr, t.c_str(), sizeof(manualTimeStr) - 1);
            hasManualTime = true;
            Serial.printf("[Relógio] Hora atualizada: %s\n", manualTimeStr);
        }
    }
    // -------------------------------------------------------------------------
    // 1. EXPRESSÕES FACIAIS E HUMORES (Sem músicas longas bloqueantes!)
    // -------------------------------------------------------------------------
    else if (cmd == "1" || cmd == "happy" || cmd == "feliz") {
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        roboEyes.setSweat(OFF);
        roboEyes.setCuriosity(ON);
        play8BitCoin();
    }
    else if (cmd == "2" || cmd == "angry" || cmd == "bravo") {
        currentMoodStr = "ANGRY";
        roboEyes.setMood(ANGRY);
        roboEyes.setSweat(OFF);
        roboEyes.setCuriosity(OFF);
        // Rosnado rápido de robô irritado (apenas 220ms, NUNCA toca a Marcha Imperial!)
        buzzerTone(140);
        analogWrite(PIN_LED_VERMELHO, 255);
        delay(100);
        buzzerTone(90);
        delay(120);
        buzzerNoTone();
        analogWrite(PIN_LED_VERMELHO, 0);
    }
    else if (cmd == "3" || cmd == "tired" || cmd == "sleepy" || cmd == "sono" || cmd == "cansado") {
        currentMoodStr = "TIRED";
        roboEyes.setMood(TIRED);
        roboEyes.setSweat(OFF);
        roboEyes.setCuriosity(OFF);
        buzzerTone(NOTE_FS4);
        delay(200);
        buzzerNoTone();
        ledsOff();
    }
    else if (cmd == "sad" || cmd == "triste") {
        currentMoodStr = "TIRED";
        roboEyes.setMood(TIRED);
        roboEyes.setSweat(ON); // Ativa gota de suor/lágrima animada na testa!
        buzzerTone(NOTE_D4);
        analogWrite(PIN_LED_VERMELHO, 120);
        delay(140);
        buzzerTone(NOTE_C4);
        delay(220);
        buzzerNoTone();
        ledsOff();
    }
    else if (cmd == "surprised" || cmd == "alerta" || cmd == "surpreso") {
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
        roboEyes.setSweat(OFF);
        roboEyes.setCuriosity(ON);
        roboEyes.anim_confused();
        buzzerTone(NOTE_E6);
        analogWrite(PIN_LED_AMARELO, 255);
        delay(90);
        buzzerNoTone();
        ledsOff();
    }
    else if (cmd == "love" || cmd == "amor") {
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        roboEyes.setSweat(OFF);
        roboEyes.anim_laugh();
        analogWrite(PIN_LED_VERMELHO, 255);
        analogWrite(PIN_LED_VERDE, 70);
        buzzerTone(NOTE_C6);
        delay(90);
        buzzerTone(NOTE_G6);
        delay(160);
        buzzerNoTone();
        ledsOff();
    }
    else if (cmd == "focus" || cmd == "foco") {
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
        roboEyes.setSweat(OFF);
        roboEyes.setCuriosity(OFF);
        buzzerTone(NOTE_A5);
        delay(60);
        buzzerNoTone();
        ledsOff();
    }
    else if (cmd == "blink" || cmd == "piscar") {
        roboEyes.blink();
    }
    else if (cmd == "laugh" || cmd == "risada") {
        currentMoodStr = "HAPPY";
        roboEyes.anim_laugh();
        play8BitCoin();
    }
    else if (cmd == "confused" || cmd == "confuso") {
        roboEyes.anim_confused();
        buzzerTone(NOTE_A5);
        delay(80);
        buzzerNoTone();
        ledsOff();
    }
    else if (cmd == "4" || cmd == "default" || cmd == "normal") {
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
        roboEyes.setSweat(OFF);
        roboEyes.setCuriosity(ON);
        ledsOff();
    }
    else if (cmd == "bbas3" || cmd == "bolsa") {
        updateStockBBAS3();
    }
    else if (cmd == "coin" || cmd == "moeda") {
        notifyMarioCoin();
    }
    // -------------------------------------------------------------------------
    // 2. MÚSICAS COMPLETAS E RETRÔ 8-BITS (Somente quando o usuário pede música!)
    // -------------------------------------------------------------------------
    else if (cmd == "starwars" || cmd == "sw" || cmd == "starwars_main") {
        Serial.println("[Música] Tocando Star Wars: Tema Principal + Logo no OLED!");
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        playSong(SONG_STAR_WARS_MAIN, LEN_STAR_WARS_MAIN, THEME_STAR_WARS_MAIN);
    }
    else if (cmd == "imperial" || cmd == "vader") {
        Serial.println("[Música] Tocando Star Wars: Marcha Imperial + Logo no OLED!");
        currentMoodStr = "ANGRY";
        roboEyes.setMood(ANGRY);
        playSong(SONG_IMPERIAL_MARCH, LEN_IMPERIAL_MARCH, THEME_STAR_WARS_IMPERIAL);
    }
    else if (cmd == "mario") {
        Serial.println("[Música] Tocando Super Mario Bros + Animação no OLED!");
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        playSong(SONG_MARIO_FULL, LEN_MARIO_FULL, THEME_MARIO);
    }
    else if (cmd == "tetris") {
        Serial.println("[Música] Tocando Tetris Korobeiniki Completo!");
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
        playSong(SONG_TETRIS_FULL, LEN_TETRIS_FULL, THEME_NONE);
    }
    else if (cmd == "senna" || cmd == "vitoria" || cmd == "f1") {
        Serial.println("[Música] 🏎️ Ayrton Senna: Tema da Vitória + Logo Senna no OLED!");
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        roboEyes.anim_laugh();
        playSong(SONG_SENNA_VITORIA, LEN_SENNA_VITORIA, THEME_SENNA);
    }
    else if (cmd == "pacman" || cmd == "pac") {
        Serial.println("[Música] 🟡 Pac-Man Arcade 1980 + Labirinto no OLED!");
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        playSong(SONG_PACMAN_FULL, LEN_PACMAN_FULL, THEME_PACMAN);
    }
    else if (cmd == "topgun" || cmd == "dangerzone" || cmd == "f14") {
        Serial.println("[Música] ✈️ Top Gun: Danger Zone + Caça F-14 Tomcat no OLED!");
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
        playSong(SONG_TOP_GUN, LEN_TOP_GUN, THEME_TOP_GUN);
    }
    else if (cmd == "space" || cmd == "invaders" || cmd == "arcade" || cmd == "escape") {
        Serial.println("[Música] 👾 Space Invaders Arcade Escape + Batalha Espacial no OLED!");
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
        playSong(SONG_ARCADE_SPACE, LEN_ARCADE_SPACE, THEME_ARCADE_SPACE);
    }
    else if (cmd == "takeonme" || cmd == "80s" || cmd == "anos80" || cmd == "aha") {
        Serial.println("[Música] 📼 Anos 80: Take On Me (A-ha) + Synthwave Grid no OLED!");
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        roboEyes.anim_laugh();
        playSong(SONG_TAKE_ON_ME, LEN_TAKE_ON_ME, THEME_80S_SYNTH);
    }
    else if (cmd == "laser") {
        Serial.println("[Efeito] ⚡ Disparo Laser 8-Bit!");
        playLaserSfx();
    }
    else if (cmd == "1up") {
        Serial.println("[Efeito] ⭐ Vida Extra 1-Up Arcade!");
        play1UpSfx();
    }
    else if (cmd == "dualbeep" || cmd == "chord" || cmd == "acorde") {
        Serial.println("[Áudio] 🎶 Tocando Acorde Polifônico Dual Buzzer (GPIO 15 + GPIO 16)!");
        playDualChordSfx();
    }
    else if (cmd == "stereo" || cmd == "estereo") {
        Serial.println("[Áudio] ⚡ Disparo Laser Estéreo (Buzzer 1 ➔ Buzzer 2)!");
        playStereoLaserSfx();
    }
    else if (cmd == "buzzer1") {
        Serial.println("[Áudio] Testando Buzzer 1 (GPIO 15)...");
        buzzerTone1(NOTE_A4);
        delay(280);
        buzzerNoTone1();
    }
    else if (cmd == "buzzer2") {
        Serial.println("[Áudio] Testando Buzzer 2 (GPIO 16)...");
        buzzerTone2(NOTE_E5);
        delay(280);
        buzzerNoTone2();
    }
    else {
        askGemini(cmd);
    }
}

// ----------------------------------------------------
// Tarefa FreeRTOS no Core 0: Polling da Fila Remota na Nuvem Hostinger
// Roda em segundo plano no Core 0 sem nunca travar os olhos/FPS no Core 1!
// ----------------------------------------------------
static volatile bool pendingCloudCmd = false;
static char cloudCmdBuffer[64] = "";
static portMUX_TYPE cloudMux = portMUX_INITIALIZER_UNLOCKED;

void cloudPollingTask(void* pvParameters) {
    Serial.println("[FreeRTOS Core 0] Polling de Nuvem iniciado com sucesso!");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1500)); // Espera 1.5s entre requisições

        if (WiFi.status() == WL_CONNECTED && !showingText) {
            WiFiClientSecure client;
            client.setInsecure();
            HTTPClient https;
            https.setTimeout(2200);

            String url = String(CLOUD_API_URL) + "?action=poll&auto_ack=1&ip=" + WiFi.localIP().toString() +
                         "&rssi=" + String(WiFi.RSSI()) + "&mood=" + currentMoodStr;

            if (https.begin(client, url)) {
                int httpCode = https.GET();
                if (httpCode == HTTP_CODE_OK) {
                    String payload = https.getString();
                    JsonDocument doc;
                    DeserializationError err = deserializeJson(doc, payload);
                    if (!err) {
                        const char* cmd = doc["cmd"];
                        if (cmd && strlen(cmd) > 0) {
                            portENTER_CRITICAL(&cloudMux);
                            strncpy(cloudCmdBuffer, cmd, sizeof(cloudCmdBuffer) - 1);
                            cloudCmdBuffer[sizeof(cloudCmdBuffer) - 1] = '\0';
                            pendingCloudCmd = true;
                            portEXIT_CRITICAL(&cloudMux);
                        }
                    }
                }
                https.end();
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0); // Não bloqueia quando rodando sem cabo USB (carregador/bateria)
    delay(400);
    Serial.println("\n==============================================");
    Serial.println("  🤖 Dollynho Toy: Músicas 8-Bits + Nuvem Hostinger ");
    Serial.println("==============================================");

    pinMode(PIN_LED_VERDE, OUTPUT);
    pinMode(PIN_LED_AMARELO, OUTPUT);
    pinMode(PIN_LED_VERMELHO, OUTPUT);
    pinMode(PIN_POTENCIOMETRO, INPUT);

    // Desliga IMEDIATAMENTE os LEDs da placa e externos
    ledsOff();

    // Inicialização direta dos 2 buzzers (GPIO 15 e GPIO 16) via hardware LEDC (Áudio Estéreo/Polifonia)
    ledcAttach(PIN_BUZZER_1, 2000, 10);
    ledcAttach(PIN_BUZZER_2, 2000, 10);
    buzzerNoTone();

    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    Wire.setClock(400000); // Fast Mode I2C 400kHz (4x mais rápido que o padrão 100kHz)

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    }

    // Aumenta a taxa de oscilação interna do painel SSD1306 para eliminar flicker de varredura
    display.ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    display.ssd1306_command(0xF0); // 0xF0 = Máxima frequência do oscilador interno (~160Hz)

    // Inicialização do RoboEyes a 30 FPS estáveis (33ms por quadro, tempo ideal para I2C a 400kHz)
    roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 30);
    roboEyes.setWidth(34, 34);
    roboEyes.setHeight(30, 30);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setSpacebetween(10);
    roboEyes.setCuriosity(ON);
    roboEyes.setAutoblinker(ON, 3, 2);
    roboEyes.setIdleMode(ON, 2, 2);

    // Sincronização de horário NTP com fuso horário de Brasília (UTC-3)
    configTime(-3 * 3600, 0, "pool.ntp.org", "time.google.com");

    // Modo Wi-Fi Híbrido: Conecta no Roteador (STA) e cria Rede Própria (AP)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Dollynho-Toy", "12345678");
    Serial.println("[Wi-Fi AP] Ponto de Acesso criado: 'Dollynho-Toy' (Senha: 12345678)");
    Serial.println("[Wi-Fi AP] IP direto do Robô: " + WiFi.softAPIP().toString());

    // Wi-Fi Multi: Conecta na rede disponível ("Hugo" ou "Apto 1412")
    wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
    wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

    Serial.println("[Wi-Fi Multi] Conectando a rede configurada...");
    int attempts = 0;
    while (wifiMulti.run() != WL_CONNECTED && attempts < 15) {
        delay(350);
        attempts++;
        roboEyes.update();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[Wi-Fi Local] Conectado à rede '" + WiFi.SSID() + "'! IP: " + WiFi.localIP().toString());
        currentMoodStr = "HAPPY";
        roboEyes.setMood(HAPPY);
        blinkWifiConnected();
    } else {
        Serial.println("[Wi-Fi Local] Modo Offline (Use o Wi-Fi direto 'Dollynho-Toy')");
        currentMoodStr = "DEFAULT";
        roboEyes.setMood(DEFAULT);
    }

    // Configura Servidor Web Local (Porta 80)
    localServer.on("/", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Dollynho Toy</title></head>"
                      "<body style='font-family:sans-serif;text-align:center;padding:30px;background:#0a0f1d;color:#fff;'>"
                      "<h1>🤖 Dollynho Toy Online!</h1>"
                      "<p>IP Local: " + WiFi.localIP().toString() + "</p>"
                      "<p>IP Direto (AP): " + WiFi.softAPIP().toString() + "</p>"
                      "<p>Painel Web na Nuvem: <a style='color:#38bdf8;' href='https://bazardasnovidades.com/robo/'>bazardasnovidades.com/robo/</a></p>"
                      "</body></html>";
        localServer.send(200, "text/html", html);
    });

    localServer.on("/cmd", HTTP_GET, []() {
        if (localServer.hasArg("c")) {
            String c = localServer.arg("c");
            executeRobotCommand(c);
            localServer.send(200, "application/json", "{\"success\":true,\"cmd\":\"" + c + "\"}");
        } else {
            localServer.send(400, "application/json", "{\"success\":false,\"error\":\"Parametro c ausente\"}");
        }
    });

    localServer.on("/status", HTTP_GET, []() {
        String json = "{\"online\":true,\"ip\":\"" + WiFi.localIP().toString() + 
                      "\",\"rssi\":" + String(WiFi.RSSI()) + 
                      ",\"ssid\":\"" + WiFi.SSID() + 
                      "\",\"mood\":\"" + currentMoodStr + "\"}";
        localServer.send(200, "application/json", json);
    });

    localServer.begin();
    Serial.println("[HTTP Local] Servidor Web ativo na porta 80");

    // Hook de overlay para desenhar a barra de status no mesmo buffer dos olhos (sem flicker)
    roboEyes.onDrawOverlay = [](Adafruit_SSD1306* disp) {
        static bool cachedWifi = false;
        static int cachedRssi = 0;
        static unsigned long lastWifiCheck = 0;
        unsigned long t = millis();
        if (t - lastWifiCheck > 2500 || lastWifiCheck == 0) {
            lastWifiCheck = t;
            cachedWifi = (WiFi.status() == WL_CONNECTED);
            if (cachedWifi) cachedRssi = WiFi.RSSI();
        }
        drawStatusBar(*disp, cachedWifi, cachedRssi, getTimeString());
    };

    // Inicia a tarefa de polling da nuvem no Core 0 (libera 100% o Core 1 para gráficos!)
    xTaskCreatePinnedToCore(
        cloudPollingTask,
        "cloudPollingTask",
        8192,
        NULL,
        1,
        NULL,
        0 // Core 0
    );

    // Garante LEDs 100% desligados
    ledsOff();
}

void loop() {
    unsigned long now = millis();

    // Atende requisições do servidor web local
    localServer.handleClient();

    // Mantém a conexão Wi-Fi ativa via WiFiMulti
    static unsigned long lastWifiCheck = 0;
    if (now - lastWifiCheck > 5000) {
        lastWifiCheck = now;
        if (WiFi.status() != WL_CONNECTED) {
            wifiMulti.run();
        }
    }

    // Renderização dos olhos: fluida, sem travamentos e com framerate cravado a 30 FPS!
    if (showingText) {
        if (now - textStartTime > textDuration) {
            showingText = false;
        }
    } else {
        roboEyes.update();
    }

    // Leitura do Monitor Serial / Web Serial USB
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        executeRobotCommand(cmd);
    }

    // Executa comando remoto recebido da nuvem pelo Core 0
    if (pendingCloudCmd) {
        char cmdToRun[64];
        portENTER_CRITICAL(&cloudMux);
        strncpy(cmdToRun, cloudCmdBuffer, sizeof(cmdToRun));
        pendingCloudCmd = false;
        portEXIT_CRITICAL(&cloudMux);

        Serial.println("[Nuvem Hostinger] 📡 Executando comando remoto: " + String(cmdToRun));
        rgbLedWrite(RGB_BUILTIN, 0, 100, 255);
        delay(60);
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0);

        executeRobotCommand(String(cmdToRun));
    }

    delay(2);
}

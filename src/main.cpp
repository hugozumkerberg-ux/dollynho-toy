#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
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

// LED RGB On-board do ESP32-S3 (GPIO 48 no DevKitC-1)
#ifndef RGB_BUILTIN
#define RGB_BUILTIN 48
#endif

// Estados de Exibição
bool showingText = false;
unsigned long textStartTime = 0;
unsigned long textDuration = 6000;

// Variáveis de Cotação da B3 (BBAS3)
float bbas3Price = 28.45;
float bbas3Change = 1.82;
unsigned long lastStockCheckTime = 0;

// Ajuste fino dos LEDs (externos e interno do ESP32)
void setLedsColor(uint8_t r, uint8_t g, uint8_t b, uint8_t scale = 255) {
    analogWrite(PIN_LED_VERMELHO, (r * scale) / 255);
    analogWrite(PIN_LED_AMARELO,  (((g > 0 && r > 0 ? 180 : 0)) * scale) / 255);
    analogWrite(PIN_LED_VERDE,    (g * scale) / 255);
    // Ativa o LED RGB On-board do ESP32-S3 (GPIO 48)
    rgbLedWrite(RGB_BUILTIN, (r * scale) / 255, (g * scale) / 255, (b * scale) / 255);
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
    THEME_SENNA
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
        }

        if (pitch > 0) {
            tone(PIN_BUZZER, pitch, duration);

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
            } else if (pitch > 550) {
                setLedsColor(0, 255, 0, brightness);
            } else {
                setLedsColor(255, 0, 0, brightness);
            }
        } else {
            noTone(PIN_BUZZER);
            setLedsColor(0, 0, 0, 0);
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

        noTone(PIN_BUZZER);
        setLedsColor(0, 0, 0, 0);
        delay(25); // Breve pausa entre as notas
    }
    noTone(PIN_BUZZER);
    showingText = false; // Retorna aos olhinhos orgânicos do RoboEyes
    roboEyes.update();
}

int totalCoinsCollected = 0;

void notifyMarioCoin() {
    totalCoinsCollected++;
    showingText = true;
    textStartTime = millis();
    textDuration = 2200; // Mantém a notificação na tela por 2.2 segundos

    Serial.printf("[Mario] 🪙 Moeda Coletada! Total: %d (+200 PTS)\n", totalCoinsCollected);
    roboEyes.setMood(HAPPY);

    // Primeiro tom agudo (B5) com brilho dourado nos LEDs
    tone(PIN_BUZZER, NOTE_B5, 75);
    analogWrite(PIN_LED_VERDE, 255);
    rgbLedWrite(RGB_BUILTIN, 255, 215, 0); // Dourado brilhante no ESP32

    for (int f = 0; f < 8; f++) {
        drawMarioCoinNotification(f, totalCoinsCollected, display);
        delay(28);
    }

    // Segundo tom (E6) no ápice do pulo com brilho verde
    tone(PIN_BUZZER, NOTE_E6, 260);
    analogWrite(PIN_LED_VERDE, 200);
    rgbLedWrite(RGB_BUILTIN, 50, 255, 100);

    for (int f = 8; f <= 15; f++) {
        drawMarioCoinNotification(f, totalCoinsCollected, display);
        delay(28);
    }

    noTone(PIN_BUZZER);
    analogWrite(PIN_LED_VERDE, 0);
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
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
        flashLeds(3, 80, true);
        play8BitCoin();
    } else {
        roboEyes.setMood(TIRED);
        roboEyes.setSweat(ON);
        flashLeds(3, 80, false);
        tone(PIN_BUZZER, NOTE_FS4, 300);
        delay(300);
        noTone(PIN_BUZZER);
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
    tone(PIN_BUZZER, NOTE_A5, 60);
    delay(60);
    noTone(PIN_BUZZER);

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
            blinkApiOk(); // 1 piscada azul: API IA respondeu com sucesso!
            String respStr = https.getString();
            JsonDocument docResp;
            if (!deserializeJson(docResp, respStr)) {
                const char* replyText = docResp["candidates"][0]["content"]["parts"][0]["text"];
                if (replyText) {
                    String fullReply = String(replyText);
                    fullReply.trim();
                    Serial.println("\n[Dollynho]: " + fullReply);

                    if (fullReply.indexOf("[ANGRY]") >= 0) {
                        roboEyes.setMood(ANGRY);
                        fullReply.replace("[ANGRY]", "");
                        playSong(SONG_IMPERIAL_MARCH, 18, THEME_STAR_WARS_IMPERIAL); // Toca trecho da marcha imperial
                    } else if (fullReply.indexOf("[TIRED]") >= 0) {
                        roboEyes.setMood(TIRED);
                        fullReply.replace("[TIRED]", "");
                        tone(PIN_BUZZER, NOTE_FS4, 250); delay(250); noTone(PIN_BUZZER);
                    } else if (fullReply.indexOf("[HAPPY]") >= 0) {
                        roboEyes.setMood(HAPPY);
                        roboEyes.anim_laugh();
                        fullReply.replace("[HAPPY]", "");
                        play8BitCoin();
                    } else {
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
// Pulsação das Lâmpadas de LED Conforme o Sentimento
// ----------------------------------------------------
void updatePulsingLeds() {
    if (showingText) return; // Não interfere durante reprodução de texto/notificações

    unsigned long now = millis();
    int potValue = analogRead(PIN_POTENCIOMETRO);
    uint8_t maxBrightness = map(potValue, 0, 4095, 25, 255);

    float pulse = 0.0f;
    uint8_t r = 0, g = 0, b = 0;

    if (roboEyes.happy) {
        // FELIZ (^ ^): Pulso respiratório suave e alegre em Verde Esmeralda (~1.3s)
        float wave = (sin(now * 0.0048f) + 1.0f) * 0.5f;
        pulse = 0.25f + 0.75f * wave;
        r = 0;
        g = 255;
        b = 30;
    }
    else if (roboEyes.angry) {
        // BRAVO (> <): Pulso tenso e acelerado em Vermelho Vivo (~0.5s) com pico agressivo
        float wave = (sin(now * 0.0125f) + 1.0f) * 0.5f;
        pulse = 0.20f + 0.80f * (wave * wave);
        r = 255;
        g = 0;
        b = 0;
    }
    else if (roboEyes.tired) {
        // CANSADO / TRISTE (T_T): Respiração lenta, suave e fading em Âmbar/Vermelho (~3.5s)
        float wave = (sin(now * 0.0018f) + 1.0f) * 0.5f;
        pulse = 0.10f + 0.55f * wave;
        r = 230;
        g = 40;
        b = 0;
    }
    else {
        // NORMAL / CURIOSO (o o): Pulso calmo e rítmico em Ciano Zen (~2.3s)
        float wave = (sin(now * 0.0028f) + 1.0f) * 0.5f;
        pulse = 0.20f + 0.80f * wave;
        r = 0;
        g = 180;
        b = 230;
    }

    uint8_t scaledBrightness = (uint8_t)(maxBrightness * pulse);
    setLedsColor(r, g, b, scaledBrightness);
}

void setup() {
    Serial.begin(115200);
    delay(800);
    Serial.println("\n==============================================");
    Serial.println("  🤖 Dollynho Toy: Músicas 8-Bits + Star Wars ");
    Serial.println("==============================================");

    pinMode(PIN_LED_VERDE, OUTPUT);
    pinMode(PIN_LED_AMARELO, OUTPUT);
    pinMode(PIN_LED_VERMELHO, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_POTENCIOMETRO, INPUT);

    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    // Inicialização do RoboEyes
    roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 50);
    roboEyes.setWidth(36, 36);
    roboEyes.setHeight(36, 36);
    roboEyes.setBorderradius(8, 8);
    roboEyes.setSpacebetween(10);
    roboEyes.setCuriosity(ON);
    roboEyes.setAutoblinker(ON, 3, 2);
    roboEyes.setIdleMode(ON, 2, 2);

    setLedsColor(0, 255, 0);
    play8BitCoin();

    // Conexão Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(350);
        attempts++;
        roboEyes.update();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[Wi-Fi] Conectado! IP: " + WiFi.localIP().toString());
        blinkWifiConnected(); // 3 piscadas verdes no LED do ESP32!
        roboEyes.setMood(HAPPY);
        roboEyes.anim_laugh();
    } else {
        Serial.println("[Wi-Fi] Modo Offline");
        roboEyes.setMood(TIRED);
    }
}

void loop() {
    unsigned long now = millis();

    // Atualiza a pulsação contínua dos LEDs conforme o sentimento
    updatePulsingLeds();

    if (showingText) {
        if (now - textStartTime > textDuration) {
            showingText = false;
        }
    } else {
        roboEyes.update();
    }

    // Consulta periódica da BBAS3 a cada 5 minutos
    if (now - lastStockCheckTime > 300000) {
        lastStockCheckTime = now;
        updateStockBBAS3();
    }

    // Leitura do Monitor Serial
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "1" || cmd == "happy") { roboEyes.setMood(HAPPY); play8BitCoin(); }
        else if (cmd == "2" || cmd == "angry") { roboEyes.setMood(ANGRY); playSong(SONG_IMPERIAL_MARCH, LEN_IMPERIAL_MARCH); }
        else if (cmd == "3" || cmd == "tired") { roboEyes.setMood(TIRED); tone(PIN_BUZZER, NOTE_FS4, 250); delay(250); noTone(PIN_BUZZER); }
        else if (cmd == "4" || cmd == "default") { roboEyes.setMood(DEFAULT); roboEyes.setSweat(OFF); }
        else if (cmd == "laugh") { roboEyes.anim_laugh(); play8BitCoin(); }
        else if (cmd == "confused") { roboEyes.anim_confused(); tone(PIN_BUZZER, NOTE_A5, 80); delay(80); noTone(PIN_BUZZER); }
        else if (cmd == "bbas3" || cmd == "bolsa") { updateStockBBAS3(); }
        else if (cmd == "coin" || cmd == "moeda") { notifyMarioCoin(); }
        
        // MÚSICAS COMPLETAS COM ANIMAÇÃO VISUAL
        else if (cmd == "starwars" || cmd == "sw") {
            Serial.println("[Música] Tocando Star Wars: Tema Principal + Logo no OLED!");
            roboEyes.setMood(HAPPY);
            playSong(SONG_STAR_WARS_MAIN, LEN_STAR_WARS_MAIN, THEME_STAR_WARS_MAIN);
        }
        else if (cmd == "imperial" || cmd == "vader") {
            Serial.println("[Música] Tocando Star Wars: Marcha Imperial + Logo no OLED!");
            roboEyes.setMood(ANGRY);
            playSong(SONG_IMPERIAL_MARCH, LEN_IMPERIAL_MARCH, THEME_STAR_WARS_IMPERIAL);
        }
        else if (cmd == "mario") {
            Serial.println("[Música] Tocando Super Mario Bros + Animação no OLED!");
            roboEyes.setMood(HAPPY);
            playSong(SONG_MARIO_FULL, LEN_MARIO_FULL, THEME_MARIO);
        }
        else if (cmd == "tetris") {
            Serial.println("[Música] Tocando Tetris Korobeiniki Completo!");
            roboEyes.setMood(DEFAULT);
            playSong(SONG_TETRIS_FULL, LEN_TETRIS_FULL, THEME_NONE);
        }
        else if (cmd == "senna" || cmd == "vitoria" || cmd == "f1") {
            Serial.println("[Música] 🏎️ Ayrton Senna: Tema da Vitória + Logo Senna no OLED!");
            roboEyes.setMood(HAPPY);
            roboEyes.anim_laugh();
            playSong(SONG_SENNA_VITORIA, LEN_SENNA_VITORIA, THEME_SENNA);
        }
        else {
            askGemini(cmd);
        }
    }

    delay(5);
}

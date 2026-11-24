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
    neopixelWrite(RGB_BUILTIN, (r * scale) / 255, (g * scale) / 255, (b * scale) / 255);
}

// 3 Piscadas Verdes: Wi-Fi Conectado com sucesso
void blinkWifiConnected() {
    Serial.println("[LED ESP32] 3 Piscadas Verdes: Wi-Fi Conectado!");
    for (int i = 0; i < 3; i++) {
        neopixelWrite(RGB_BUILTIN, 0, 220, 0); // Verde forte no ESP32
        analogWrite(PIN_LED_VERDE, 255);       // Verde externo
        delay(180);
        neopixelWrite(RGB_BUILTIN, 0, 0, 0);
        analogWrite(PIN_LED_VERDE, 0);
        delay(180);
    }
}

// 1 Piscada Azul: Resposta da API IA recebida com sucesso (OK)
void blinkApiOk() {
    Serial.println("[LED ESP32] 1 Piscada Azul: API Gemini IA OK!");
    neopixelWrite(RGB_BUILTIN, 0, 0, 255); // Azul forte no ESP32
    delay(450);
    neopixelWrite(RGB_BUILTIN, 0, 0, 0);
}

void flashLeds(int count, int delayMs, bool isGreen) {
    for (int i = 0; i < count; i++) {
        analogWrite(isGreen ? PIN_LED_VERDE : PIN_LED_VERMELHO, 255);
        analogWrite(isGreen ? PIN_LED_VERMELHO : PIN_LED_VERDE, 0);
        neopixelWrite(RGB_BUILTIN, isGreen ? 0 : 255, isGreen ? 255 : 0, 0);
        delay(delayMs);
        analogWrite(PIN_LED_VERDE, 0);
        analogWrite(PIN_LED_VERMELHO, 0);
        neopixelWrite(RGB_BUILTIN, 0, 0, 0);
        delay(delayMs);
    }
}

// ----------------------------------------------------
// Tocador de Músicas 8-Bits com Animação Fluida dos Olhos
// ----------------------------------------------------
void playSong(const Note* song, int length) {
    for (int i = 0; i < length; i++) {
        Note n;
        memcpy_P(&n, &song[i], sizeof(Note));

        int pitch = n.pitch;
        int duration = n.durationMs;

        // Leitura dinâmica do potenciômetro para brilho durante a música
        int potValue = analogRead(PIN_POTENCIOMETRO);
        uint8_t brightness = map(potValue, 0, 4095, 40, 255);

        if (pitch > 0) {
            tone(PIN_BUZZER, pitch, duration);

            // LEDs piscam no ritmo da música (verde para agudos, vermelho para graves)
            if (pitch > 550) {
                setLedsColor(0, 255, 0, brightness);
            } else {
                setLedsColor(255, 0, 0, brightness);
            }
        } else {
            noTone(PIN_BUZZER);
            setLedsColor(0, 0, 0, 0);
        }

        // Divide a duração da nota em fatias pequenas para manter o RoboEyes animando suavemente
        int elapsed = 0;
        while (elapsed < duration) {
            int step = min(20, duration - elapsed);
            delay(step);
            elapsed += step;
            if (!showingText) {
                roboEyes.update();
            }
        }

        noTone(PIN_BUZZER);
        setLedsColor(0, 0, 0, 0);
        delay(25); // Breve pausa entre as notas
    }
    noTone(PIN_BUZZER);
}

void play8BitCoin() {
    tone(PIN_BUZZER, NOTE_B5, 80);
    delay(80);
    tone(PIN_BUZZER, NOTE_E6, 250);
    delay(250);
    noTone(PIN_BUZZER);
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
        
        String systemPrompt = "Voce e o robo assistente 'Bito'. Responda em portugues em ate 14 palavras. "
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
                    Serial.println("\n[Bito]: " + fullReply);

                    if (fullReply.indexOf("[ANGRY]") >= 0) {
                        roboEyes.setMood(ANGRY);
                        fullReply.replace("[ANGRY]", "");
                        playSong(SONG_IMPERIAL_MARCH, 18); // Toca trecho da marcha imperial
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
                    display.println("--- Bito diz: ---");
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

void setup() {
    Serial.begin(115200);
    delay(800);
    Serial.println("\n==============================================");
    Serial.println("  🤖 Bito: Músicas 8-Bits + Star Wars + B3    ");
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

    // Controle de brilho analógico via potenciômetro
    int potValue = analogRead(PIN_POTENCIOMETRO);
    uint8_t brightness = map(potValue, 0, 4095, 30, 255);

    if (roboEyes.happy) setLedsColor(0, 255, 0, brightness);
    else if (roboEyes.angry || roboEyes.tired) setLedsColor(255, 0, 0, brightness);
    else setLedsColor(0, 200, 200, brightness);

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
        
        // MÚSICAS COMPLETAS
        else if (cmd == "starwars" || cmd == "sw") {
            Serial.println("[Música] Tocando Star Wars: Tema Principal Completo!");
            roboEyes.setMood(HAPPY);
            playSong(SONG_STAR_WARS_MAIN, LEN_STAR_WARS_MAIN);
        }
        else if (cmd == "imperial" || cmd == "vader") {
            Serial.println("[Música] Tocando Star Wars: Marcha Imperial Completa!");
            roboEyes.setMood(ANGRY);
            playSong(SONG_IMPERIAL_MARCH, LEN_IMPERIAL_MARCH);
        }
        else if (cmd == "mario") {
            Serial.println("[Música] Tocando Super Mario Bros Completo!");
            roboEyes.setMood(HAPPY);
            playSong(SONG_MARIO_FULL, LEN_MARIO_FULL);
        }
        else if (cmd == "tetris") {
            Serial.println("[Música] Tocando Tetris Korobeiniki Completo!");
            roboEyes.setMood(DEFAULT);
            playSong(SONG_TETRIS_FULL, LEN_TETRIS_FULL);
        }
        else {
            askGemini(cmd);
        }
    }

    delay(5);
}

#ifndef CONFIG_ROBOT_H
#define CONFIG_ROBOT_H

// --- Configurações de Pinos (ESP32-S3 N16R8) ---
#define PIN_OLED_SDA 8
#define PIN_OLED_SCL 9

#define PIN_LED_VERDE   4   // Feliz / BBAS3 Alta
#define PIN_LED_AMARELO 5   // Alerta / Atenção
#define PIN_LED_VERMELHO 6  // Triste / BBAS3 Queda

#define PIN_POTENCIOMETRO 1  // ADC1_CH0 (Ajuste de Brilho)
#define PIN_BUZZER        15 // Buzzer Piezoelétrico (Músicas 8-bit)

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// Wi-Fi (Configure com a sua rede)
#define WIFI_SSID "SEU_WIFI_AQUI"
#define WIFI_PASS "SUA_SENHA_AQUI"

// Google Gemini API Key
#define GEMINI_API_KEY "SUA_CHAVE_GEMINI_AQUI"
#define GEMINI_MODEL   "gemini-2.5-flash"

#endif // CONFIG_ROBOT_H

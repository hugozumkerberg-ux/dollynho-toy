#ifndef GRAPHICS_OLED_H
#define GRAPHICS_OLED_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------------------------------------------------------------
// Sprites 16x16 do Super Mario 8-Bit
// -------------------------------------------------------------
const unsigned char PROGMEM MARIO_RUN1_16x16[] = {
    0x07, 0x00, 0x0f, 0xc0, 0x04, 0x80, 0x0e, 0xc0,
    0x0f, 0xc0, 0x03, 0x80, 0x07, 0xf0, 0x1f, 0xf8,
    0x3f, 0xfc, 0x2f, 0xf4, 0x0f, 0xf0, 0x0f, 0xf0,
    0x1e, 0x78, 0x3c, 0x3c, 0x78, 0x1e, 0xe0, 0x07
};

const unsigned char PROGMEM MARIO_JUMP_16x16[] = {
    0x07, 0x00, 0x0f, 0xc0, 0x04, 0x80, 0x0e, 0xc0,
    0x0f, 0xc0, 0x03, 0x80, 0x1f, 0xf8, 0x3f, 0xfc,
    0x7f, 0xfe, 0x6f, 0xf6, 0x0f, 0xf0, 0x1f, 0xf8,
    0x38, 0x1c, 0x70, 0x0e, 0x60, 0x06, 0x00, 0x00
};

// Bloco [?] 12x12
const unsigned char PROGMEM BLOCK_QUESTION_12x12[] = {
    0xff, 0xf0, 0x80, 0x10, 0x9c, 0x90, 0xb2, 0xd0,
    0x86, 0x10, 0x8c, 0x10, 0x8c, 0x10, 0x80, 0x10,
    0x8c, 0x10, 0x8c, 0x10, 0x80, 0x10, 0xff, 0xf0
};

// Moeda 8x8
const unsigned char PROGMEM COIN_8x8[] = {
    0x3c, 0x7e, 0x66, 0x66, 0x66, 0x66, 0x7e, 0x3c
};

// Moeda Rotacionando (Fases de giro para notificação brilhante)
const unsigned char PROGMEM COIN_ANIM_FRONT[] = {
    0x3c, 0x00, 0x7e, 0x00, 0xff, 0x00, 0xdb, 0x00,
    0xdb, 0x00, 0xdb, 0x00, 0xdb, 0x00, 0xdb, 0x00,
    0xff, 0x00, 0x7e, 0x00, 0x3c, 0x00, 0x00, 0x00
};
const unsigned char PROGMEM COIN_ANIM_SIDE[] = {
    0x18, 0x00, 0x3c, 0x00, 0x7e, 0x00, 0x66, 0x00,
    0x66, 0x00, 0x66, 0x00, 0x66, 0x00, 0x66, 0x00,
    0x7e, 0x00, 0x3c, 0x00, 0x18, 0x00, 0x00, 0x00
};
const unsigned char PROGMEM COIN_ANIM_EDGE[] = {
    0x18, 0x00, 0x18, 0x00, 0x3c, 0x00, 0x3c, 0x00,
    0x3c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x3c, 0x00,
    0x3c, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00
};

// -------------------------------------------------------------
// Renderizador da Animação do Mario no OLED 128x64
// -------------------------------------------------------------
inline void drawMarioScene(int step, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Placar / Topo estilo NES
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(4, 2);
    disp.print("MARIO");
    disp.setCursor(50, 2);
    disp.print("x03");
    disp.setCursor(84, 2);
    disp.print("WORLD 1-1");

    // Chão com tijolos
    disp.drawLine(0, 60, 127, 60, SSD1306_WHITE);
    for (int x = 0; x < 128; x += 8) {
        disp.drawPixel(x, 62, SSD1306_WHITE);
    }

    // Bloco [?] flutuante no centro
    disp.drawBitmap(58, 26, BLOCK_QUESTION_12x12, 12, 12, SSD1306_WHITE);

    // Posição horizontal do Mario (vai da esquerda para a direita)
    int marioX = (step * 4) % 134 - 8;
    int marioY = 44;
    bool isJumping = false;

    // Mario pula ao passar perto do bloco central
    if (marioX >= 36 && marioX <= 80) {
        isJumping = true;
        float progress = (marioX - 36) / 44.0;
        marioY = 44 - (int)(sin(progress * 3.14159) * 16);

        // Moeda pula para fora do bloco no ápice do pulo
        if (marioX >= 52 && marioX <= 68) {
            disp.drawBitmap(60, 10, COIN_8x8, 8, 8, SSD1306_WHITE);
        }
    }

    if (isJumping) {
        disp.drawBitmap(marioX, marioY, MARIO_JUMP_16x16, 16, 16, SSD1306_WHITE);
    } else {
        disp.drawBitmap(marioX, marioY, MARIO_RUN1_16x16, 16, 16, SSD1306_WHITE);
    }

    disp.display();
}

// -------------------------------------------------------------
// Renderizador do Logo STAR WARS + Hyperspace no OLED 128x64
// -------------------------------------------------------------
inline void drawStarWarsScene(int step, const char* subtitle, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Efeito Hyperspace (Estrelas viajando em direção à tela)
    for (int i = 0; i < 22; i++) {
        float angle = (i * 16.36) * 0.0174533; // Ângulo radial
        float dist = ((step * 3 + i * 8) % 68);
        int sx = 64 + (int)(cos(angle) * dist * 1.0);
        int sy = 32 + (int)(sin(angle) * dist * 0.52);

        if (sx >= 0 && sx < 128 && sy >= 0 && sy < 64) {
            disp.drawPixel(sx, sy, SSD1306_WHITE);
            if (dist > 35) {
                // Rastro da estrela na velocidade da luz
                disp.drawPixel(sx - 1, sy, SSD1306_WHITE);
            }
        }
    }

    // Moldura do Logo STAR WARS centralizada
    disp.fillRoundRect(8, 12, 112, 40, 5, SSD1306_BLACK);
    disp.drawRoundRect(8, 12, 112, 40, 5, SSD1306_WHITE);
    disp.drawRoundRect(10, 14, 108, 36, 4, SSD1306_WHITE);

    // Tipografia STAR WARS
    disp.setTextSize(2);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(14, 17);
    disp.print("STAR");
    disp.setCursor(66, 17);
    disp.print("WARS");

    // Subtítulo da música
    disp.setTextSize(1);
    disp.setCursor(14, 38);
    disp.print(subtitle);

    disp.display();
}

// -------------------------------------------------------------
// Notificação Especial: Moeda do Super Mario (+200 PTS) no OLED
// -------------------------------------------------------------
inline void drawMarioCoinNotification(int frame, int totalCoins, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Moldura estilo card retrô NES
    disp.drawRoundRect(2, 2, 124, 60, 4, SSD1306_WHITE);
    disp.drawFastHLine(6, 16, 116, SSD1306_WHITE);

    // Cabeçalho da Notificação
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(18, 5);
    disp.print("! MOEDA MARIO !");

    // Bloco [?] central na parte inferior
    int blockBump = (frame >= 1 && frame <= 4) ? (frame <= 2 ? -2 : -1) : 0;
    disp.drawBitmap(58, 44 + blockBump, BLOCK_QUESTION_12x12, 12, 12, SSD1306_WHITE);

    // Salto parabólico da moeda saindo do bloco
    float progress = (float)frame / 14.0f;
    if (progress > 1.0f) progress = 1.0f;
    int coinY = 40 - (int)(sin(progress * 3.14159f) * 22);

    // Animação de rotação da moeda
    int spin = frame % 4;
    const unsigned char* coinSprite = (spin == 0) ? COIN_ANIM_FRONT : ((spin == 1 || spin == 3) ? COIN_ANIM_SIDE : COIN_ANIM_EDGE);
    disp.drawBitmap(59, coinY, coinSprite, 10, 12, SSD1306_WHITE);

    // Estrelas cintilantes de brilho no ápice
    if (frame >= 5 && frame <= 11) {
        disp.drawPixel(50, coinY + 2, SSD1306_WHITE);
        disp.drawPixel(51, coinY + 1, SSD1306_WHITE);
        disp.drawPixel(51, coinY + 3, SSD1306_WHITE);
        disp.drawPixel(52, coinY + 2, SSD1306_WHITE);

        disp.drawPixel(76, coinY - 1, SSD1306_WHITE);
        disp.drawPixel(77, coinY - 2, SSD1306_WHITE);
        disp.drawPixel(77, coinY,     SSD1306_WHITE);
        disp.drawPixel(78, coinY - 1, SSD1306_WHITE);
    }

    // Texto flutuante de pontuação "+200"
    if (frame >= 3) {
        int textY = 26 - (frame - 3);
        if (textY < 19) textY = 19;
        disp.setCursor(48, textY);
        disp.print("+200");
    }

    // Placar inferior
    disp.setCursor(8, 48);
    disp.printf("x%02d", totalCoins);
    disp.setCursor(82, 48);
    disp.print("+1 COIN");

    disp.display();
}

#endif // GRAPHICS_OLED_H

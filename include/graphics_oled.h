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

// -------------------------------------------------------------
// Sprites do Ayrton Senna: Logo "S" 32x22 e Carro F1 24x10
// -------------------------------------------------------------
const unsigned char PROGMEM SENNA_LOGO_32x22[] = {
    0x03, 0xfc, 0x00, 0x00,
    0x07, 0xff, 0x00, 0x00,
    0x0f, 0x0f, 0x80, 0x00,
    0x1e, 0x07, 0xc0, 0x00,
    0x1c, 0x03, 0xc0, 0x00,
    0x1c, 0x03, 0x80, 0x00,
    0x00, 0x07, 0x00, 0x00,
    0x00, 0x0f, 0x00, 0x00,
    0x00, 0x3e, 0x00, 0x00,
    0x00, 0x7c, 0x00, 0x00,
    0x01, 0xf8, 0x00, 0x00,
    0x03, 0xf0, 0x00, 0x00,
    0x07, 0xc0, 0x00, 0x00,
    0x0f, 0x00, 0x00, 0x00,
    0x1c, 0x03, 0x80, 0x00,
    0x1c, 0x07, 0x00, 0x00,
    0x1e, 0x0e, 0x00, 0x00,
    0x0f, 0x1e, 0x00, 0x00,
    0x07, 0xfc, 0x00, 0x00,
    0x01, 0xf0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM F1_CAR_24x10[] = {
    0x00, 0x38, 0x00,
    0x00, 0x7c, 0x00,
    0x60, 0x7e, 0x00,
    0x60, 0x7e, 0x00,
    0x71, 0xff, 0x80,
    0x7f, 0xff, 0xe0,
    0x3f, 0xff, 0xfc,
    0x1c, 0x00, 0x38,
    0x1c, 0x00, 0x38,
    0x08, 0x00, 0x10
};

// -------------------------------------------------------------
// Renderizador Especial: Ayrton Senna + Logo "S" no OLED
// -------------------------------------------------------------
inline void drawSennaScene(int step, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Bandeira quadriculada da Fórmula 1 no topo
    for (int x = 0; x < 128; x += 8) {
        if ((x / 8 + (step % 2)) % 2 == 0) {
            disp.fillRect(x, 0, 4, 3, SSD1306_WHITE);
            disp.fillRect(x + 4, 3, 4, 3, SSD1306_WHITE);
        } else {
            disp.fillRect(x + 4, 0, 4, 3, SSD1306_WHITE);
            disp.fillRect(x, 3, 4, 3, SSD1306_WHITE);
        }
    }

    // Logo SENNA 'S' estilizado no lado esquerdo
    disp.drawBitmap(4, 12, SENNA_LOGO_32x22, 32, 22, SSD1306_WHITE);

    // Tipografia Ayrton Senna
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(42, 10);
    disp.print("AYRTON");

    disp.setTextSize(2);
    disp.setCursor(42, 20);
    disp.print("SENNA");

    disp.setTextSize(1);
    disp.setCursor(42, 38);
    disp.print("DO BRASIL! #1");

    // Linha de asfalto / pista de corrida
    disp.drawLine(0, 52, 127, 52, SSD1306_WHITE);

    // Carro de Fórmula 1 acelerando pela pista
    int carX = (step * 8) % 160 - 24;
    disp.drawBitmap(carX, 53, F1_CAR_24x10, 24, 10, SSD1306_WHITE);

    // Linhas de velocidade atrás do carro
    if (carX > 10) {
        disp.drawFastHLine(carX - 8, 56, 6, SSD1306_WHITE);
        disp.drawFastHLine(carX - 16, 58, 8, SSD1306_WHITE);
    }

    disp.display();
}

// -------------------------------------------------------------
// Sprites do Pac-Man 16x16 e Fantasma Blinky 16x16
// -------------------------------------------------------------
const unsigned char PROGMEM PACMAN_OPEN_16x16[] = {
    0x07, 0xe0, 0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe,
    0x7f, 0xfe, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xe0,
    0xff, 0xe0, 0xff, 0xf8, 0xff, 0xff, 0x7f, 0xfe,
    0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x07, 0xe0
};

const unsigned char PROGMEM PACMAN_CLOSED_16x16[] = {
    0x07, 0xe0, 0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe,
    0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe,
    0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x07, 0xe0
};

const unsigned char PROGMEM GHOST_BLINKY_16x16[] = {
    0x07, 0xe0, 0x1f, 0xf8, 0x3f, 0xfc, 0x79, 0x9e,
    0x71, 0x8e, 0xf9, 0x9f, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xdb, 0xdb, 0xd9, 0x9b, 0x81, 0x81
};

// -------------------------------------------------------------
// Renderizador Especial: PAC-MAN Arcade 1980 no OLED
// -------------------------------------------------------------
inline void drawPacmanScene(int step, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Labirinto / Bordas azuis estilo arcade
    disp.drawFastHLine(0, 0, 128, SSD1306_WHITE);
    disp.drawFastHLine(0, 2, 128, SSD1306_WHITE);
    disp.drawFastHLine(0, 61, 128, SSD1306_WHITE);
    disp.drawFastHLine(0, 63, 128, SSD1306_WHITE);

    // Header Arcade
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(6, 6);
    disp.print("PAC-MAN");
    disp.setCursor(76, 6);
    disp.printf("PTS %04d", (step * 50) % 9990);

    // Bolinhas de comida do Pac-Man (Dots)
    for (int dx = 6; dx < 124; dx += 12) {
        disp.fillRect(dx, 33, 2, 2, SSD1306_WHITE);
    }
    // Pílula Energizadora maior
    disp.fillCircle(116, 34, 3, SSD1306_WHITE);

    // Posição de deslocamento do Pac-Man (da esquerda para a direita)
    int pacX = (step * 6) % 160 - 20;
    bool openMouth = (step % 2 == 0);
    disp.drawBitmap(pacX, 26, openMouth ? PACMAN_OPEN_16x16 : PACMAN_CLOSED_16x16, 16, 16, SSD1306_WHITE);

    // Fantasma Blinky perseguindo
    int ghostX = pacX - 24;
    if (ghostX > -16 && ghostX < 128) {
        disp.drawBitmap(ghostX, 26, GHOST_BLINKY_16x16, 16, 16, SSD1306_WHITE);
    }

    // Efeito WAKKA WAKKA no rodapé
    disp.setCursor(40, 48);
    disp.print(openMouth ? "WAKKA!" : "WAKKA~");

    disp.display();
}

// -------------------------------------------------------------
// Sprites do Top Gun: Caça F-14 Tomcat 32x14
// -------------------------------------------------------------
const unsigned char PROGMEM F14_JET_32x14[] = {
    0x00, 0x01, 0x80, 0x00,
    0x00, 0x03, 0xc0, 0x00,
    0x00, 0x07, 0xe0, 0x00,
    0x00, 0x0f, 0xf0, 0x00,
    0x00, 0x1f, 0xf8, 0x00,
    0x00, 0x7f, 0xfe, 0x00,
    0x01, 0xff, 0xff, 0x80,
    0x0f, 0xff, 0xff, 0xf0,
    0x7f, 0xff, 0xff, 0xfe,
    0xff, 0xff, 0xff, 0xff,
    0x1f, 0x87, 0xe1, 0xf8,
    0x07, 0x03, 0xc0, 0xe0,
    0x0e, 0x01, 0x80, 0x70,
    0x0c, 0x00, 0x00, 0x30
};

// -------------------------------------------------------------
// Renderizador Especial: Top Gun (Danger Zone) no OLED
// -------------------------------------------------------------
inline void drawTopGunScene(int step, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Linhas de velocidade supersônica no horizonte
    for (int y = 8; y < 60; y += 14) {
        int xOffset = (step * 14 + y * 5) % 128;
        disp.drawFastHLine(xOffset, y, 24, SSD1306_WHITE);
    }

    // Logo Anos 80 "TOP GUN"
    disp.setTextSize(2);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(18, 4);
    disp.print("TOP GUN");

    disp.setTextSize(1);
    disp.setCursor(14, 22);
    disp.print(">> DANGER ZONE <<");

    // Caça F-14 Tomcat cortando o céu
    int jetX = 46 + (int)(sin((float)step * 0.4f) * 14.0f);
    int jetY = 36 + (int)(cos((float)step * 0.3f) * 4.0f);
    disp.drawBitmap(jetX, jetY, F14_JET_32x14, 32, 14, SSD1306_WHITE);

    // Fogo da pós-combustão (Afterburner exhaust)
    int exhaustLen = (step % 3 == 0) ? 14 : ((step % 3 == 1) ? 10 : 16);
    disp.drawLine(jetX + 10, jetY + 14, jetX + 10 - exhaustLen / 2, jetY + 14 + exhaustLen, SSD1306_WHITE);
    disp.drawLine(jetX + 21, jetY + 14, jetX + 21 + exhaustLen / 2, jetY + 14 + exhaustLen, SSD1306_WHITE);

    // Indicador de velocidade Mach no rodapé
    disp.setCursor(4, 54);
    disp.printf("MACH 2.%d", (step % 9) + 1);
    disp.setCursor(82, 54);
    disp.print("US NAVY");

    disp.display();
}

// -------------------------------------------------------------
// Sprites de Space Invaders 16x12 e Canhão Espacial 16x8
// -------------------------------------------------------------
const unsigned char PROGMEM INVADER_CRAB_16x12[] = {
    0x00, 0x00, 0x06, 0x60, 0x02, 0x40, 0x0f, 0xf0,
    0x1b, 0xd8, 0x3f, 0xfc, 0x30, 0x0c, 0x3f, 0xfc,
    0x35, 0xac, 0x10, 0x08, 0x1b, 0xd8, 0x06, 0x60
};

const unsigned char PROGMEM SPACE_CANNON_16x8[] = {
    0x01, 0x80, 0x01, 0x80, 0x07, 0xe0, 0x07, 0xe0,
    0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe, 0x7f, 0xfe
};

// -------------------------------------------------------------
// Renderizador Especial: Arcade Space / Space Invaders no OLED
// -------------------------------------------------------------
inline void drawArcadeSpaceScene(int step, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Placar Arcade clássico
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(4, 2);
    disp.print("SCORE<1>");
    disp.setCursor(68, 2);
    disp.printf("%05d", step * 110);

    // Estrelas piscando no espaço profundo
    for (int s = 0; s < 12; s++) {
        int sx = (s * 29 + step * 3) % 126 + 1;
        int sy = (s * 17) % 40 + 12;
        disp.drawPixel(sx, sy, SSD1306_WHITE);
    }

    // Alien Invader oscilando e descendo
    int alienX = 14 + (step * 4) % 94;
    int alienY = 14 + ((step / 10) % 3) * 6;
    disp.drawBitmap(alienX, alienY, INVADER_CRAB_16x12, 16, 12, SSD1306_WHITE);

    // Canhão de Defesa Terrestre
    int shipX = 56 + (int)(sin((float)step * 0.5f) * 26.0f);
    disp.drawBitmap(shipX, 54, SPACE_CANNON_16x8, 16, 8, SSD1306_WHITE);

    // Disparos de laser subindo em direção ao alien
    int laserY = 50 - ((step * 6) % 38);
    disp.drawFastVLine(shipX + 7, laserY, 5, SSD1306_WHITE);
    disp.drawFastVLine(shipX + 8, laserY, 5, SSD1306_WHITE);

    // Bunkers de proteção
    disp.fillRect(20, 48, 8, 4, SSD1306_WHITE);
    disp.fillRect(98, 48, 8, 4, SSD1306_WHITE);

    disp.display();
}

// -------------------------------------------------------------
// Renderizador Especial: Anos 80 / Take On Me (Synthwave Grid)
// -------------------------------------------------------------
inline void drawTakeOnMeScene(int step, Adafruit_SSD1306 &disp) {
    disp.clearDisplay();

    // Sol Retrô Synthwave com listras horizontais
    int sunX = 64;
    int sunY = 22;
    int sunR = 14;
    disp.drawCircle(sunX, sunY, sunR, SSD1306_WHITE);
    disp.drawCircle(sunX, sunY, sunR - 1, SSD1306_WHITE);
    for (int y = sunY - sunR + 3; y <= sunY + sunR - 3; y += 4) {
        disp.drawFastHLine(sunX - sunR + 2, y, sunR * 2 - 4, SSD1306_BLACK);
    }

    // Linha do horizonte
    disp.drawFastHLine(0, 36, 128, SSD1306_WHITE);

    // Grade de perspectiva 3D (Wireframe Grid anos 80)
    for (int x = 0; x <= 128; x += 16) {
        disp.drawLine(x, 63, 64 + (x - 64) / 4, 36, SSD1306_WHITE);
    }
    int gridScroll = (step * 3) % 8;
    for (int gy = 37 + gridScroll; gy < 64; gy += 7) {
        disp.drawFastHLine(0, gy, 128, SSD1306_WHITE);
    }

    // Tipografia Synthwave Anos 80
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(4, 4);
    disp.print("A-HA 1985");
    disp.setCursor(68, 4);
    disp.print("TAKE ON ME");

    // Equalizador 8-Bits animado nos cantos superiores
    for (int b = 0; b < 5; b++) {
        int h = (abs((step + b * 2) % 10 - 5) + 2) * 2;
        disp.fillRect(4 + b * 4, 32 - h, 3, h, SSD1306_WHITE);
        disp.fillRect(104 + b * 4, 32 - h, 3, h, SSD1306_WHITE);
    }

    disp.display();
}

// -------------------------------------------------------------
// Barra de Status Superior do OLED: Sinal de Wi-Fi e Hora
// -------------------------------------------------------------
inline void drawStatusBar(Adafruit_SSD1306 &disp, bool isWifiConnected, int rssi, const char* timeStr) {
    // Faixa superior de 11 pixels para a barra
    disp.fillRect(0, 0, 128, 11, SSD1306_BLACK);

    // 1. Ícone do Wi-Fi (Barras de intensidade de sinal)
    int bars = 1;
    if (isWifiConnected) {
        if (rssi >= -60) bars = 4;
        else if (rssi >= -70) bars = 3;
        else if (rssi >= -80) bars = 2;
        else bars = 1;
    } else {
        bars = 0; // Desconectado / Modo AP
    }

    // Desenha as 4 barras de sinal
    disp.fillRect(2, 7, 2, 3, (bars >= 1) ? SSD1306_WHITE : SSD1306_BLACK);
    if (bars < 1) disp.drawPixel(2, 9, SSD1306_WHITE);
    disp.fillRect(5, 5, 2, 5, (bars >= 2) ? SSD1306_WHITE : SSD1306_BLACK);
    disp.fillRect(8, 3, 2, 7, (bars >= 3) ? SSD1306_WHITE : SSD1306_BLACK);
    disp.fillRect(11, 1, 2, 9, (bars >= 4) ? SSD1306_WHITE : SSD1306_BLACK);

    // Texto de status do Wi-Fi ao lado do ícone
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    disp.setCursor(16, 2);
    if (isWifiConnected) {
        disp.print("WiFi");
    } else {
        disp.print("AP");
    }

    // 2. Hora no canto direito (ex: "04:55" ou "12:30")
    if (timeStr && strlen(timeStr) > 0) {
        disp.setCursor(96, 2);
        disp.print(timeStr);
    }

    // 3. Linha divisória pontilhada sutil separando a barra dos olhos
    for (int x = 0; x < 128; x += 3) {
        disp.drawPixel(x, 11, SSD1306_WHITE);
    }
}

#endif // GRAPHICS_OLED_H

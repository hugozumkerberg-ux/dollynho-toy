#ifndef SONGS_8BIT_H
#define SONGS_8BIT_H

#include <Arduino.h>
#include "pitches.h"

// Estrutura de Nota Musical
struct Note {
    int pitch;
    int durationMs;
};

// -------------------------------------------------------------
// 1. STAR WARS: THE IMPERIAL MARCH (MARCHA IMPERIAL COMPLETA)
// -------------------------------------------------------------
const Note PROGMEM SONG_IMPERIAL_MARCH[] = {
    // Parte 1
    {NOTE_A4, 500}, {NOTE_A4, 500}, {NOTE_A4, 500}, {NOTE_F4, 350}, {NOTE_C5, 150},
    {NOTE_A4, 500}, {NOTE_F4, 350}, {NOTE_C5, 150}, {NOTE_A4, 650}, {REST, 150},
    {NOTE_E5, 500}, {NOTE_E5, 500}, {NOTE_E5, 500}, {NOTE_F5, 350}, {NOTE_C5, 150},
    {NOTE_GS4, 500}, {NOTE_F4, 350}, {NOTE_C5, 150}, {NOTE_A4, 650}, {REST, 150},

    // Parte 2 (Solo Agudo)
    {NOTE_A5, 500}, {NOTE_A4, 300}, {NOTE_A4, 150}, {NOTE_A5, 450}, {NOTE_GS5, 300}, {NOTE_G5, 200},
    {NOTE_FS5, 125}, {NOTE_F5, 125}, {NOTE_FS5, 250}, {REST, 250}, {NOTE_AS4, 250}, {NOTE_DS5, 400},
    {NOTE_D5, 200}, {NOTE_CS5, 125}, {NOTE_C5, 125}, {NOTE_B4, 250}, {REST, 250}, {NOTE_F4, 250},
    {NOTE_GS4, 450}, {NOTE_F4, 300}, {NOTE_A4, 150}, {NOTE_C5, 450}, {NOTE_A4, 300}, {NOTE_C5, 150},
    {NOTE_E5, 650}, {REST, 150},

    // Parte 3 (Repetição com final clássico)
    {NOTE_A5, 500}, {NOTE_A4, 300}, {NOTE_A4, 150}, {NOTE_A5, 450}, {NOTE_GS5, 300}, {NOTE_G5, 200},
    {NOTE_FS5, 125}, {NOTE_F5, 125}, {NOTE_FS5, 250}, {REST, 250}, {NOTE_AS4, 250}, {NOTE_DS5, 400},
    {NOTE_D5, 200}, {NOTE_CS5, 125}, {NOTE_C5, 125}, {NOTE_B4, 250}, {REST, 250}, {NOTE_F4, 250},
    {NOTE_GS4, 450}, {NOTE_F4, 300}, {NOTE_C5, 150}, {NOTE_A4, 500}, {NOTE_F4, 350}, {NOTE_C5, 150},
    {NOTE_A4, 800}
};
const int LEN_IMPERIAL_MARCH = sizeof(SONG_IMPERIAL_MARCH) / sizeof(Note);

// -------------------------------------------------------------
// 2. STAR WARS: MAIN THEME (TEMA PRINCIPAL / FANFARRA COMPLETA)
// -------------------------------------------------------------
const Note PROGMEM SONG_STAR_WARS_MAIN[] = {
    // Intro
    {NOTE_D4, 150}, {NOTE_D4, 150}, {NOTE_D4, 150},
    {NOTE_G4, 600}, {NOTE_D5, 600},
    {NOTE_C5, 150}, {NOTE_B4, 150}, {NOTE_A4, 150}, {NOTE_G5, 600}, {NOTE_D5, 350},
    {NOTE_C5, 150}, {NOTE_B4, 150}, {NOTE_A4, 150}, {NOTE_G5, 600}, {NOTE_D5, 350},
    {NOTE_C5, 150}, {NOTE_B4, 150}, {NOTE_C5, 150}, {NOTE_A4, 600}, {REST, 200},

    // Segunda estrofe
    {NOTE_D4, 150}, {NOTE_D4, 150}, {NOTE_D4, 150},
    {NOTE_G4, 600}, {NOTE_D5, 600},
    {NOTE_C5, 150}, {NOTE_B4, 150}, {NOTE_A4, 150}, {NOTE_G5, 600}, {NOTE_D5, 350},
    {NOTE_C5, 150}, {NOTE_B4, 150}, {NOTE_A4, 150}, {NOTE_G5, 600}, {NOTE_D5, 350},
    {NOTE_C5, 150}, {NOTE_B4, 150}, {NOTE_C5, 150}, {NOTE_A4, 600}, {REST, 200},

    // Ponte épica
    {NOTE_D4, 200}, {NOTE_D4, 100}, {NOTE_E4, 300}, {NOTE_E4, 150}, {NOTE_C5, 200}, {NOTE_B4, 150}, {NOTE_A4, 150}, {NOTE_G4, 150},
    {NOTE_G4, 150}, {NOTE_A4, 150}, {NOTE_B4, 150}, {NOTE_A4, 300}, {NOTE_E4, 200}, {NOTE_FS4, 400},
    {NOTE_D4, 200}, {NOTE_D4, 100}, {NOTE_E4, 300}, {NOTE_E4, 150}, {NOTE_C5, 200}, {NOTE_B4, 150}, {NOTE_A4, 150}, {NOTE_G4, 150},
    {NOTE_D5, 500}, {NOTE_A4, 500}, {NOTE_D4, 600}
};
const int LEN_STAR_WARS_MAIN = sizeof(SONG_STAR_WARS_MAIN) / sizeof(Note);

// -------------------------------------------------------------
// 3. SUPER MARIO BROS (TEMA PRINCIPAL COMPLETO)
// -------------------------------------------------------------
const Note PROGMEM SONG_MARIO_FULL[] = {
    // Intro
    {NOTE_E5, 120}, {NOTE_E5, 120}, {REST, 100}, {NOTE_E5, 120}, {REST, 100}, {NOTE_C5, 120}, {NOTE_E5, 120},
    {REST, 100}, {NOTE_G5, 240}, {REST, 150}, {NOTE_G4, 240}, {REST, 200},

    // Parte A
    {NOTE_C5, 180}, {REST, 100}, {NOTE_G4, 180}, {REST, 100}, {NOTE_E4, 180}, {REST, 100},
    {NOTE_A4, 180}, {NOTE_B4, 180}, {NOTE_AS4, 120}, {NOTE_A4, 180},
    {NOTE_G4, 140}, {NOTE_E5, 140}, {NOTE_G5, 140}, {NOTE_A5, 180}, {NOTE_F5, 120}, {NOTE_G5, 120},
    {REST, 80}, {NOTE_E5, 180}, {NOTE_C5, 120}, {NOTE_D5, 120}, {NOTE_B4, 200}, {REST, 150},

    // Parte B (Repete com variação)
    {NOTE_C5, 180}, {REST, 100}, {NOTE_G4, 180}, {REST, 100}, {NOTE_E4, 180}, {REST, 100},
    {NOTE_A4, 180}, {NOTE_B4, 180}, {NOTE_AS4, 120}, {NOTE_A4, 180},
    {NOTE_G4, 140}, {NOTE_E5, 140}, {NOTE_G5, 140}, {NOTE_A5, 180}, {NOTE_F5, 120}, {NOTE_G5, 120},
    {REST, 80}, {NOTE_E5, 180}, {NOTE_C5, 120}, {NOTE_D5, 120}, {NOTE_B4, 250}
};
const int LEN_MARIO_FULL = sizeof(SONG_MARIO_FULL) / sizeof(Note);

// -------------------------------------------------------------
// 4. TETRIS: KOROBEINIKI (TEMA COMPLETO)
// -------------------------------------------------------------
const Note PROGMEM SONG_TETRIS_FULL[] = {
    // Parte 1
    {NOTE_E5, 200}, {NOTE_B4, 100}, {NOTE_C5, 100}, {NOTE_D5, 200}, {NOTE_C5, 100}, {NOTE_B4, 100},
    {NOTE_A4, 200}, {NOTE_A4, 100}, {NOTE_C5, 100}, {NOTE_E5, 200}, {NOTE_D5, 100}, {NOTE_C5, 100},
    {NOTE_B4, 250}, {NOTE_C5, 100}, {NOTE_D5, 200}, {NOTE_E5, 200},
    {NOTE_C5, 200}, {NOTE_A4, 200}, {NOTE_A4, 300}, {REST, 100},

    // Parte 2
    {NOTE_D5, 250}, {NOTE_F5, 100}, {NOTE_A5, 200}, {NOTE_G5, 100}, {NOTE_F5, 100},
    {NOTE_E5, 300}, {NOTE_C5, 100}, {NOTE_E5, 200}, {NOTE_D5, 100}, {NOTE_C5, 100},
    {NOTE_B4, 200}, {NOTE_B4, 100}, {NOTE_C5, 100}, {NOTE_D5, 200}, {NOTE_E5, 200},
    {NOTE_C5, 200}, {NOTE_A4, 200}, {NOTE_A4, 350}, {REST, 150},

    // Parte 3 (Refrão / Baixo)
    {NOTE_E4, 200}, {NOTE_C4, 200}, {NOTE_D4, 200}, {NOTE_B3, 200},
    {NOTE_C4, 200}, {NOTE_A3, 200}, {NOTE_GS3, 250}, {NOTE_B3, 200},
    {NOTE_E4, 200}, {NOTE_C4, 200}, {NOTE_D4, 200}, {NOTE_B3, 200},
    {NOTE_C4, 150}, {NOTE_E4, 150}, {NOTE_A4, 300}, {NOTE_GS4, 400}
};
const int LEN_TETRIS_FULL = sizeof(SONG_TETRIS_FULL) / sizeof(Note);

// -------------------------------------------------------------
// 5. AYRTON SENNA: TEMA DA VITÓRIA (FÓRMULA 1 COMPLETO)
// -------------------------------------------------------------
const Note PROGMEM SONG_SENNA_VITORIA[] = {
    // Fanfarra Triunfal / Introdução
    {NOTE_D4, 180}, {NOTE_G4, 420}, {NOTE_B4, 180}, {NOTE_D5, 650}, {REST, 140},
    {NOTE_C5, 180}, {NOTE_B4, 180}, {NOTE_A4, 180}, {NOTE_G4, 180}, {NOTE_A4, 750}, {REST, 180},

    {NOTE_D4, 180}, {NOTE_A4, 420}, {NOTE_C5, 180}, {NOTE_E5, 650}, {REST, 140},
    {NOTE_D5, 180}, {NOTE_C5, 180}, {NOTE_B4, 180}, {NOTE_A4, 180}, {NOTE_B4, 750}, {REST, 200},

    // Refrão Épico "Senna do Brasil!"
    {NOTE_D5, 180}, {NOTE_G5, 480}, {NOTE_FS5, 180}, {NOTE_E5, 180}, {NOTE_D5, 320}, {NOTE_C5, 320}, {NOTE_B4, 700}, {REST, 140},
    {NOTE_A4, 180}, {NOTE_B4, 180}, {NOTE_C5, 280}, {NOTE_B4, 280}, {NOTE_A4, 280}, {NOTE_G4, 900}, {REST, 180},

    // Segunda estrofe com resolução máxima
    {NOTE_D4, 180}, {NOTE_G4, 420}, {NOTE_B4, 180}, {NOTE_D5, 650}, {REST, 140},
    {NOTE_C5, 180}, {NOTE_B4, 180}, {NOTE_A4, 180}, {NOTE_G4, 180}, {NOTE_A4, 750}, {REST, 180},

    {NOTE_D5, 180}, {NOTE_G5, 520}, {NOTE_FS5, 180}, {NOTE_E5, 180}, {NOTE_D5, 320}, {NOTE_C5, 320}, {NOTE_B4, 700}, {REST, 140},
    {NOTE_A4, 180}, {NOTE_B4, 180}, {NOTE_C5, 280}, {NOTE_D5, 280}, {NOTE_B4, 300}, {NOTE_G4, 1200}
};
const int LEN_SENNA_VITORIA = sizeof(SONG_SENNA_VITORIA) / sizeof(Note);

// -------------------------------------------------------------
// 6. PAC-MAN (ARCADE 1980 - TEMA CLÁSSICO DE ABERTURA)
// -------------------------------------------------------------
const Note PROGMEM SONG_PACMAN_FULL[] = {
    // Frase A
    {NOTE_B4, 130}, {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 130},
    {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 180},
    {NOTE_C5, 130}, {NOTE_C6, 130}, {NOTE_G5, 130}, {NOTE_E5, 130},
    {NOTE_C6, 130}, {NOTE_G5, 130}, {NOTE_E5, 180},
    {NOTE_B4, 130}, {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 130},
    {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 180},
    {NOTE_DS5, 80}, {NOTE_E5, 80}, {NOTE_F5, 80}, {NOTE_F5, 80},
    {NOTE_FS5, 80}, {NOTE_G5, 80}, {NOTE_GS5, 80}, {NOTE_A5, 80},
    {NOTE_B5, 300}, {REST, 140},

    // Frase B (Repetição staccato)
    {NOTE_B4, 130}, {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 130},
    {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 180},
    {NOTE_C5, 130}, {NOTE_C6, 130}, {NOTE_G5, 130}, {NOTE_E5, 130},
    {NOTE_C6, 130}, {NOTE_G5, 130}, {NOTE_E5, 180},
    {NOTE_B4, 130}, {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 130},
    {NOTE_B5, 130}, {NOTE_FS5, 130}, {NOTE_DS5, 180},
    {NOTE_DS5, 80}, {NOTE_E5, 80}, {NOTE_F5, 80}, {NOTE_FS5, 80},
    {NOTE_G5, 80}, {NOTE_GS5, 80}, {NOTE_A5, 80}, {NOTE_B5, 450}
};
const int LEN_PACMAN_FULL = sizeof(SONG_PACMAN_FULL) / sizeof(Note);

// -------------------------------------------------------------
// 7. TOP GUN: DANGER ZONE & ANTHEM (KENNY LOGGINS 1986)
// -------------------------------------------------------------
const Note PROGMEM SONG_TOP_GUN[] = {
    // Intro Riff Enérgico
    {NOTE_C4, 150}, {NOTE_C4, 150}, {NOTE_DS4, 200}, {NOTE_F4, 200},
    {NOTE_G4, 250}, {NOTE_F4, 150}, {NOTE_DS4, 200}, {NOTE_C4, 300}, {REST, 100},
    {NOTE_C4, 150}, {NOTE_C4, 150}, {NOTE_DS4, 200}, {NOTE_F4, 200},
    {NOTE_AS4, 300}, {NOTE_G4, 350}, {REST, 150},

    // Refrão Épico: "Highway to the Danger Zone!"
    {NOTE_G4, 250}, {NOTE_AS4, 250}, {NOTE_C5, 400}, {NOTE_AS4, 250},
    {NOTE_G4, 250}, {NOTE_F4, 250}, {NOTE_DS4, 250}, {NOTE_C4, 500}, {REST, 150},

    // "Ride into the Danger Zone!"
    {NOTE_G4, 250}, {NOTE_AS4, 250}, {NOTE_C5, 400}, {NOTE_AS4, 250},
    {NOTE_C5, 300}, {NOTE_D5, 300}, {NOTE_DS5, 600}, {REST, 150},

    // Solo Top Gun Anthem (Harold Faltermeyer)
    {NOTE_C4, 250}, {NOTE_G4, 250}, {NOTE_F4, 200}, {NOTE_G4, 500},
    {NOTE_D5, 300}, {NOTE_C5, 300}, {NOTE_G4, 500},
    {NOTE_F4, 250}, {NOTE_G4, 250}, {NOTE_AS4, 300}, {NOTE_C5, 750}
};
const int LEN_TOP_GUN = sizeof(SONG_TOP_GUN) / sizeof(Note);

// -------------------------------------------------------------
// 8. SPACE ARCADE / SPACE INVADERS & GALAGA (ARCADE ESCAPE 1980)
// -------------------------------------------------------------
const Note PROGMEM SONG_ARCADE_SPACE[] = {
    // Fanfarra Galaga Clássica
    {NOTE_C4, 120}, {NOTE_G3, 120}, {NOTE_E3, 120}, {NOTE_C3, 180},
    {NOTE_D3, 120}, {NOTE_A3, 120}, {NOTE_F3, 120}, {NOTE_D3, 180},
    {NOTE_E3, 120}, {NOTE_B3, 120}, {NOTE_G3, 120}, {NOTE_E3, 180},
    {NOTE_F3, 120}, {NOTE_C4, 120}, {NOTE_A3, 120}, {NOTE_F3, 180},

    // Batalha Space Invaders Acelerando
    {NOTE_A4, 110}, {NOTE_E4, 110}, {NOTE_A4, 110}, {NOTE_C5, 160},
    {NOTE_B4, 110}, {NOTE_G4, 110}, {NOTE_B4, 110}, {NOTE_D5, 160},
    {NOTE_C5, 110}, {NOTE_A4, 110}, {NOTE_E5, 220}, {NOTE_D5, 160}, {NOTE_C5, 160},
    {NOTE_B4, 160}, {NOTE_A4, 160}, {NOTE_GS4, 220}, {NOTE_E4, 280},

    // Fanfarra Vitória Arcade 8-Bit
    {NOTE_A4, 120}, {NOTE_C5, 120}, {NOTE_E5, 150}, {NOTE_A5, 350},
    {NOTE_G5, 120}, {NOTE_E5, 120}, {NOTE_G5, 150}, {NOTE_E5, 350},
    {NOTE_C5, 120}, {NOTE_D5, 120}, {NOTE_E5, 120}, {NOTE_D5, 120},
    {NOTE_C5, 200}, {NOTE_B4, 200}, {NOTE_A4, 650}
};
const int LEN_ARCADE_SPACE = sizeof(SONG_ARCADE_SPACE) / sizeof(Note);

// -------------------------------------------------------------
// 9. ANOS 80: TAKE ON ME (A-HA 1985 SYNTH-POP)
// -------------------------------------------------------------
const Note PROGMEM SONG_TAKE_ON_ME[] = {
    // Frase 1 do Teclado Sintetizador
    {NOTE_FS4, 150}, {NOTE_FS4, 150}, {NOTE_D4, 150}, {NOTE_B3, 150}, {REST, 120},
    {NOTE_B3, 150}, {REST, 120}, {NOTE_E4, 150}, {REST, 120},
    {NOTE_E4, 150}, {REST, 120}, {NOTE_E4, 150}, {NOTE_GS4, 150},
    {NOTE_GS4, 150}, {NOTE_A4, 150}, {NOTE_B4, 150},

    // Frase 2
    {NOTE_A4, 150}, {NOTE_A4, 150}, {NOTE_A4, 150}, {NOTE_E4, 150}, {REST, 120},
    {NOTE_D4, 150}, {REST, 120}, {NOTE_FS4, 150}, {REST, 120},
    {NOTE_FS4, 150}, {REST, 120}, {NOTE_FS4, 150}, {NOTE_E4, 150},
    {NOTE_E4, 150}, {NOTE_FS4, 150}, {NOTE_E4, 280},

    // Frase 3 com tom agudo clássico
    {NOTE_FS4, 150}, {NOTE_FS4, 150}, {NOTE_D4, 150}, {NOTE_B3, 150}, {REST, 120},
    {NOTE_B3, 150}, {REST, 120}, {NOTE_E4, 150}, {REST, 120},
    {NOTE_E4, 150}, {REST, 120}, {NOTE_E4, 150}, {NOTE_GS4, 150},
    {NOTE_GS4, 150}, {NOTE_A4, 150}, {NOTE_B4, 150},

    {NOTE_A4, 150}, {NOTE_A4, 150}, {NOTE_A4, 150}, {NOTE_E4, 150}, {REST, 120},
    {NOTE_D4, 150}, {REST, 120}, {NOTE_FS4, 150}, {REST, 120},
    {NOTE_FS4, 150}, {REST, 120}, {NOTE_FS4, 150}, {NOTE_E4, 150},
    {NOTE_E4, 150}, {NOTE_FS4, 150}, {NOTE_E4, 500}
};
const int LEN_TAKE_ON_ME = sizeof(SONG_TAKE_ON_ME) / sizeof(Note);

#endif // SONGS_8BIT_H

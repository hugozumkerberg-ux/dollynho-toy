// ============================================================
// GABINETE PARAMETRICO PARA ROBO ASSISTENTE ESP32-S3 N16R8
// ============================================================
// Projetado para:
// - Tela OLED 0.96" I2C (128x64)
// - ESP32-S3 DevKit
// - LEDs bochecha/laterais (5mm)
// - Potenciômetro rotativo (7mm)
// - Grade acústica para Buzzer
// - Entrada USB-C traseira
// ============================================================

$fn = 60; // Suavidade das curvas cilíndricas

// Dimensoes da Caixa (mm)
largura = 75;
profundidade = 65;
altura = 42;
espessura = 2.5;
raio_canto = 4;

// Dimensoes dos Componentes
oled_largura = 27.5;
oled_altura = 19.5;
pot_diametro = 7.2;
led_diametro = 5.2;

// ------------------------------------------------------------
// Modulo: Corpo Arredondado
// ------------------------------------------------------------
module bloco_arredondado(l, p, a, r) {
    hull() {
        translate([-l/2 + r, -p/2 + r, 0]) cylinder(r=r, h=a);
        translate([ l/2 - r, -p/2 + r, 0]) cylinder(r=r, h=a);
        translate([-l/2 + r,  p/2 - r, 0]) cylinder(r=r, h=a);
        translate([ l/2 - r,  p/2 - r, 0]) cylinder(r=r, h=a);
    }
}

// ------------------------------------------------------------
// PARTE 1: Caixa Principal (Base)
// ------------------------------------------------------------
module caixa_base() {
    difference() {
        // Corpo externo
        bloco_arredondado(largura, profundidade, altura, raio_canto);

        // Oco interno
        translate([0, 0, espessura])
            bloco_arredondado(largura - 2*espessura, profundidade - 2*espessura, altura, raio_canto - 1);

        // --- Recorte Frontal para Tela OLED 0.96" ---
        translate([0, -profundidade/2, altura/2 + 2])
            rotate([90, 0, 0])
            cube([oled_largura, oled_altura, espessura*3], center=true);

        // --- Furos para LEDs Laterais (Bochechas de Expressão) ---
        // Lado Esquerdo
        translate([-largura/2, 0, altura/2])
            rotate([0, 90, 0])
            cylinder(d=led_diametro, h=espessura*3, center=true);
        translate([-largura/2, 10, altura/2])
            rotate([0, 90, 0])
            cylinder(d=led_diametro, h=espessura*3, center=true);

        // Lado Direito
        translate([largura/2, 0, altura/2])
            rotate([0, 90, 0])
            cylinder(d=led_diametro, h=espessura*3, center=true);
        translate([largura/2, 10, altura/2])
            rotate([0, 90, 0])
            cylinder(d=led_diametro, h=espessura*3, center=true);

        // --- Recorte Traseiro para Portas USB Duplas (COM e USB Nativo) ---
        // ESP32-S3 DevKit possui 2 conectores USB-C lado a lado
        translate([0, profundidade/2, espessura + 5])
            rotate([90, 0, 0])
            hull() {
                translate([-12, 0, 0]) cylinder(d=7, h=espessura*3, center=true);
                translate([ 12, 0, 0]) cylinder(d=7, h=espessura*3, center=true);
            }

        // --- Furo do Potenciômetro no Topo Dianteiro ---
        translate([22, -profundidade/4, altura - espessura/2])
            cylinder(d=pot_diametro, h=espessura*2, center=true);

        // --- Grade Acústica do Buzzer (Micro Furos) ---
        for (i = [-2 : 2]) {
            for (j = [-2 : 2]) {
                translate([-22 + i*3, -profundidade/4 + j*3, altura - espessura/2])
                    cylinder(d=1.5, h=espessura*2, center=true);
            }
        }
    }

    // Torres de Fixacao da Tela OLED 0.96" (4 furos M2, espacamento 24x24mm)
    oled_hole_dx = 24.0 / 2;
    oled_hole_dz = 23.5 / 2;
    for (ox = [-oled_hole_dx, oled_hole_dx]) {
        for (oz = [-oled_hole_dz, oled_hole_dz]) {
            difference() {
                translate([ox, -profundidade/2 + espessura, altura/2 + 2 + oz])
                    rotate([90, 0, 0])
                    cylinder(d=4.5, h=4); // Suporte de encosto da placa
                translate([ox, -profundidade/2 + espessura + 0.1, altura/2 + 2 + oz])
                    rotate([90, 0, 0])
                    cylinder(d=1.8, h=6); // Furo para parafuso M2
            }
        }
    }

    // Torres de Fixacao da Tampa nos 4 Cantos (Rosca M3)
    for (x = [-largura/2 + 8, largura/2 - 8]) {
        for (y = [-profundidade/2 + 8, profundidade/2 - 8]) {
            difference() {
                translate([x, y, espessura])
                    cylinder(d=6.5, h=altura - espessura);
                translate([x, y, espessura])
                    cylinder(d=2.8, h=altura); // Furo guia para parafuso autoatarraxante M3
            }
        }
    }
}

// ------------------------------------------------------------
// PARTE 2: Tampa Traseira / Inferior de Fechamento
// ------------------------------------------------------------
module tampa_fechamento() {
    difference() {
        bloco_arredondado(largura - 0.4, profundidade - 0.4, espessura, raio_canto);

        // Furos para fixacao da tampa
        for (x = [-largura/2 + 8, largura/2 - 8]) {
            for (y = [-profundidade/2 + 8, profundidade/2 - 8]) {
                translate([x, y, -1])
                    cylinder(d=3.4, h=espessura + 2);
                translate([x, y, espessura - 1.2])
                    cylinder(d=6, h=2); // Rebaixo para cabeca de parafuso
            }
        }
    }
}

// Renderiza a Caixa
caixa_base();

// Para visualizar a tampa separada, descomente a linha abaixo:
// translate([largura + 15, 0, 0]) tampa_fechamento();

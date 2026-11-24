// ============================================================
// GABINETE RETRO PC WIN95 EM 2 PARTES (CHASSI EM L + CAPA)
// ============================================================
// PARTE 1: Chassi em "L" (Base onde assenta o ESP32 + Frente da Tela)
// PARTE 2: Capa Traseira (Laterais esquerda/direita, teto e traseira)
// ============================================================

$fn = 50;

largura = 74;       // Largura total externa (mm)
profundidade = 70;  // Profundidade da base (mm)
altura = 60;        // Altura frontal (mm)
espessura = 2.2;    // Espessura das paredes (mm)

// Tela OLED 0.96"
oled_w = 26.5;
oled_h = 16.5;
oled_dx = 24.0 / 2;
oled_dz = 23.5 / 2;

// ESP32-S3 DevKit (suportes na base)
esp_furo_x = 24.0 / 2;
esp_furo_y = 50.0 / 2;

// ------------------------------------------------------------
// PARTE 1: Chassi em "L" (Frente + Base)
// ------------------------------------------------------------
module chassi_em_L() {
    difference() {
        union() {
            // Piso / Base Horizontal
            translate([0, profundidade/2, espessura/2])
                cube([largura, profundidade, espessura], center=true);

            // Parede Frontal Vertical
            translate([0, espessura/2, altura/2])
                cube([largura, espessura, altura], center=true);

            // Reforço em triângulo nas bordas da quina do L
            translate([-largura/2 + espessura, 0, 0])
                rotate([0, -90, 0])
                linear_extrude(espessura)
                polygon([[espessura, espessura], [espessura, 16], [16, espessura]]);

            translate([largura/2, 0, 0])
                rotate([0, -90, 0])
                linear_extrude(espessura)
                polygon([[espessura, espessura], [espessura, 16], [16, espessura]]);

            // Torres de fixação da tela OLED (parafusar de DENTRO para FORA)
            for (ox = [-oled_dx, oled_dx]) {
                for (oz = [-oled_dz, oled_dz]) {
                    translate([ox, espessura, altura/2 + 6 + oz])
                        rotate([-90, 0, 0])
                        difference() {
                            cylinder(d=4.5, h=4);
                            cylinder(d=1.8, h=6); // Furo M2
                        }
                }
            }

            // Torres na base para fixar o ESP32-S3
            for (ex = [-esp_furo_x, esp_furo_x]) {
                for (ey = [-esp_furo_y, esp_furo_y]) {
                    translate([ex, profundidade/2 + ey + 5, espessura])
                        difference() {
                            cylinder(d=5, h=4);
                            cylinder(d=2.0, h=6); // Furo M2.5/M3
                        }
                }
            }
        }

        // Janela da Tela OLED Frontal
        translate([0, espessura/2, altura/2 + 6])
            cube([oled_w, espessura*3, oled_h], center=true);

        // Furos para LEDs (bochechas nos cantos frontais)
        translate([-largura/3, espessura/2, altura/2 + 6])
            rotate([90, 0, 0]) cylinder(d=5.2, h=espessura*3, center=true);
        translate([largura/3, espessura/2, altura/2 + 6])
            rotate([90, 0, 0]) cylinder(d=5.2, h=espessura*3, center=true);

        // Furo do Potenciômetro (Queixo frontal retrô estilo botão de volume)
        translate([largura/4, espessura/2, 16])
            rotate([90, 0, 0]) cylinder(d=7.2, h=espessura*3, center=true);

        // Grade de ventilação / Buzzer
        for (i = [-3 : 3]) {
            translate([-largura/4 + i*2.8, espessura/2, 16])
                rotate([90, 0, 0]) cylinder(d=1.4, h=espessura*3, center=true);
        }
    }
}

// ------------------------------------------------------------
// PARTE 2: Capa Traseira (Costas + Laterais esquerda/direita + Teto)
// ------------------------------------------------------------
module capa_traseira() {
    folga = 0.3; // Tolerância para encaixe perfeito
    c_largura = largura + folga*2 + espessura*2;
    c_prof = profundidade + espessura;
    c_altura = altura + folga + espessura;

    difference() {
        // Bloco externo da capa
        translate([0, c_prof/2, c_altura/2])
            cube([c_largura, c_prof, c_altura], center=true);

        // Oco interno onde o chassi em L desliza
        translate([0, (profundidade)/2 + espessura/2 - 0.5, (altura)/2 + espessura/2])
            cube([largura + folga*2, profundidade + 2, altura + folga*2], center=true);

        // Abertura frontal vazada (onde a frente do L aparece)
        translate([0, -1, c_altura/2])
            cube([largura + folga*2 + 0.1, espessura*3, c_altura + 2], center=true);

        // Fundo aberto (onde a base do L assenta no chão)
        translate([0, c_prof/2, -1])
            cube([largura + folga*2 + 0.1, c_prof + 2, espessura*2 + 2], center=true);

        // Recorte Traseiro para as Duas Portas USB-C (COM + USB)
        translate([0, c_prof + 0.1, espessura + 8])
            rotate([90, 0, 0])
            hull() {
                translate([-11, 0, 0]) cylinder(d=7.5, h=espessura*3, center=true);
                translate([ 11, 0, 0]) cylinder(d=7.5, h=espessura*3, center=true);
            }

        // Fendas de ar retrô nas laterais (estilo gabinete anos 90)
        for (side = [-1, 1]) {
            for (j = [0 : 5]) {
                translate([side * (c_largura/2), c_prof/2 + (j - 2.5)*6, c_altura/2])
                    cube([espessura*3, 2, 16], center=true);
            }
        }
    }
}

// ------------------------------------------------------------
// RENDERIZAÇÃO
// ------------------------------------------------------------
// Para visualizar montado, deixe as duas linhas ativas:
chassi_em_L();

// Para visualizar a capa separada para imprimir, descomente abaixo:
translate([largura + 20, 0, 0]) capa_traseira();

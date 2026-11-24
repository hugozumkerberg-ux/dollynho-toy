// ============================================================
// GABINETE RETRO CRT (MONITOR DE TUBO) PARA ESP32-S3 + OLED 0.96"
// ============================================================

$fn = 60;

// Dimensoes do Monitor Retrô (mm)
largura_frente = 76;
altura_frente = 62;
largura_traseira = 56;
altura_traseira = 48;
profundidade = 68;
espessura = 2.4;

// Tela OLED 0.96"
oled_w = 26.5;
oled_h = 16.5;

module corpo_crt() {
    hull() {
        // Moldura Frontal do Monitor
        translate([0, -profundidade/2, 0])
            minkowski() {
                cube([largura_frente - 8, 1, altura_frente - 8], center=true);
                cylinder(r=4, h=1, center=true);
            }
        // Traseira Afunilada do Tubo
        translate([0, profundidade/2 - 2, 4])
            minkowski() {
                cube([largura_traseira - 8, 1, altura_traseira - 8], center=true);
                cylinder(r=4, h=1, center=true);
            }
    }
}

module monitor_crt() {
    difference() {
        // Corpo externo do monitor de tubo
        corpo_crt();

        // Oco interno para abrigar ESP32-S3 e componentes
        translate([0, 0, 0])
            scale([(largura_frente - 2*espessura)/largura_frente, 
                   (profundidade - 2*espessura)/profundidade, 
                   (altura_frente - 2*espessura)/altura_frente])
                corpo_crt();

        // --- Bezel / Recorte Curvado da Tela Frontal ---
        translate([0, -profundidade/2, 6])
            rotate([90, 0, 0])
            minkowski() {
                cube([oled_w - 4, oled_h - 4, espessura*3], center=true);
                cylinder(r=2, h=1, center=true);
            }

        // --- Bochechas / LEDs Laterais (5mm) ---
        translate([-largura_frente/2 + 2, -profundidade/4, 6])
            rotate([0, 90, 0]) cylinder(d=5.2, h=10, center=true);
        translate([largura_frente/2 - 2, -profundidade/4, 6])
            rotate([0, 90, 0]) cylinder(d=5.2, h=10, center=true);

        // --- Botao Rotativo Retrô (Potenciômetro no Queixo) ---
        translate([largura_frente/4, -profundidade/2 + 1, -altura_frente/4])
            rotate([90, 0, 0])
            cylinder(d=7.2, h=espessura*3, center=true);

        // --- Grelha de Ventilação / Buzzer no Queixo ---
        for (i = [-3 : 3]) {
            translate([-largura_frente/4 + i*3, -profundidade/2 + 1, -altura_frente/4])
                rotate([90, 0, 0])
                cylinder(d=1.4, h=espessura*3, center=true);
        }

        // --- Recorte Traseiro Largo para as Duas Portas USB-C (COM + USB) ---
        translate([0, profundidade/2, -altura_traseira/4])
            rotate([90, 0, 0])
            hull() {
                translate([-10, 0, 0]) cylinder(d=7.5, h=espessura*3, center=true);
                translate([ 10, 0, 0]) cylinder(d=7.5, h=espessura*3, center=true);
            }
    }

    // --- Torres Internas de Fixação para Parafusar de DENTRO para FORA ---
    // Encostos para os 4 cantos da placa OLED (furos M2)
    oled_dx = 24.0 / 2;
    oled_dz = 23.5 / 2;
    for (ox = [-oled_dx, oled_dx]) {
        for (oz = [-oled_dz, oled_dz]) {
            difference() {
                translate([ox, -profundidade/2 + espessura + 1.5, 6 + oz])
                    rotate([90, 0, 0])
                    cylinder(d=4.6, h=3.5, center=true);
                translate([ox, -profundidade/2 + espessura + 1.5, 6 + oz])
                    rotate([90, 0, 0])
                    cylinder(d=1.8, h=6, center=true); // Rosca M2
            }
        }
    }
}

monitor_crt();

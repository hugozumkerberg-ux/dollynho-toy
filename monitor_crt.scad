// ============================================================
// PARTE 1: MONITOR CRT RETRO COM 4 LEDS NO FUNDO
// ============================================================
$fn = 50;

largura = 72;
altura = 56;
profundidade = 48;
espessura = 2.2;

// Tela OLED 0.96"
oled_w = 26.5;
oled_h = 16.5;
oled_dx = 24.0 / 2;
oled_dz = 23.5 / 2;

module monitor_crt() {
    difference() {
        // Corpo arredondado do monitor
        minkowski() {
            cube([largura - 8, profundidade - 8, altura - 8], center=true);
            cylinder(r=4, h=2, center=true);
        }

        // Oco interno
        translate([0, 0, 0])
            cube([largura - 2*espessura, profundidade - 2*espessura, altura - 2*espessura], center=true);

        // Abertura da Tela Frontal (Chanfrada estilo tubo CRT)
        translate([0, -profundidade/2, 4])
            rotate([90, 0, 0])
            cube([oled_w, oled_h, espessura*3], center=true);

        // --- 4 LEDs de 5mm no Fundo / Traseira (Iluminacao ambiente/emocional) ---
        for (i = [-1.5, -0.5, 0.5, 1.5]) {
            translate([i * 14, profundidade/2, 6])
                rotate([90, 0, 0])
                cylinder(d=5.2, h=espessura*3, center=true);
        }

        // Passagem inferior de cabos para conectar na base do teclado
        translate([0, 0, -altura/2])
            cube([36, 20, espessura*3], center=true);
    }

    // 4 Torres internas de fixacao da tela (parafusar de DENTRO para FORA)
    for (ox = [-oled_dx, oled_dx]) {
        for (oz = [-oled_dz, oled_dz]) {
            translate([ox, -profundidade/2 + espessura + 1.5, 4 + oz])
                rotate([90, 0, 0])
                difference() {
                    cylinder(d=4.6, h=3.5, center=true);
                    cylinder(d=1.8, h=6, center=true); // Furo M2
                }
        }
    }

    // Pinos de encaixe na base do teclado
    for (px = [-largura/3, largura/3]) {
        translate([px, 0, -altura/2 - 2.5])
            cylinder(d=3.8, h=5, center=true);
    }
}

monitor_crt();

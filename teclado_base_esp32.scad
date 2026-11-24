// ============================================================
// PARTE 2: TECLADINHO RETRO (BASE ONDE ESCONDE O ESP32-S3)
// ============================================================
$fn = 50;

largura = 86;
profundidade = 76;
altura_traseira = 26;
altura_frente = 12;
espessura = 2.2;

module teclado_base() {
    difference() {
        // Bloco inclinado estilo rampa de teclado antigo
        hull() {
            // Frente baixa
            translate([0, -profundidade/2 + 4, altura_frente/2])
                minkowski() {
                    cube([largura - 8, 2, altura_frente - 4], center=true);
                    cylinder(r=3, h=1, center=true);
                }
            // Traseira mais alta
            translate([0, profundidade/2 - 4, altura_traseira/2])
                minkowski() {
                    cube([largura - 8, 2, altura_traseira - 4], center=true);
                    cylinder(r=3, h=1, center=true);
                }
        }

        // Oco interno para o ESP32-S3
        translate([0, 0, espessura])
            hull() {
                translate([0, -profundidade/2 + 6, (altura_frente - 2*espessura)/2])
                    cube([largura - 2*espessura - 4, 2, altura_frente - 2*espessura], center=true);
                translate([0, profundidade/2 - 6, (altura_traseira - 2*espessura)/2])
                    cube([largura - 2*espessura - 4, 2, altura_traseira - 2*espessura], center=true);
            }

        // Furos de encaixe para os pinos do Monitor CRT
        for (px = [-72/3, 72/3]) {
            translate([px, 6, altura_traseira - 4])
                cylinder(d=4.2, h=12, center=true);
        }

        // Passagem central para os fios da tela e LEDs descerem até o ESP32
        translate([0, 6, altura_traseira - 4])
            cube([32, 18, 14], center=true);

        // Abertura traseira para os 2 conectores USB-C do ESP32 (COM + USB)
        translate([0, profundidade/2, 10])
            rotate([90, 0, 0])
            hull() {
                translate([-11, 0, 0]) cylinder(d=7.5, h=espessura*3, center=true);
                translate([ 11, 0, 0]) cylinder(d=7.5, h=espessura*3, center=true);
            }

        // Detalhe de relevo visual de teclas no topo dianteiro (estilo teclado retrô)
        for (row = [0 : 2]) {
            for (col = [-3 : 3]) {
                translate([col * 9.5, -profundidade/4 + row * 8.5, altura_frente + 1.2])
                    cube([7.5, 6.5, 1.2], center=true);
            }
        }
    }

    // Suportes de apoio interno para a plaquinha ESP32-S3
    for (sx = [-24/2, 24/2]) {
        for (sy = [-40/2, 40/2]) {
            translate([sx, sy - 4, espessura])
                difference() {
                    cylinder(d=5.5, h=3.5);
                    cylinder(d=2.0, h=5);
                }
        }
    }
}

teclado_base();

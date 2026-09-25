/*
 * Film_Clap_Board - Professional Cinema Edition (Seeed_GFX2)
 * Hardware: Seeed Studio XIAO ESP32-S3 Plus + XIAO ePaper Display Board (EE04)
 * Panel: 7.5-inch Monochrome ePaper (800x480, UC8179)
 *
 * Características:
 *  - Sin Timecode (diseño limpio y espacioso)
 *  - Sincronización automática de FECHA desde el reloj interno del smartphone/tablet
 *  - Soporte para el emoji de flor de cerezo (🌸) en mapa de bits monocromático de 32x32
 *  - Metadatos de cámara y sonido: CAM (A/B/C), SOUND (SYNC/MOS)
 *  - Metadatos de rodaje: INT/EXT y DAY/NIGHT con badges visuales de alto contraste
 *  - Botón Key1: +1 TAKE | Botón Key3: +1 ROLL
 *  - Acceso inmediato vía http://clap.local (o http://192.168.4.1)
 *  - Mayúsculas sostenidas forzadas en todos los campos
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Seeed_GFX.h>

// Instancia Seeed_GFX2 para el panel 7.5" en la placa EE04
Seeed_GFX display(Seeed_Product::Seeed_ePaper_7INCH5);

// Definición de pines para botones físicos de la placa EE04 (activos en nivel BAJO)
const int PIN_BTN_TAKE_1 = 3; // KEY1 (GPIO3)
const int PIN_BTN_TAKE_0 = 2; // KEY0 (GPIO2) - respaldo
const int PIN_BTN_ROLL   = 5; // KEY3 (GPIO5)

// Configuración Wi-Fi SoftAP
const char* ssid = "NEBRALTA_Clap";
const char* password = "123456789P";

WebServer server(80);

// Campos Principales de la Claqueta
String roll = "A001";
String scene = "35A";
String take = "1";
String prod = "NEBRALTA 🌸";
String director = "JONATHAN LOPEZ";
String dop = "FELIPE SALAS";
String note = "ISO 120";
String date = "25.09.2026";

// Metadatos de Cámara, Audio y Rodaje
String cam = "A";
String sound = "SYNC";       // "SYNC" o "MOS"
String loc = "INT";          // "INT" o "EXT"
String light = "DAY";        // "DAY" o "NIGHT"

IPAddress localIP;

// Prototipos
void updateDisplay();
void incrementTake();
void incrementRoll();

// =========================================================================
// MAPA DE BITS MONOCROMÁTICO DE 32x32 PÍXELES PARA EL EMOJI 🌸
// =========================================================================
const unsigned char epd_bitmap_sakura[128] PROGMEM = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x04, 0x20, 0x00,
  0x00, 0x0F, 0xF0, 0x00,
  0x00, 0x0F, 0xF0, 0x00,
  0x00, 0x1F, 0xF8, 0x00,
  0x00, 0x1F, 0xF8, 0x00,
  0x00, 0x0F, 0xF0, 0x00,
  0x07, 0xC7, 0xE3, 0xE0,
  0x0F, 0xC3, 0xC3, 0xF0,
  0x07, 0xE1, 0x87, 0xE0,
  0x07, 0xE0, 0x07, 0xE0,
  0x0F, 0xF0, 0x0F, 0xF0,
  0x1F, 0xF3, 0xCF, 0xF8,
  0x0F, 0xE3, 0xC7, 0xF0,
  0x0F, 0x83, 0xC1, 0xF0,
  0x03, 0x03, 0xC0, 0xC0,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x3C, 0x3C, 0x00,
  0x01, 0xFC, 0x3F, 0x80,
  0x03, 0xFC, 0x3F, 0xC0,
  0x03, 0xFC, 0x3F, 0xC0,
  0x03, 0xFE, 0x7F, 0xC0,
  0x03, 0xFE, 0x7F, 0xC0,
  0x00, 0x7E, 0x7E, 0x00,
  0x00, 0x3C, 0x3C, 0x00,
  0x00, 0x30, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Secuencia UTF-8 de 4 bytes para el emoji 🌸
const String SAKURA_UTF8 = "\xF0\x9F\x8C\xB8";

/**
 * Calcula el ancho real en píxeles de una línea, contando el bitmap de 🌸 como 36px
 */
int getLineWidthWithSakura(const String& line, int size) {
  int w = 0;
  int pos = 0;
  while (pos < (int)line.length()) {
    int nextEmoji = line.indexOf(SAKURA_UTF8, pos);
    if (nextEmoji == -1) {
      w += (line.length() - pos) * 6 * size;
      break;
    } else {
      w += (nextEmoji - pos) * 6 * size;
      w += 32 + 6; // Icono de 32px + 6px de margen
      pos = nextEmoji + SAKURA_UTF8.length();
    }
  }
  return w;
}

/**
 * Dibuja una línea de texto renderizando el emoji 🌸 como icono gráfico
 */
void drawLineWithSakura(const String& line, int startX, int startY, int size) {
  int curX = startX;
  int pos = 0;
  int len = line.length();

  while (pos < len) {
    int nextEmoji = line.indexOf(SAKURA_UTF8, pos);
    if (nextEmoji == -1) {
      String sub = line.substring(pos);
      display.setTextSize(size);
      display.drawString(sub, curX, startY);
      curX += sub.length() * 6 * size;
      break;
    } else {
      if (nextEmoji > pos) {
        String sub = line.substring(pos, nextEmoji);
        display.setTextSize(size);
        display.drawString(sub, curX, startY);
        curX += sub.length() * 6 * size;
      }
      
      // Alinear verticalmente el icono de 32x32 con la altura del texto
      int textH = 8 * size;
      int iconY = startY + ((textH - 32) / 2);
      if (iconY < startY) iconY = startY;

      display.drawBitmap(curX + 2, iconY, epd_bitmap_sakura, 32, 32, TFT_BLACK);
      curX += 32 + 6;
      pos = nextEmoji + SAKURA_UTF8.length();
    }
  }
}

/**
 * Dibuja un badge rectangular activo/inactivo de alto contraste
 */
void drawBadge(const String& label, int x, int y, int w, int h, bool active) {
  display.setFreeFont(nullptr);
  if (active) {
    display.fillRect(x, y, w, h, TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
  } else {
    display.drawRect(x, y, w, h, TFT_BLACK);
    display.setTextColor(TFT_BLACK, TFT_WHITE);
  }
  display.setTextSize(2);
  display.drawCentreString(label, x + (w / 2), y + ((h - 14) / 2), 1);
}

/**
 * Dibuja texto con ajuste automático inteligente:
 * - Evalúa primero si cabe en UNA SOLA LÍNEA (probando tamaños desde defaultSize hacia abajo).
 * - Si cabe en una sola línea con un tamaño grande/óptimo (>= 4), se mantiene en 1 línea más grande y sin cortes.
 * - Si el texto es excesivamente largo, busca un espacio para partirlo en 2 líneas.
 * - Centra verticalmente el texto dentro del espacio disponible (maxH).
 * - Soporta el emoji 🌸 y fuerza mayúsculas sostenidas.
 */
void drawTextAuto(const String& text, int x, int y, int maxW, int maxH, int defaultSize, int minSize = 2) {
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setFreeFont(nullptr);

  String uText = text;
  uText.toUpperCase();
  uText.trim();

  if (uText.length() == 0) return;

  // 1. Evaluar el tamaño máximo que cabe en UNA SOLA LÍNEA
  int bestSingleSize = -1;
  for (int s = defaultSize; s >= minSize; s--) {
    int w = getLineWidthWithSakura(uText, s);
    int h = 8 * s;
    if (w <= maxW && h <= maxH) {
      bestSingleSize = s;
      break;
    }
  }

  // 2. Evaluar el tamaño máximo que cabría en DOS LÍNEAS
  int bestDoubleSize = -1;
  String line1 = "", line2 = "";
  int splitIdx = -1;

  int mid = uText.length() / 2;
  while (mid < (int)uText.length() && (uText[mid] & 0xC0) == 0x80) {
    mid++; // Evitar romper caracteres UTF-8 multi-byte
  }

  for (int i = 0; i < (int)uText.length(); i++) {
    if (uText[i] == ' ') {
      if (splitIdx == -1 || abs(i - mid) < abs(splitIdx - mid)) {
        splitIdx = i;
      }
    }
  }

  if (splitIdx > 0 && splitIdx < (int)uText.length() - 1) {
    line1 = uText.substring(0, splitIdx);
    line2 = uText.substring(splitIdx + 1);
    line1.trim();
    line2.trim();

    for (int s = defaultSize - 1; s >= minSize; s--) {
      int w1 = getLineWidthWithSakura(line1, s);
      int w2 = getLineWidthWithSakura(line2, s);
      int totalH = (8 * s * 2) + 4;
      if (w1 <= maxW && w2 <= maxW && totalH <= maxH) {
        bestDoubleSize = s;
        break;
      }
    }
  }

  // 3. Decisión inteligente: Priorizar UNA SOLA LÍNEA siempre que sea posible
  bool useSingleLine = false;
  if (bestSingleSize != -1) {
    if (bestDoubleSize == -1) {
      // No hay espacios o no cabe en 2 líneas -> 1 línea obligatoria
      useSingleLine = true;
    } else if (bestSingleSize >= bestDoubleSize) {
      // En 1 línea es igual o más grande que en 2 líneas -> 1 línea indiscutible
      useSingleLine = true;
    } else if (bestSingleSize >= 5) {
      // Con tamaño 5 o superior (>= 40px de alto), se ve enorme en 1 línea
      useSingleLine = true;
    } else if (bestSingleSize >= 4 && (bestDoubleSize - bestSingleSize) <= 1) {
      // Con tamaño 4 (letras de 32px de alto), si 2 líneas solo daría 5, preferimos no partir
      useSingleLine = true;
    }
  }

  if (useSingleLine) {
    int textH = 8 * bestSingleSize;
    int drawY = y + ((maxH - textH) / 2);
    if (drawY < y) drawY = y;
    drawLineWithSakura(uText, x, drawY, bestSingleSize);
  } else if (bestDoubleSize != -1) {
    int lineH = 8 * bestDoubleSize;
    int gap = 4;
    int totalH = (lineH * 2) + gap;
    int drawY = y + ((maxH - totalH) / 2);
    if (drawY < y) drawY = y;
    drawLineWithSakura(line1, x, drawY, bestDoubleSize);
    drawLineWithSakura(line2, x, drawY + lineH + gap, bestDoubleSize);
  } else {
    int s = (bestSingleSize != -1) ? bestSingleSize : minSize;
    drawLineWithSakura(uText, x, y, s);
  }
}

// Pantalla de bienvenida en el arranque
void showIPOnBoot() {
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setFreeFont(nullptr);

  display.drawRect(20, 20, 760, 440, TFT_BLACK);
  display.drawRect(22, 22, 756, 436, TFT_BLACK);

  display.setTextSize(4);
  display.drawCentreString("E-SLATE DIGITAL CLAPPER", 400, 50, 1);
  display.drawLine(50, 105, 750, 105, TFT_BLACK);

  display.setTextSize(3);
  display.drawString("1. Conectate a la red Wi-Fi:", 60, 130);
  display.setTextSize(4);
  display.drawString(String("\"") + ssid + "\"", 100, 170);

  display.setTextSize(3);
  display.drawString("2. Entra en tu navegador a:", 60, 230);
  display.setTextSize(4);
  display.drawString("http://clap.local", 100, 275);
  display.setTextSize(3);
  display.drawString(String("(o http://") + localIP.toString() + ")", 100, 325);

  display.drawLine(50, 375, 750, 375, TFT_BLACK);
  display.setTextSize(2);
  display.drawString("Botones:  [Key1] = +1 TAKE   |   [Key3] = +1 ROLL", 70, 395);
  display.drawString(String("Pass Wi-Fi: ") + password, 70, 425);

  display.refresh();
  delay(3000);
}

// Renderizado de la claqueta cinematográfica en el panel 800x480
void updateDisplay() {
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setFreeFont(nullptr);

  // --- LÍNEAS DE LA GRILLA CINEMATOGRÁFICA ---
  // Líneas horizontales
  display.drawLine(0, 180, 800, 180, TFT_BLACK); // Fin fila superior (ROLL/SCENE/TAKE)
  display.drawLine(0, 275, 800, 275, TFT_BLACK); // Fin PROD
  display.drawLine(0, 370, 800, 370, TFT_BLACK); // Fin DIR / DOP

  // Líneas verticales
  display.drawLine(266, 0, 266, 180, TFT_BLACK); // Divisor ROLL-SCENE
  display.drawLine(548, 0, 548, 180, TFT_BLACK); // Divisor SCENE-TAKE
  display.drawLine(400, 275, 400, 480, TFT_BLACK); // Divisor central inferior
  display.drawLine(525, 370, 525, 480, TFT_BLACK); // Divisor CAM

  // ==========================================
  // BLOQUE 1: ROLL | SCENE | TAKE (y: 0..180)
  // ==========================================
  display.setTextSize(3);
  display.drawString("ROLL", 15, 12);
  display.drawString("SCENE", 280, 12);
  display.drawString("TAKE", 565, 12);

  drawTextAuto(roll, 25, 65, 230, 110, 8, 4);
  drawTextAuto(scene, 285, 65, 250, 110, 8, 4);
  drawTextAuto(take, 565, 65, 220, 110, 8, 4);

  // ==========================================
  // BLOQUE 2: PRODUCCIÓN (y: 180..275)
  // ==========================================
  display.setTextSize(3);
  display.drawString("PROD", 15, 215);
  // Ancho generoso de 680px para nombre y flor (x: 105..785)
  drawTextAuto(prod, 105, 185, 680, 85, 7, 3);

  // ==========================================
  // BLOQUE 3: DIR & DOP (y: 275..370)
  // ==========================================
  display.setTextSize(2);
  display.drawString("DIR", 15, 314);
  display.drawString("DOP", 415, 314);

  drawTextAuto(director, 65, 280, 325, 85, 5, 2);
  drawTextAuto(dop, 465, 280, 325, 85, 5, 2);

  // ==========================================
  // BLOQUE 4: DATE / NOTE | CAM | BADGES (y: 370..480)
  // ==========================================
  // Lado izquierdo (x: 0..400): DATE y NOTE
  display.setTextSize(2);
  display.drawString("DATE", 15, 385);
  display.setTextSize(4);
  display.drawString(date, 90, 380);

  display.setTextSize(2);
  display.drawString("NOTE", 15, 435);
  drawTextAuto(note, 90, 435, 300, 38, 3, 2);

  // Centro (x: 400..525): CAM
  display.setTextSize(2);
  display.drawString("CAM", 420, 385);
  display.setTextSize(6);
  display.drawString(cam, 445, 415);

  // Lado derecho (x: 525..800): BADGES DE AUDIO, LOCACIÓN Y LUZ
  // Fila superior: SOUND (SYNC vs MOS)
  bool isSync = (sound == "SYNC");
  drawBadge("SYNC", 540, 380, 120, 42, isSync);
  drawBadge("MOS", 670, 380, 115, 42, !isSync);

  // Fila inferior: INT/EXT y DAY/NIGHT
  bool isInt = (loc == "INT");
  drawBadge("INT", 540, 430, 58, 40, isInt);
  drawBadge("EXT", 602, 430, 58, 40, !isInt);

  bool isDay = (light == "DAY");
  drawBadge("DAY", 670, 430, 56, 40, isDay);
  drawBadge("NIGHT", 730, 430, 56, 40, !isDay);

  // Refrescar panel e-paper
  const GfxResult res = display.refresh();
  if (!res) {
    Serial.printf("[Seeed_GFX2 Error]: %s\n", res.message);
  } else {
    Serial.println("Display actualizado con exito.");
  }
}

// Incremento inteligente de TAKE (+1)
void incrementTake() {
  long val = take.toInt();
  if (val > 0 || take == "0") {
    take = String(val + 1);
  } else {
    int i = 0;
    while (i < (int)take.length() && isDigit(take[i])) i++;
    if (i > 0) {
      int num = take.substring(0, i).toInt() + 1;
      take = String(num) + take.substring(i);
    } else {
      take = "1";
    }
  }
  Serial.printf("[BOTON] TAKE -> %s\n", take.c_str());
  updateDisplay();
}

// Incremento inteligente de ROLL (+1)
void incrementRoll() {
  int numStart = -1;
  for (int i = 0; i < (int)roll.length(); i++) {
    if (isDigit(roll[i])) {
      numStart = i;
      break;
    }
  }

  if (numStart != -1) {
    String prefix = roll.substring(0, numStart);
    String numStr = roll.substring(numStart);
    int numLen = numStr.length();
    long nextNum = numStr.toInt() + 1;

    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%0%dld", numLen);
    char buf[32];
    snprintf(buf, sizeof(buf), fmt, nextNum);

    roll = prefix + String(buf);
  } else if (roll.length() == 1 && isAlpha(roll[0])) {
    char nextL = roll[0] + 1;
    if (nextL > 'Z' && roll[0] <= 'Z') nextL = 'A';
    roll = String(nextL);
  } else {
    roll = "A001";
  }

  Serial.printf("[BOTON] ROLL -> %s\n", roll.c_str());
  updateDisplay();
}

// Interfaz Web Móvil Profesional
String htmlPage() {
  String html = R"HTML(<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>E-Slate Cinema Pro</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: #0f0f11;
      color: #eee;
      padding: 12px;
      display: flex;
      justify-content: center;
      min-height: 100vh;
    }
    .container { max-width: 680px; width: 100%; margin: 0 auto; }
    .header { text-align: center; margin-bottom: 12px; }
    .header h1 { font-size: 1.6rem; letter-spacing: 2px; text-transform: uppercase; color: #fff; }
    .header p { font-size: 0.85rem; color: #8fc31f; font-weight: 700; }
    .card {
      background: #18181b;
      border: 1px solid #27272a;
      padding: 18px;
      border-radius: 12px;
      box-shadow: 0 8px 30px rgba(0,0,0,0.6);
    }
    .quick-actions {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin-bottom: 15px;
    }
    .btn-quick {
      background: #27272a;
      border: 1px solid #8fc31f;
      color: #8fc31f;
      padding: 12px;
      border-radius: 8px;
      font-weight: 800;
      font-size: 0.95rem;
      cursor: pointer;
      text-transform: uppercase;
    }
    .btn-quick:active { background: #8fc31f; color: #000; }
    .form-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
      margin-bottom: 14px;
    }
    .form-group { margin-bottom: 8px; }
    label {
      display: block;
      font-weight: 700;
      margin-bottom: 4px;
      color: #a1a1aa;
      font-size: 0.72rem;
      letter-spacing: 1px;
    }
    input, select {
      width: 100%;
      padding: 10px;
      border-radius: 6px;
      border: 1px solid #3f3f46;
      background: #27272a;
      color: #fff;
      font-size: 14px;
      font-weight: 600;
      text-transform: uppercase;
    }
    input:focus, select:focus {
      outline: none;
      border-color: #8fc31f;
      background: #323238;
    }
    .full-width { grid-column: 1 / -1; }
    .two-cols { grid-column: span 2; }
    
    .toggle-group {
      display: flex;
      gap: 6px;
    }
    .toggle-group button {
      flex: 1;
      padding: 9px;
      border: 1px solid #3f3f46;
      background: #27272a;
      color: #aaa;
      font-weight: 700;
      font-size: 0.8rem;
      border-radius: 6px;
      cursor: pointer;
    }
    .toggle-group button.active {
      background: #8fc31f;
      color: #000;
      border-color: #8fc31f;
    }
    
    .btn-submit {
      background: #8fc31f;
      color: #000;
      padding: 14px;
      border-radius: 8px;
      font-weight: 800;
      border: none;
      cursor: pointer;
      font-size: 1.05rem;
      letter-spacing: 1px;
      width: 100%;
      text-transform: uppercase;
      margin-top: 8px;
    }
    .btn-submit:hover { background: #9ee024; }
    .footer-links {
      text-align: center;
      margin-top: 14px;
      color: #71717a;
      font-size: 0.8rem;
    }
    .footer-links a { color: #8fc31f; text-decoration: none; font-weight: bold; }
    @media (max-width: 520px) {
      .form-grid { grid-template-columns: 1fr; }
      .two-cols { grid-column: span 1; }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🎬 E-Slate Cinema Pro</h1>
      <p>http://clap.local</p>
    </div>
    
    <div class="card">
      <div class="quick-actions">
        <form method="POST" action="/inc-roll" style="display:contents;">
          <button class="btn-quick" type="submit">➕ ROLL (+1)</button>
        </form>
        <form method="POST" action="/inc-take" style="display:contents;">
          <button class="btn-quick" type="submit">➕ TAKE (+1)</button>
        </form>
      </div>

      <form method="POST" action="/update" id="slateForm">
        <!-- Campos Principales -->
        <div class="form-grid">
          <div class="form-group">
            <label for="roll">ROLL</label>
            <input type="text" id="roll" name="roll" value=")HTML";
  html += roll;
  html += R"HTML(" maxlength="10">
          </div>
          <div class="form-group">
            <label for="scene">SCENE</label>
            <input type="text" id="scene" name="scene" value=")HTML";
  html += scene;
  html += R"HTML(" maxlength="10">
          </div>
          <div class="form-group">
            <label for="take">TAKE</label>
            <input type="text" id="take" name="take" value=")HTML";
  html += take;
  html += R"HTML(" maxlength="10">
          </div>
          <div class="form-group full-width">
            <label for="prod">PRODUCCIÓN (PROD)</label>
            <div style="display:flex; gap:6px;">
              <input type="text" id="prod" name="prod" value=")HTML";
  html += prod;
  html += R"HTML(" maxlength="40">
              <button type="button" onclick="insertSakura()" style="background:#27272a;border:1px solid #ec4899;color:#fff;border-radius:6px;padding:0 12px;font-size:1.1rem;cursor:pointer;">🌸</button>
            </div>
          </div>
          <div class="form-group two-cols">
            <label for="director">DIRECTOR (DIR)</label>
            <input type="text" id="director" name="director" value=")HTML";
  html += director;
  html += R"HTML(" maxlength="35">
          </div>
          <div class="form-group">
            <label for="dop">FOTOGRAFÍA (DOP)</label>
            <input type="text" id="dop" name="dop" value=")HTML";
  html += dop;
  html += R"HTML(" maxlength="35">
          </div>
        </div>

        <!-- Metadatos de Cámara, Audio y Rodaje -->
        <div class="form-grid">
          <div class="form-group">
            <label for="cam">CÁMARA (CAM)</label>
            <input type="text" id="cam" name="cam" value=")HTML";
  html += cam;
  html += R"HTML(" maxlength="5">
          </div>
          <div class="form-group">
            <label>AUDIO</label>
            <div class="toggle-group">
              <input type="hidden" name="sound" id="soundInput" value=")HTML";
  html += sound;
  html += R"HTML(">
              <button type="button" id="btnSync" onclick="setToggle('soundInput','SYNC',this,'btnMos')">SYNC</button>
              <button type="button" id="btnMos" onclick="setToggle('soundInput','MOS',this,'btnSync')">MOS</button>
            </div>
          </div>
          <div class="form-group">
            <label>LOCACIÓN</label>
            <div class="toggle-group">
              <input type="hidden" name="loc" id="locInput" value=")HTML";
  html += loc;
  html += R"HTML(">
              <button type="button" id="btnInt" onclick="setToggle('locInput','INT',this,'btnExt')">INT</button>
              <button type="button" id="btnExt" onclick="setToggle('locInput','EXT',this,'btnInt')">EXT</button>
            </div>
          </div>
          <div class="form-group">
            <label>LUZ</label>
            <div class="toggle-group">
              <input type="hidden" name="light" id="lightInput" value=")HTML";
  html += light;
  html += R"HTML(">
              <button type="button" id="btnDay" onclick="setToggle('lightInput','DAY',this,'btnNight')">DAY</button>
              <button type="button" id="btnNight" onclick="setToggle('lightInput','NIGHT',this,'btnDay')">NIGHT</button>
            </div>
          </div>
          <div class="form-group two-cols">
            <label for="date">FECHA</label>
            <div style="display:flex; gap:6px;">
              <input type="text" id="date" name="date" value=")HTML";
  html += date;
  html += R"HTML(" maxlength="15">
              <button type="button" onclick="syncPhoneDate()" title="Sincronizar fecha de este móvil" style="background:#27272a;border:1px solid #8fc31f;color:#8fc31f;border-radius:6px;padding:0 10px;font-size:0.75rem;cursor:pointer;font-weight:bold;">📱 HOY</button>
            </div>
            <label style="margin-top:6px; font-size:0.72rem; color:#8fc31f; display:flex; align-items:center; gap:6px; cursor:pointer;">
              <input type="checkbox" id="autoSyncDate" checked style="width:auto; margin:0; cursor:pointer;">
              Auto-sincronizar con reloj de este dispositivo al actualizar
            </label>
          </div>
          <div class="form-group full-width">
            <label for="note">NOTA / LENTE</label>
            <input type="text" id="note" name="note" value=")HTML";
  html += note;
  html += R"HTML(" maxlength="35">
          </div>
        </div>

        <button class="btn-submit" type="submit">Actualizar Claqueta</button>
      </form>
    </div>
    
    <div class="footer-links">
      Acceso web: <a href="http://clap.local">http://clap.local</a> | IP: <strong>)HTML";
  html += localIP.toString();
  html += R"HTML(</strong>
    </div>
  </div>

  <script>
    function setToggle(inputId, val, btnActive, btnOtherId) {
      document.getElementById(inputId).value = val;
      btnActive.classList.add('active');
      document.getElementById(btnOtherId).classList.remove('active');
    }

    function insertSakura() {
      const p = document.getElementById('prod');
      p.value = p.value ? p.value + ' 🌸' : '🌸';
    }

    // Obtener la fecha del reloj del smartphone / tablet
    function syncPhoneDate() {
      const now = new Date();
      const dd = String(now.getDate()).padStart(2, '0');
      const mm = String(now.getMonth() + 1).padStart(2, '0');
      const yyyy = now.getFullYear();
      const d = document.getElementById('date');
      if (d) d.value = `${dd}.${mm}.${yyyy}`;
    }

    document.addEventListener('DOMContentLoaded', function() {
      // Auto-sincronizar fecha al cargar la página si la casilla está activa
      const autoSync = document.getElementById('autoSyncDate');
      if (autoSync && autoSync.checked) {
        syncPhoneDate();
      }

      // Asegurar fecha actual al enviar el formulario
      const form = document.getElementById('slateForm');
      if (form) {
        form.addEventListener('submit', function() {
          const autoSync = document.getElementById('autoSyncDate');
          if (autoSync && autoSync.checked) {
            syncPhoneDate();
          }
        });
      }

      // Estados de los botones toggle
      const s = document.getElementById('soundInput').value;
      if (s === 'MOS') document.getElementById('btnMos').classList.add('active');
      else document.getElementById('btnSync').classList.add('active');

      const l = document.getElementById('locInput').value;
      if (l === 'EXT') document.getElementById('btnExt').classList.add('active');
      else document.getElementById('btnInt').classList.add('active');

      const lt = document.getElementById('lightInput').value;
      if (lt === 'NIGHT') document.getElementById('btnNight').classList.add('active');
      else document.getElementById('btnDay').classList.add('active');
    });
  </script>
</body>
</html>)HTML";

  return html;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleUpdate() {
  if (server.hasArg("roll")) roll = server.arg("roll");
  if (server.hasArg("scene")) scene = server.arg("scene");
  if (server.hasArg("take")) take = server.arg("take");
  if (server.hasArg("prod")) prod = server.arg("prod");
  if (server.hasArg("director")) director = server.arg("director");
  if (server.hasArg("dop")) dop = server.arg("dop");
  if (server.hasArg("note")) note = server.arg("note");
  if (server.hasArg("date")) date = server.arg("date");
  if (server.hasArg("cam")) cam = server.arg("cam");
  if (server.hasArg("sound")) sound = server.arg("sound");
  if (server.hasArg("loc")) loc = server.arg("loc");
  if (server.hasArg("light")) light = server.arg("light");

  // Siempre forzar a mayúsculas sostenidas (el emoji 🌸 en UTF-8 queda intacto)
  roll.toUpperCase();
  scene.toUpperCase();
  take.toUpperCase();
  prod.toUpperCase();
  director.toUpperCase();
  dop.toUpperCase();
  note.toUpperCase();
  date.toUpperCase();
  cam.toUpperCase();
  sound.toUpperCase();
  loc.toUpperCase();
  light.toUpperCase();

  // Responder de inmediato al cliente HTTP antes de refrescar el ePaper
  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Actualizado. Redirigiendo...");

  updateDisplay();
}

void handleIncTake() {
  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Incrementando TAKE...");
  incrementTake();
}

void handleIncRoll() {
  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Incrementando ROLL...");
  incrementRoll();
}

// Lectura de botones físicos con debounce y protección anti-rebote
void checkButtons() {
  static unsigned long lastActionTime = 0;
  unsigned long now = millis();

  // Cooldown de 1.5 segundos para evitar pulsaciones múltiples durante el refresco del panel e-Paper
  if (now - lastActionTime < 1500) return;

  // Botón Key1: Incrementa TAKE (+1) (GPIO3 o GPIO2)
  if (digitalRead(PIN_BTN_TAKE_1) == LOW || digitalRead(PIN_BTN_TAKE_0) == LOW) {
    lastActionTime = now;
    incrementTake();
    return;
  }

  // Botón Key3: Incrementa ROLL (+1) (GPIO5)
  if (digitalRead(PIN_BTN_ROLL) == LOW) {
    lastActionTime = now;
    incrementRoll();
    return;
  }
}

void setup() {
  Serial.begin(115200);

  // Configurar botones físicos con pull-up interno
  pinMode(PIN_BTN_TAKE_1, INPUT_PULLUP);
  pinMode(PIN_BTN_TAKE_0, INPUT_PULLUP);
  pinMode(PIN_BTN_ROLL, INPUT_PULLUP);

  // Espera para USB CDC
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 2500));

  Serial.println("\n==============================================");
  Serial.println("  E-Slate Cinema Pro - Seeed_GFX2             ");
  Serial.println("==============================================");

  // Inicializar controlador gráfico Seeed_GFX2
  if (!display.begin()) {
    Serial.printf("Error Seeed_GFX2: %s\n", display.lastResult().message);
  } else {
    Serial.println("Seeed_GFX2 iniciado correctamente.");
  }

  // Iniciar Access Point Wi-Fi
  WiFi.softAP(ssid, password);
  localIP = WiFi.softAPIP();

  Serial.printf("Wi-Fi AP: %s (Password: %s)\n", ssid, password);
  Serial.printf("IP: %s\n", localIP.toString().c_str());

  // Iniciar servicio mDNS para responder a http://clap.local
  if (MDNS.begin("clap")) {
    Serial.println("mDNS iniciado: http://clap.local");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error iniciando mDNS.");
  }

  // Rutas del Servidor Web HTTP
  server.on("/", handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/inc-take", HTTP_POST, handleIncTake);
  server.on("/inc-roll", HTTP_POST, handleIncRoll);
  server.begin();
  Serial.println("Servidor Web HTTP activo.");

  // Pantalla de bienvenida
  showIPOnBoot();

  // Primer renderizado de la claqueta
  updateDisplay();
}

void loop() {
  server.handleClient();
  checkButtons();
}

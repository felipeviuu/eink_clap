#include <Seeed_GFX.h>
#include <WebServer.h>
#include <WiFi.h>

// Instancia Seeed_GFX2 para el panel 7.5" en la placa EE04
Seeed_GFX display(Seeed_Product::Seeed_ePaper_7INCH5);

// Configuración del Access Point Wi-Fi
const char *ssid = "NEBRALTA_Clap";
const char *password = "123456789P";

WebServer server(80);

// Campos de la claqueta (Clapperboard)
String roll = "A001";
String scene = "35A";
String take = "1";
String prod = "NEBRALTA";
String director = "JONATHAN LOPEZ";
String dop = "FELIPE SALAS";
String note = "ISO 120";
String date = "11.03.2026";
IPAddress localIP;

// Mostrar pantalla de bienvenida e instrucciones de conexión IP
void showIPOnBoot() {
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setFreeFont(nullptr);

  // Marco exterior
  display.drawRect(20, 20, 760, 440, TFT_BLACK);
  display.drawRect(22, 22, 756, 436, TFT_BLACK);

  // Título
  display.setTextSize(4);
  display.drawCentreString("DIGITAL FILM CLAPPER", 400, 80, 1);

  // Línea divisoria
  display.drawLine(60, 140, 740, 140, TFT_BLACK);

  // Instrucciones WiFi
  display.setTextSize(3);
  display.drawString("1. Conectate a la red WiFi:", 80, 180);
  display.setTextSize(4);
  display.drawString(String("\"") + ssid + "\"", 120, 225);

  display.setTextSize(3);
  display.drawString("2. Abre tu navegador en la IP:", 80, 290);
  display.setTextSize(4);
  display.drawString(String("http://") + localIP.toString(), 120, 335);

  display.setTextSize(2);
  display.drawString("Password WiFi: 123456789", 80, 400);

  // Refrescar pantalla
  display.refresh();
  delay(4000); // Pausa para permitir lectura
}

// Renderizar la claqueta completa en el panel e-paper
void updateDisplay() {
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setFreeFont(nullptr);

  // Líneas de la grilla de la claqueta
  display.drawLine(0, 193, 798, 193, TFT_BLACK);
  display.drawLine(0, 313, 798, 313, TFT_BLACK);
  display.drawLine(400, 314, 400, 478, TFT_BLACK);
  display.drawLine(0, 394, 798, 394, TFT_BLACK);
  display.drawLine(266, 0, 266, 192, TFT_BLACK);
  display.drawLine(548, 0, 548, 192, TFT_BLACK);

  // --- SECCIÓN SUPERIOR: ROLL | SCENE | TAKE ---
  // Etiquetas
  display.setTextSize(3);
  display.drawString("ROLL", 15, 12);
  display.drawString("SCENE", 280, 12);
  display.drawString("TAKE", 565, 12);

  // Valores grandes
  display.setTextSize(8);
  display.drawString(roll, 35, 75);
  display.drawString(scene, 320, 75);
  display.drawString(take, 640, 75);

  // --- SECCIÓN PRODUCCIÓN: PROD ---
  display.setTextSize(3);
  display.drawString("PROD", 15, 205);
  display.setTextSize(7);
  display.drawString(prod, 160, 220);

  // --- SECCIÓN MEDIA / INFERIOR: DIR, DOP, NOTE, DATE ---
  // Etiquetas
  display.setTextSize(2);
  display.drawString("DIR", 15, 325);
  display.drawString("DOP", 415, 325);
  display.drawString("NOTE", 15, 405);
  display.drawString("DATE", 415, 405);

  // Valores
  display.setTextSize(5);
  display.drawString(director, 90, 335);
  display.drawString(dop, 500, 335);
  display.drawString(note, 90, 415);
  display.drawString(date, 500, 415);

  // Ejecutar el refresco en el driver e-paper
  const GfxResult res = display.refresh();
  if (!res) {
    Serial.printf("[Seeed_GFX2 Error]: %s\n", res.message);
  } else {
    Serial.println("Display actualizado con éxito.");
  }
}

// Interfaz Web HTML moderna y responsive
String htmlPage() {
  String html = R"HTML(<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Film Clapperboard Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: #121212;
      color: #eee;
      padding: 15px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .container { max-width: 650px; width: 100%; margin: 0 auto; }
    .header { text-align: center; margin-bottom: 20px; }
    .header h1 { font-size: 1.8rem; letter-spacing: 2px; text-transform: uppercase; color: #fff; }
    .header p { font-size: 0.9rem; color: #8fc31f; margin-top: 4px; }
    .card {
      background: #1e1e1e;
      border: 1px solid #333;
      padding: 22px;
      border-radius: 12px;
      box-shadow: 0 8px 30px rgba(0,0,0,0.5);
    }
    .form-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
      margin-bottom: 18px;
    }
    .form-group { margin-bottom: 10px; }
    label {
      display: block;
      font-weight: 700;
      margin-bottom: 5px;
      color: #aaa;
      font-size: 0.75rem;
      letter-spacing: 1px;
    }
    input {
      width: 100%;
      padding: 10px;
      border-radius: 6px;
      border: 1px solid #444;
      background: #2a2a2a;
      color: #fff;
      font-size: 15px;
      font-weight: 600;
      transition: border-color 0.2s;
    }
    input:focus {
      outline: none;
      border-color: #8fc31f;
      background: #333;
    }
    .full-width { grid-column: 1 / -1; }
    .two-cols { grid-column: span 2; }
    .btn {
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
      transition: background 0.2s, transform 0.1s;
    }
    .btn:hover { background: #9ee024; }
    .btn:active { transform: scale(0.99); }
    .footer-ip {
      text-align: center;
      margin-top: 15px;
      color: #777;
      font-size: 0.8rem;
    }
    @media (max-width: 500px) {
      .form-grid { grid-template-columns: 1fr; }
      .two-cols { grid-column: span 1; }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🎬 E-Slate Digital</h1>
      <p>Control Remoto de Claqueta</p>
    </div>
    <div class="card">
      <form method="POST" action="/update">
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
            <input type="text" id="prod" name="prod" value=")HTML";
  html += prod;
  html += R"HTML(" maxlength="35">
          </div>
          <div class="form-group two-cols">
            <label for="director">DIRECTOR (DIR)</label>
            <input type="text" id="director" name="director" value=")HTML";
  html += director;
  html += R"HTML(" maxlength="30">
          </div>
          <div class="form-group">
            <label for="dop">FOTOGRAFÍA (DOP)</label>
            <input type="text" id="dop" name="dop" value=")HTML";
  html += dop;
  html += R"HTML(" maxlength="30">
          </div>
          <div class="form-group two-cols">
            <label for="note">NOTA</label>
            <input type="text" id="note" name="note" value=")HTML";
  html += note;
  html += R"HTML(" maxlength="30">
          </div>
          <div class="form-group">
            <label for="date">FECHA</label>
            <input type="text" id="date" name="date" value=")HTML";
  html += date;
  html += R"HTML(" maxlength="15">
          </div>
        </div>
        <button class="btn" type="submit">Actualizar Claqueta</button>
      </form>
    </div>
    <div class="footer-ip">
      Conectado al punto de acceso | IP: <strong>)HTML";
  html += localIP.toString();
  html += R"HTML(</strong>
    </div>
  </div>
</body>
</html>)HTML";

  return html;
}

void handleRoot() { server.send(200, "text/html", htmlPage()); }

void handleUpdate() {
  if (server.hasArg("roll"))
    roll = server.arg("roll");
  if (server.hasArg("scene"))
    scene = server.arg("scene");
  if (server.hasArg("take"))
    take = server.arg("take");
  if (server.hasArg("prod"))
    prod = server.arg("prod");
  if (server.hasArg("director"))
    director = server.arg("director");
  if (server.hasArg("dop"))
    dop = server.arg("dop");
  if (server.hasArg("note"))
    note = server.arg("note");
  if (server.hasArg("date"))
    date = server.arg("date");

  // Redirigir de inmediato al navegador para respuesta ágil
  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Actualizado. Redirigiendo...");

  // Actualizar la pantalla e-Paper
  updateDisplay();
}

void setup() {
  Serial.begin(115200);

  // Esperar a que el puerto USB CDC nativo responda
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 3000))
    ;

  Serial.println("\n--- Iniciando E-Slate Claqueta (Seeed_GFX2) ---");

  // Inicializar el controlador Seeed_GFX2 para la pantalla ePaper
  if (!display.begin()) {
    Serial.printf("Error al iniciar Seeed_GFX2: %s\n",
                  display.lastResult().message);
  } else {
    Serial.println("Seeed_GFX2 inicializado correctamente.");
  }

  // Iniciar Punto de Acceso WiFi (SoftAP)
  WiFi.softAP(ssid, password);
  localIP = WiFi.softAPIP();

  Serial.printf("WiFi AP iniciado: %s (Pass: %s)\n", ssid, password);
  Serial.printf("Direccion IP: %s\n", localIP.toString().c_str());

  // Rutas del Servidor Web
  server.on("/", handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.begin();
  Serial.println("Servidor Web HTTP activo.");

  // Mostrar mensaje inicial en pantalla
  showIPOnBoot();

  // Dibujar estado actual de la claqueta
  updateDisplay();
}

void loop() { server.handleClient(); }

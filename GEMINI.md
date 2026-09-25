# Project Context & Agent Memory: E-Slate Claqueta Digital (XIAO ESP32-S3 Plus + Seeed_GFX2)

## 🎯 Directiva Actual del Proyecto
> **ESTADO**: Se pospone de forma indefinida la integración con el backend TRMNL Terminus (Docker, PostgreSQL, Cloudflare Tunnel).
> **OBJETIVO ACTIVO**: Desarrollo y operación autónoma del dispositivo **E-Slate (Digital Film Clapperboard)** utilizando:
> 1. Placa **Seeed Studio XIAO ESP32-S3 Plus** (16MB Flash, 8MB PSRAM OPI).
> 2. Placa base de conexión **XIAO ePaper Display Board (EE04)**.
> 3. Pantalla **7.5" Monochrome ePaper (800x480, UC8179)**.
> 4. Librería gráfica moderna **Seeed_GFX2**.
> 5. Servidor Web integrado + Punto de acceso Wi-Fi local autónomo (**`NEBRALTA_Clap`** / pass **`123456789P`**).
> 6. Entorno de desarrollo primario: **PlatformIO en Antigravity IDE**.

---

## 🛠️ Especificaciones de Hardware

| Componente | Especificación Técnica |
| :--- | :--- |
| **Microcontrolador** | **Seeed Studio XIAO ESP32-S3 Plus** (Xtensa LX7 Dual-Core @ 240MHz) |
| **Memoria Flash** | **16 MB** Quad-SPI (QIO) |
| **Memoria PSRAM** | **8 MB** Octal-SPI (OPI PSRAM) - Modo `qio_opi` |
| **Carrier Board** | **XIAO ePaper Display Board - EE04** (conector FPC 24 pines y 50 pines) |
| **Display e-Paper** | **7.5 pulgadas Monocromático (800 x 480 píxeles, controlador UC8179)** |
| **Buffer de Render** | 1 bpp empaquetado (optimizado para bajo consumo de RAM/PSRAM) |
| **Interfaz USB** | USB Nativo OTG / Serial JTAG (GPIO19 = D-, GPIO20 = D+) |
| **Pulsadores EE04** | KEY0 (GPIO2), KEY1 (GPIO3), KEY2 (GPIO5) - Activos en bajo |
| **Batería / ADC** | Voltaje en A0 (GPIO1), habilitador ADC en A5 (GPIO6) |

---

## 💻 Entorno de Desarrollo: PlatformIO en Antigravity IDE

El proyecto está optimizado para **PlatformIO** dentro de Antigravity IDE (VS Code) para compilación paralela de alta velocidad.

### 1. Archivo `platformio.ini`
```ini
[platformio]
src_dir = eink_clap

[env:seeed_xiao_esp32s3_plus]
platform = espressif32
board = seeed_xiao_esp32s3
framework = arduino

; Configuración de memoria para el modelo PLUS (16MB Flash / 8MB OPI PSRAM)
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv
board_build.arduino.memory_type = qio_opi

; Flags obligatorios para USB CDC nativo y PSRAM
build_flags = 
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1
    -D BOARD_HAS_PSRAM

; Descarga automática de la librería Seeed_GFX2 desde GitHub
lib_deps = 
    https://github.com/Seeed-Studio/Seeed_GFX2.git

monitor_speed = 115200
```

### 2. Estructura de Directorios del Proyecto
```text
eink_clap/
├── .vscode/               # Configuración de extensiones de Antigravity IDE
├── eink_clap/
│   └── eink_clap.ino      # Firmware principal de la claqueta (Seeed_GFX2 + SoftAP + WebServer)
├── src -> eink_clap       # Enlace simbólico estándar para PlatformIO
├── include/               # Cabeceras adicionales C/C++
├── lib/                   # Librerías locales privadas
├── platformio.ini         # Configuración del entorno de compilación
├── GEMINI.md              # Memoria, directivas y reglas del agente (este archivo)
└── README.md              # Documentación general del repositorio
```

### 3. Comandos Útiles de PlatformIO
- **Compilar**: `pio run` (o icono ✔️ en la barra inferior)
- **Flashear**: `pio run -t upload` (o icono ➡️ en la barra inferior)
- **Monitor Serie**: `pio device monitor` (o icono 🔌 a 115200 baudios)
- **Abrir en PlatformIO Home**: Si no aparece en la lista visual, pulsar casita 🏠 → *Open Project* → seleccionar carpeta `/Users/felipesalas/Development/eink_clap`.

---

## 🎨 Reglas Obligatorias de `Seeed_GFX2`

1. **PROHIBIDO usar `TFT_eSPI`**:
   - `Seeed_GFX2` entra en conflicto directo de símbolos con `TFT_eSPI`.
   - No usar `#define EPAPER_ENABLE` ni instanciar `EPaper epaper;` (eso pertenecía al fork obsoleto).
2. **Inclusión e Instanciación Oficial**:
   ```cpp
   #include <Seeed_GFX.h>
   Seeed_GFX display(Seeed_Product::Seeed_ePaper_7INCH5);
   ```
3. **Inicialización y Refresco**:
   - Iniciar siempre validando hardware:
     ```cpp
     if (!display.begin()) {
       Serial.printf("Error: %s\n", display.lastResult().message);
     }
     ```
   - Para actualizar el panel físico: `const GfxResult res = display.refresh();`
4. **Respuesta Web Ágil**:
   - En peticiones POST de actualización (`/update`), el servidor HTTP debe enviar de inmediato `303 See Other` al navegador **antes** o en paralelo al `display.refresh()`, evitando que el navegador móvil quede en estado de espera mientras el e-paper refresca físicamente.

---

## 🔌 Protocolo de Conexión USB y Bootloader en macOS

### Detección de Placa
- Si macOS no lista el puerto o la placa se reinicia en bucle:
  1. Conectar cable USB-C con soporte de datos al Mac.
  2. Mantener presionado botón **BOOT** (`B`).
  3. Presionar y soltar botón **RESET** (`R`).
  4. Soltar botón **BOOT** (`B`).
  5. macOS expondrá el puerto de inmediato como `/dev/cu.usbmodem*`.
- Verificar en Terminal:
  ```bash
  ls -l /dev/cu.usb*
  esptool.py flash_id
  ```

---

## 🎬 Funcionalidad del Firmware (`eink_clap.ino`)

1. **Punto de Acceso Wi-Fi Local**:
   - SSID: `NEBRALTA_Clap`
   - Password: `123456789P`
   - IP estática del dispositivo: `192.168.4.1`
   - Dominio mDNS: `http://clap.local`
2. **Pantalla de Bienvenida (`showIPOnBoot`)**:
   - Al arrancar, dibuja un marco con el nombre de la red y la URL a visitar.
3. **Interfaz Web Responsiva y Valores Predeterminados**:
   - Tema oscuro cinematográfico accesible desde smartphone o tablet.
   - **PROD**: `NEBRALTA 🌸` (soporta emoji de flor de cerezo monocromático de 32x32)
   - **DIR**: `JONATHAN LOPEZ`
   - **DOP**: `FELIPE SALAS`
   - **NOTE**: `ISO 120`
   - **DATE**: Sincronización automática desde el reloj del smartphone/tablet (con botón `📱 HOY` y opción manual)
   - **ROLL**: `A001` | **SCENE**: `35A` | **TAKE**: `1`
   - **CAM**: `A` | **SOUND**: `SYNC` / `MOS` | **LOC**: `INT` / `EXT` | **LIGHT**: `DAY` / `NIGHT`.
   - **Sin Timecode**: Eliminado por completo para un diseño limpio, espacioso y legible a gran distancia.
4. **Grilla de Claqueta en Panel 800x480**:
   - Dibuja divisiones precisas de claqueta tradicional.
   - Ajuste inteligente automático (`drawTextAuto`):
     - Prioriza **UNA SOLA LÍNEA** más grande (probando tamaños decrecientes).
     - Evita saltos de línea innecesarios en títulos como `"FLOR Y CONVENTO"`, renderizándolo en tamaño 7 (56px) o tamaño 6 (48px con flor).
     - Centrado vertical dinámico dentro de cada franja.
   - **Mayúsculas sostenidas**: Todos los textos se transforman y renderizan obligatoriamente en MAYÚSCULAS (ALL CAPS).
   - Badges visuales de alto contraste (`drawBadge`): fondo negro y texto blanco para el estado activo (`SYNC`, `INT`, `DAY`).
   - Botones físicos EE04: Key1 (GPIO3/2) suma +1 TAKE, Key3 (GPIO5) suma +1 ROLL con cooldown anti-rebote de 1.5s.

---

## 📋 Estado y Próximos Pasos

- [x] Contexto y directivas configuradas en `GEMINI.md`.
- [x] Configuración de `platformio.ini` para XIAO ESP32-S3 Plus (16MB/8MB OPI PSRAM).
- [x] Código de la claqueta migrado 100% a `Seeed_GFX2` en `eink_clap.ino`.
- [x] Soporte mDNS (`http://clap.local`), ajuste inteligente a una/dos líneas y mayúsculas sostenidas forzadas.
- [x] Soporte para el emoji `🌸` mediante bitmap monocromático embebido en PROGMEM.
- [x] Sincronización automática de fecha desde el navegador del dispositivo móvil.
- [x] Integración de botones físicos KEY1 (+1 TAKE) y KEY3 (+1 ROLL) con cooldown anti-rebote.
- [x] Edición Cinema Pro con CAM, SYNC/MOS, INT/EXT y DAY/NIGHT (Timecode retirado).
- [x] Optimización de `drawTextAuto` para priorizar una sola línea grande ("FLOR Y CONVENTO").
- [ ] Flasheo físico en la placa desde Arduino IDE / PlatformIO.
- [ ] Validación visual final en el panel e-paper de 7.5".

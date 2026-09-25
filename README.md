# E-Slate Claqueta Digital (Film Clapperboard) 🎬📟

Claqueta de cine digital y portátil construida sobre tinta electrónica (**e-ink / e-Paper**), impulsada por el microcontrolador **Seeed Studio XIAO ESP32-S3 Plus** y la placa controladora **XIAO ePaper Display Board (EE04)** con la librería gráfica **[Seeed_GFX2](https://github.com/Seeed-Studio/Seeed_GFX2)**.

El dispositivo genera su propia red Wi-Fi y aloja un servidor web interno para que cualquier miembro del equipo de rodaje pueda actualizar remotamente los datos de la claqueta (ROLL, SCENE, TAKE, PROD, DIR, DOP, etc.) desde su teléfono móvil o tablet en tiempo real.

---

## 🛠️ Especificaciones de Hardware

| Componente | Especificación Técnica |
| :--- | :--- |
| **Microcontrolador** | **Seeed Studio XIAO ESP32-S3 Plus** (Xtensa LX7 Dual-Core @ 240MHz) |
| **Memoria** | 16 MB Flash (QIO) + 8 MB PSRAM OPI (`qio_opi`) |
| **Carrier Board** | **XIAO ePaper Display Board - EE04** |
| **Pantalla** | **7.5" Monochrome ePaper (800 x 480 px, UC8179)** |
| **Librería Gráfica** | **Seeed_GFX2** (buffers nativos de 1 bpp) |
| **Conectividad** | Wi-Fi 802.11 b/g/n (Modo SoftAP autónomo) + WebServer HTTP |
| **Alimentación** | Batería LiPo 3.7V con conector JST 2.0mm o USB-C |

---

## 📁 Estructura del Proyecto

```text
eink_clap/
├── .vscode/               # Configuración para Antigravity IDE
├── eink_clap/
│   └── eink_clap.ino      # Firmware de la claqueta (Seeed_GFX2 + SoftAP + WebServer)
├── src -> eink_clap       # Enlace simbólico estándar para PlatformIO
├── include/               # Archivos de inclusión C/C++
├── lib/                   # Librerías locales
├── platformio.ini         # Configuración y dependencias de compilación
├── GEMINI.md              # Memoria, directivas y reglas para el agente de IA
└── README.md              # Documentación general del repositorio
```

---

## 💻 Desarrollo y Compilación con PlatformIO

Este proyecto está configurado para compilarse a máxima velocidad en **PlatformIO** dentro de **Antigravity IDE**:

1. **Abrir el proyecto**:
   - Si no aparece listado en PlatformIO Home, haz clic en la casita 🏠 → **Open Project** → selecciona la carpeta del repositorio.
   - O recarga la ventana con `Cmd + Shift + P` → `Developer: Reload Window`.
2. **Compilar**:
   - Presiona el icono del **check (✔)** en la barra inferior o ejecuta en la terminal:
     ```bash
     pio run
     ```
3. **Subir a la placa**:
   - Conecta la placa por USB y pulsa la **flecha (➜)** en la barra inferior o ejecuta:
     ```bash
     pio run -t upload
     ```
4. **Monitor Serie**:
   - Pulsa el icono del enchufe (**🔌**) a 115200 baudios o ejecuta:
     ```bash
     pio device monitor
     ```

---

## 📱 Uso en Rodaje

1. Al encender la claqueta, la pantalla mostrará las instrucciones de conexión durante unos segundos:
   - **Red Wi-Fi**: `ClapBoard_AP`
   - **Contraseña**: `123456789`
   - **Dirección Web**: `http://192.168.4.1`
2. Conéctate a la red Wi-Fi desde tu móvil o tablet.
3. Abre el navegador web en `http://192.168.4.1`.
4. Edita los campos deseados y pulsa **"Actualizar Claqueta"**. La pantalla e-Paper se actualizará automáticamente con la nueva información.

---

## 🔌 Solución de Problemas: Detección USB (macOS)

Si el puerto serie de la placa desaparece en tu Mac:
1. Mantén presionado el botón **`BOOT`** (marcado como `B`).
2. Presiona y suelta una vez el botón **`RESET`** (marcado como `R`).
3. Suelta el botón **`BOOT`**.
4. Verifica en la terminal con:
   ```bash
   ls -l /dev/cu.usb*
   ```

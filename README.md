# eink_clap 📟✨

Proyecto de desarrollo para pantalla de tinta electrónica (**e-ink / e-Paper**) basada en el microcontrolador **Seeed Studio XIAO ESP32-S3**, utilizando la librería gráfica moderna **[Seeed_GFX2](https://github.com/Seeed-Studio/Seeed_GFX2)**.

El objetivo a mediano plazo contempla la sincronización con un servidor **TRMNL Terminus** para desplegar dashboards y widgets informativos de bajo consumo.

---

## 🛠️ Especificaciones de Hardware

| Componente | Detalle |
| :--- | :--- |
| **Microcontrolador** | Seeed Studio XIAO ESP32-S3 |
| **Arquitectura** | Xtensa Dual-Core 32-bit LX7 @ hasta 240 MHz |
| **Memoria** | 8 MB Flash (Octal SPI) + 8 MB PSRAM (OPI) |
| **Interfaz USB** | USB Nativo OTG / Serial JTAG en chip (GPIO19: D-, GPIO20: D+) |
| **Pantalla** | Pantalla e-ink / e-Paper compatible con Seeed_GFX2 |
| **Librería Gráfica** | [Seeed_GFX2](https://github.com/Seeed-Studio/Seeed_GFX2) |

---

## 📁 Estructura del Repositorio

```text
eink_clap/
├── GEMINI.md               # Contexto, directivas del proyecto y reglas de desarrollo
├── README.md               # Documentación general del proyecto (este archivo)
└── eink_clap/
    └── eink_clap.ino       # Sketch base de diagnóstico de hardware y verificación USB CDC
```

---

## 🔌 Protocolo de Conexión USB y Bootloader (macOS)

El **ESP32-S3** utiliza un puerto USB nativo directo. Si el firmware cargado previamente desactiva el puerto CDC, o el microcontrolador entra en un bucle de reinicio (*kernel panic*), el puerto serie desaparecerá por completo del sistema operativo.

### Modo Bootloader Forzado (BROM)
Para forzar al chip a exponer el puerto serie en macOS:

1. Conecta el XIAO ESP32-S3 a la Mac mediante cable USB-C de datos.
2. Mantén presionado el botón **BOOT** (etiquetado como **B** en la placa).
3. Presiona y suelta el botón **RESET** (etiquetado como **R**).
4. Suelta el botón **BOOT**.
5. macOS detectará inmediatamente el dispositivo en modo ROM.

### Verificación del Puerto Serie
Abre la terminal en macOS y ejecuta:

```bash
# Listar puertos USB serie detectados
ls -l /dev/cu.usb*
```
> El puerto usualmente aparecerá con un nombre similar a `/dev/cu.usbmodem1101` o `/dev/cu.usbmodem2101`.

Para validar la comunicación con `esptool`:
```bash
esptool.py flash_id
```

### Checklist Físico y de Sistema
- [ ] **Cable USB**: Asegúrate de usar un cable de transferencia de datos de 4 hilos (no un cable que solo cargue batería).
- [ ] **Permisos de macOS**: Acepta el diálogo emergente *"¿Permitir que el accesorio se conecte a esta Mac?"* al enchufarlo.
- [ ] **Hubs y Adaptadores**: Si usas un hub USB-C o adaptador multipuerto, prueba una conexión directa o un puerto con compatibilidad USB 2.0 en caso de fallos de enumeración.

---

## ⚙️ Configuración del Entorno (Arduino IDE / CLI)

### URL del Gestor de Tarjetas
Añade la siguiente URL en **Arduino IDE > Preferences > Additional Boards Manager URLs**:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### Parámetros Críticos de la Placa (Menú *Herramientas / Tools*)

| Parámetro | Valor Requerido | Por qué es crítico |
| :--- | :--- | :--- |
| **Board** | `XIAO_ESP32S3` | Configuración oficial de pines y frecuencias para XIAO |
| **USB CDC On Boot** | **Enabled** | **IMPRESCINDIBLE**: Si está deshabilitado, `Serial` no funcionará y el puerto USB no responderá |
| **Flash Size** | `8MB (64Mb)` | Capacidad física de memoria Flash del módulo |
| **PSRAM** | `OPI PSRAM` | Habilita los 8 MB de memoria RAM externa de alta velocidad |
| **Upload Mode** | `UART0 / Hardware CDC` | Flasheo directo sobre el controlador nativo de la ROM |
| **USB DFU On Boot** | `Disabled` | Mantener apagado |

---

## 🎨 Librería Gráfica Seeed_GFX2

El proyecto emplea la librería [Seeed_GFX2](https://github.com/Seeed-Studio/Seeed_GFX2) debido a su arquitectura modular orientada a pantallas modernas:

$$\text{Board} \longrightarrow \text{Bus} \longrightarrow \text{Driver} \longrightarrow \text{Panel}$$

### ⚠️ Regla Estricta: Conflicto con `TFT_eSPI`
`Seeed_GFX2` entra en conflicto directo de nombres y símbolos si `TFT_eSPI` se encuentra instalada en el directorio de librerías de Arduino (`~/Documents/Arduino/libraries/TFT_eSPI`).
* **Acción requerida**: Desinstala o renombra la carpeta de `TFT_eSPI` antes de compilar cualquier sketch con `Seeed_GFX2`.

---

## 🚀 Primeros Pasos: Flasheo del Sketch Base

El sketch [eink_clap.ino](file:///Users/felipesalas/Development/eink_clap/eink_clap/eink_clap.ino) valida el funcionamiento del hardware, el LED integrado y la salida de telemetría por USB CDC.

1. Abre el archivo `eink_clap/eink_clap.ino` en Arduino IDE.
2. Selecciona la placa **XIAO_ESP32S3** y el puerto `/dev/cu.usbmodem*`.
3. Verifica que **USB CDC On Boot** esté en **Enabled**.
4. Haz clic en **Upload / Subir**.
5. Abre el **Serial Monitor** configurado a **115200 baudios**.

### Salida esperada en Monitor Serie:
```text
====================================
  ESP32-S3 ONLINE - Puerto CDC OK   
====================================
Chip Model: ESP32-S3 (Rev 0)
Cores: 2, CPU Freq: 240 MHz
Flash Size: 8 MB
PSRAM Size: 8 MB
Listo para pruebas con Seeed_GFX2.

[Heartbeat #0] ESP32-S3 ejecutando normalmente.
[Heartbeat #1] ESP32-S3 ejecutando normalmente.
```

---

## 🗺️ Hoja de Ruta (Roadmap)

- [x] **Fase 1: Reconocimiento de Hardware y CDC**: Creación del entorno base, verificación de bootloader y telemetría por USB nativo.
- [ ] **Fase 2: Integración de Seeed_GFX2**: Instalación de la librería y configuración del bus SPI/I80 según la pantalla seleccionada.
- [ ] **Fase 3: Pruebas de Renderizado e-Paper**: Inicialización del panel e-ink, refresco parcial/completo, renderizado de primitivas gráficas y tipografías.
- [ ] **Fase 4: Conectividad y TRMNL Terminus**: Implementación de cliente HTTP/WiFi para sincronización de imágenes y widgets generados por backend TRMNL Terminus.

---

## 📄 Licencia

Este proyecto está bajo licencia [MIT](LICENSE) (o la que se designe para el repositorio).

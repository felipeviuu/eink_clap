# Project Context & Agent Rules: ESP32-S3 + Seeed_GFX2

## 🎯 Directiva Actual del Proyecto
> **ESTADO PRIORITARIO**: Se pospone temporalmente la integración con el backend TRMNL Terminus (Docker, PostgreSQL, Cloudflare Tunnel). 
> **OBJETIVO INMEDIATO**: Reconocimiento de hardware, puesta en modo Bootloader, configuración de entorno y flasheo exitoso del **ESP32-S3** (Seeed Studio XIAO ESP32S3) con la librería gráfica **Seeed_GFX2**.

---

## 🛠️ Especificaciones de Hardware

- **Microcontrolador**: **Seeed Studio XIAO ESP32-S3 Plus** (Xtensa Dual-Core LX7, hasta 240MHz, **16MB Flash**, **8MB PSRAM OPI**, 20 GPIOs).
- **Identificador PlatformIO**: `seeed_xiao_esp32s3` con configuración de 16MB Flash y `qio_opi`.
- **Interfaz USB**: USB Nativo OTG / Serial JTAG integrado en el chip (GPIO19 = D-, GPIO20 = D+).
- **Pantalla**: Pantalla e-ink / e-Paper o LCD compatible con Seeed_GFX2.
- **Librería Gráfica**: [Seeed_GFX2](https://github.com/Seeed-Studio/Seeed_GFX2).

---

## 🔌 Protocolo de Conexión USB y Bootloader en macOS

### 1. El Problema Común del ESP32-S3
El ESP32-S3 utiliza USB nativo directo. Si el chip tiene cargado un firmware que desactiva el puerto CDC, o entra en un loop de pánico (`kernel panic`), el puerto USB desaparece por completo del sistema operativo.

### 2. Procedimiento para Forzar Modo Bootloader (BROM)
Para que el ESP32-S3 sea detectado forzosamente por macOS:
1. Conectar el ESP32-S3 por USB al Mac.
2. Mantener presionado el botón **BOOT** (o botón **B**).
3. Presionar y soltar el botón **RESET** (o botón **R**) mientras se mantiene presionado BOOT.
4. Soltar el botón **BOOT**.
5. *Resultado esperado*: El chip entra en la ROM de arranque interna y macOS expone el puerto serie inmediatamente.

### 3. Verificación en macOS
Ejecutar en la terminal del Mac:
```bash
# Listar puertos serie USB detectados:
ls -l /dev/cu.usb*

# Verificar comunicación directa con el chip usando esptool:
esptool.py chip_id
# o
esptool.py flash_id
```
*Nombre habitual del puerto en macOS*: `/dev/cu.usbmodem*` (ej. `/dev/cu.usbmodem1101` o `/dev/cu.usbmodem2101`).

### 4. Checklist Físico y de Sistema
- [ ] **Cable USB**: Asegurarse de usar un cable de transferencia de datos de 4 hilos (no un cable solo de carga).
- [ ] **Permisos de macOS**: Aceptar el diálogo del sistema *"¿Permitir que el accesorio se conecte a esta Mac?"* al enchufarlo.
- [ ] **Hubs USB-C**: Si se usa adaptador, probar conexión directa o puerto con soporte USB 2.0.

---

## ⚙️ Configuración del Entorno de Desarrollo (Arduino IDE / CLI)

### Parámetros Críticos de la Placa (Tools / Herramientas):
| Parámetro | Valor Requerido | Explicación |
| :--- | :--- | :--- |
| **Board** | `XIAO_ESP32S3` | Paquete oficial de Espressif / Seeed |
| **USB CDC On Boot** | **Enabled** | **CRÍTICO**: Sin esto, `Serial.print()` no funciona por USB nativo y el puerto se desconecta al arrancar |
| **Flash Size** | `8MB (64Mb)` | Capacidad estándar de XIAO ESP32S3 |
| **PSRAM** | `OPI PSRAM` | Para disponer de los 8MB de memoria PSRAM |
| **Upload Mode** | `UART0 / Hardware CDC` | Flasheo directo sobre el controlador nativo |
| **USB DFU On Boot** | `Disabled` | Mantener apagado |

### URL del Gestor de Tarjetas (Arduino Preferences):
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

---

## 🎨 Reglas de Seeed_GFX2

1. **Eliminar TFT_eSPI**:
   - `Seeed_GFX2` entra en conflicto directo de nombres y símbolos si `TFT_eSPI` está instalada en `~/Documents/Arduino/libraries/`.
   - **Regla estricta**: Desinstalar o renombrar la carpeta de `TFT_eSPI` antes de compilar.
2. **Instalación de Seeed_GFX2**:
   - Descargar el archivo ZIP desde [Seeed-Studio/Seeed_GFX2](https://github.com/Seeed-Studio/Seeed_GFX2) o clonar el repositorio dentro de `libraries/`.
3. **Arquitectura Modular**:
   - Utilizar el esquema: `Board` → `Bus` → `Driver` → `Panel`.
   - No requiere editar archivos globales tipo `User_Setup.h`.

---

## 📋 Hoja de Ruta Inmediata

- [x] **Paso 0**: Configurar directivas del repositorio (`GEMINI.md`).
- [ ] **Paso 1**: Obtener detección del puerto `/dev/cu.usbmodem*` en macOS mediante combinación Bootloader.
- [ ] **Paso 2**: Flashear sketch de prueba básico (`blink_cdc.ino`) para validar que la cadena de herramientas y el puerto CDC funcionan correctamente.
- [ ] **Paso 3**: Instalar y validar la librería `Seeed_GFX2`.
- [ ] **Paso 4**: Configurar el bus y driver específicos de la pantalla conectada y renderizar gráficos locales de prueba.

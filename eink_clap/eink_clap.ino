/*
 * ESP32-S3 Base Test Sketch (USB CDC & Serial Verification)
 * Board: Seeed Studio XIAO ESP32S3
 * Settings in Arduino IDE:
 *   - Board: "XIAO_ESP32S3" (o "ESP32S3 Dev Module")
 *   - USB CDC On Boot: "Enabled" (IMPRESCINDIBLE)
 *   - Flash Size: "8MB (64Mb)"
 *   - PSRAM: "OPI PSRAM"
 *   - Upload Mode: "UART0 / Hardware CDC"
 */

#ifndef LED_BUILTIN
#define LED_BUILTIN 21 // Pin del LED integrado en XIAO ESP32S3
#endif

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // En XIAO ESP32S3 el LED suele ser activo bajo o alto

  Serial.begin(115200);

  // Esperar hasta 4 segundos a que el puerto USB CDC se enumere en la PC
  unsigned long start = millis();
  while (!Serial && (millis() - start < 4000)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("\n====================================");
  Serial.println("  ESP32-S3 ONLINE - Puerto CDC OK   ");
  Serial.println("====================================");
  Serial.printf("Chip Model: %s (Rev %d)\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("Cores: %d, CPU Freq: %d MHz\n", ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.println("Listo para pruebas con Seeed_GFX2.\n");
}

int counter = 0;

void loop() {
  // Parpadeo de latido (Heartbeat)
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  Serial.printf("[Heartbeat #%d] ESP32-S3 ejecutando normalmente.\n", counter++);
}

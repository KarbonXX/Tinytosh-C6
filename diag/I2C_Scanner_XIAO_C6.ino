// I2C Scanner for XIAO ESP32-C6
// Run this to see if your OLED is responding on the I2C bus.
// Expected: device found at address 0x3C (SSD1306 OLED)

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("I2C Scanner for XIAO ESP32-C6");
  Serial.println("=============================");

  // Try the firmware's default pins (D4/D5)
  Serial.println("Scanning on D4 (SDA) / D5 (SCL) — firmware defaults...");
  Wire.begin(22, 23);  // SDA=D4=GPIO22, SCL=D5=GPIO23
  scanBus();
}

void scanBus() {
  int found = 0;
  for (byte addr = 0x03; addr <= 0x7F; addr++) {
    Wire.beginTransmission(addr);
    byte err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("  Found device at 0x");
      if (addr < 0x10) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.println();
      found++;
    }
  }
  Serial.print("Scan complete. ");
  if (found == 0) {
    Serial.println("No I2C devices found.");
    Serial.println();
    Serial.println("Troubleshooting:");
    Serial.println("  - Check SDA/SCL wires are firmly seated");
    Serial.println("  - Check OLED VCC has power (3.3V or 5V)");
    Serial.println("  - Check OLED GND is connected to XIAO GND");
    Serial.println("  - Try swapping SDA and SCL wires");
    Serial.println("  - Some OLEDs need 5V on VCC to power the backlight");
  } else if (found == 1) {
    Serial.println("One device found. (Expected: 0x3C for SSD1306 OLED)");
    Serial.println("If the address is 0x3C, your wiring is correct.");
  } else {
    Serial.println("Multiple devices found.");
  }
}

void loop() {
  delay(1000);
}

// BoardConfig.h — Board-specific pin defaults and GPIO range
// Included by both TinytoshESP32.ino and WebServerService.cpp (and other
// translation units that need to know the active GPIO range).
#pragma once

// ── Build-time board target ─────────────────────────────────────────────────
// Override via: -DTINYTOSH_BOARD=XIAO_ESP32_C6
#ifndef TINYTOSH_BOARD
  #define TINYTOSH_BOARD ESP32_C3
#endif

// Per-board pin defaults
namespace BoardPins {

#if TINYTOSH_BOARD == XIAO_ESP32_C6
  // Seeed XIAO ESP32-C6 — user-requested defaults
  //   D10 (GPIO18) — I2C SCL
  //   D9  (GPIO20) — I2C SDA
  //   D3  (GPIO21) — touch (not strapping, not flash, not UART)
  constexpr int DEFAULT_TOUCH_PIN = 21;
  constexpr int DEFAULT_SDA_PIN   = 20;
  constexpr int DEFAULT_SCL_PIN   = 18;

  // C3 defaults kept around so first-boot detection can spot a fresh flash
  // whose NVS still contains C3 pin values from a previous build.
  constexpr int C3_TOUCH_PIN = 10;
  constexpr int C3_SDA_PIN   = 8;
  constexpr int C3_SCL_PIN   = 9;

  constexpr int MIN_GPIO_PIN = 0;
  constexpr int MAX_GPIO_PIN = 30;

  // Pins the XIAO ESP32-C6 should NOT auto-pick for I2C/touch:
  //   GPIO 3, 14 → RF switch / antenna select (firmware-managed)
  //   GPIO 4, 5, 6, 7 → JTAG pads on the back of the XIAO
  //   GPIO 8, 9   → strapping pins on ESP32-C6
  //   GPIO 15     → strapping pin (JTAG MTDO)
  //   GPIO 16, 17 → UART0 (used by USB-CDC serial monitor)
  //   GPIO 18, 19 → connected to internal flash (FSPIQ, FSPID)
  constexpr int BLOCKED_PINS[] = {3, 4, 5, 6, 7, 8, 9, 15, 16, 17};
  constexpr int BLOCKED_COUNT = sizeof(BLOCKED_PINS) / sizeof(BLOCKED_PINS[0]);

  constexpr const char* BOARD_NAME = "XIAO ESP32-C6";
  constexpr const char* BOARD_ID   = "xiao_esp32_c6";

  // XIAO C6 D0..D10 silkscreen labels (indexed by GPIO number, "" if unused)
  // Per Seeed XIAO ESP32-C6 schematic + pins_arduino.h:
  //   GPIO  0 → D0,   GPIO  1 → D1,   GPIO  2 → D2,
  //   GPIO 21 → D3,   GPIO 22 → D4,  GPIO 23 → D5,
  //   GPIO 16 → D6,   GPIO 17 → D7,  GPIO 19 → D8 (SCK, flash),
  //   GPIO 20 → D9,   GPIO 18 → D10 (MOSI, flash)
  constexpr const char* SILKSCREEN[] = {
      //  0    1    2    3    4    5    6    7
      "D0", "D1", "D2", "",   "",   "",   "",   "",
      //  8    9   10   11   12   13   14   15
      "",   "",  "",   "",   "",   "",   "",   "",
      // 16   17   18   19   20   21   22   23
      "D6", "D7", "D10","D8", "D9", "D3", "D4", "D5",
      // 24..31
      "",   "",   "",   "",   "",   "",   "",   ""
  };

#else  // ESP32_C3 (default)

  constexpr int DEFAULT_TOUCH_PIN = 10;
  constexpr int DEFAULT_SDA_PIN   = 8;
  constexpr int DEFAULT_SCL_PIN   = 9;

  constexpr int C3_TOUCH_PIN = 10;
  constexpr int C3_SDA_PIN   = 8;
  constexpr int C3_SCL_PIN   = 9;

  constexpr int MIN_GPIO_PIN = 0;
  constexpr int MAX_GPIO_PIN = 21;

  constexpr int BLOCKED_PINS[] = {};
  constexpr int BLOCKED_COUNT = 0;

  constexpr const char* BOARD_NAME = "ESP32-C3";
  constexpr const char* BOARD_ID   = "esp32_c3";

  constexpr const char* SILKSCREEN[] = {
      "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
      "D8", "D9", "D10"
  };
#endif

}  // namespace BoardPins

inline bool isPinBlocked(int pin) {
  for (int i = 0; i < BoardPins::BLOCKED_COUNT; i++) {
    if (BoardPins::BLOCKED_PINS[i] == pin) return true;
  }
  return false;
}

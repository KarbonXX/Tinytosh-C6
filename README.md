# Tinytosh on XIAO ESP32-C6 — Build & Flash Guide

This fork/branch adds native support for the **Seeed XIAO ESP32-C6** to the
Tinytosh firmware. The original project targets the ESP32-C3 SuperMini; this
guide covers the changes needed for the C6 variant.

> **You are looking at a C6 variant.** For the original C3 guide, see
> <https://vladimirgitsarev.github.io/Tinytosh/>.

---

## What's different vs the C3 build

| Concern | C3 (original) | XIAO ESP32-C6 |
|---|---|---|
| **Board** | ESP32-C3 SuperMini | Seeed XIAO ESP32-C6 |
| **Default I²C** | SDA = GPIO 8, SCL = GPIO 9 | SDA = D4 (GPIO 22), SCL = D5 (GPIO 23) |
| **Default touch** | GPIO 10 | D3 (GPIO 21) |
| **GPIO range exposed in web panel** | 0..21 | 0..30, with ⚠️ markers on strapping/flash/USB pins |
| **USB serial** | Hardware UART over USB-CDC | **USB CDC On Boot must be Enabled** |
| **Arduino core** | esp32 v2.x or v3.x | **esp32 v3.0.0+** (C6 support was incomplete before) |
| **Web flasher manifest** | `manifest.json` (chipFamily: ESP32-C3) | `manifest_c6.json` (chipFamily: ESP32-C6) |
| **First-flash procedure** | Just plug in | Hold **BOOT** while plugging in USB, release after ~2 s |
| **Build flag** | none | `-DTINYTOSH_BOARD=XIAO_ESP32_C6` |

---

## Wiring (XIAO ESP32-C6)

The OLED and optional TTP223 touch sensor wire to the XIAO's standard I²C pair:

| Component | Pin | XIAO C6 connection |
|---|---|---|
| OLED VCC | — | **3V3** |
| OLED GND | — | **GND** |
| OLED SCL | — | **D5** (GPIO 23) |
| OLED SDA | — | **D4** (GPIO 22) |
| Touch VCC | — | **3V3** (NOT 5V) |
| Touch SIG | — | **D3** (GPIO 21) |
| Touch GND | — | **GND** |

You can reassign any of these in the web panel — the C6 build exposes GPIO 0–30
with warnings on pins the firmware considers risky (strapping, flash, UART).

### Pins the auto-pick avoids on C6

The firmware's `validatePins()` will not pick these for I²C or touch:

- `GPIO 3, 14` — RF switch / antenna select (firmware-managed at boot)
- `GPIO 4, 5, 6, 7` — JTAG pads on the back of the XIAO
- `GPIO 8, 9` — ESP32-C6 strapping pins
- `GPIO 15` — ESP32-C6 strapping pin (JTAG MTDO)
- `GPIO 16, 17` — UART0 TX/RX (used by USB-CDC serial monitor)
- `GPIO 18, 19` — connected to internal flash (FSPIQ, FSPID)

You can still override any pin manually in the web panel if you know what
you're doing — the ⚠️ marker is a hint, not a wall.

---

## Option A — Web Flasher (easiest)

1. Open the GitHub Pages site for this branch:
   ```
   https://<your-gh-user>.github.io/Tinytosh-C6/
   ```
2. Plug your XIAO C6 into USB-C.
3. **First-time flash only:** Hold the **BOOT** button on the XIAO while plugging in.
   Release after ~2 seconds. (Subsequent flashes work without this.)
4. Click **Connect & Flash XIAO ESP32-C6**.
5. Pick the serial port that appears, click **Install**, wait ~60 seconds.
6. Done — the OLED should show "Tinytosh is Ready".

The web installer uses the [ESP Web Tools](https://esptool-js.github.io/esp-web-tools/)
component, which talks directly to your browser's WebSerial API. No drivers
needed (the XIAO C6 uses native USB).

---

## Option B — Arduino IDE

### Board settings

| Setting | Value |
|---|---|
| Board | `XIAO_ESP32C6` (or `ESP32C6 Dev Module` if XIAO variant not listed) |
| USB CDC On Boot | **Enabled** |
| CPU Frequency | 160 MHz |
| Flash Frequency | 80 MHz |
| Flash Mode | QIO |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | **Huge APP (3MB No OTA/1MB SPIFFS)** — required, the default 1.2MB partition is too small |
| Upload Speed | 921600 |

### Build flag

In **Tools → ...** add the following to `build.extra_flags`:

```
-DTINYTOSH_BOARD=XIAO_ESP32_C6
```

### Libraries (same as C3 build)

| Library | Author | Install via |
|---|---|---|
| WiFiManager | tzapu | Library Manager (use 2.0.16-rc.2 — see "Known issues" below) |
| ArduinoJson | Benoit Blanchon | Library Manager |
| Adafruit SSD1306 | Adafruit | Library Manager (install all dependencies) |
| Adafruit GFX Library | Adafruit | Library Manager |
| OneButton | Matthias Hertel | Library Manager |
| PubSubClient | Nick O'Leary | Library Manager |

### Known issues with library compatibility

ESP32 Arduino Core v3.x renamed `ESP32` to `ARDUINO_ARCH_ESP32`, which breaks
older libraries that use `#ifdef ESP32` guards. The following patches are
needed if you hit compile errors:

| File | Fix |
|---|---|
| `Adafruit_SSD1306.cpp` | Add `\|\| defined(ARDUINO_ARCH_ESP32)` to the `util/delay.h` guard |
| `Adafruit_BusIO/Adafruit_SPIDevice.h` | Same: `defined(ESP32) \|\| defined(ARDUINO_ARCH_ESP32)` |
| `WiFiManager.h` & `.cpp` | Replace all `defined(ESP32)` with `defined(ESP32) \|\| defined(ARDUINO_ARCH_ESP32)`; replace `WiFi.hostname()` (0-arg) with `WiFi.getHostname()`; replace `system_get_sdk_version()` with `esp_get_idf_version()`; fix `HTTP_INFO_chiprev`/`HTTP_INFO_aphost` typos; replace `AUTH_MODE_NAMES[]` with a switch on `wifi_auth_mode_t`; rewrite the `WiFiEvent` callback registration using a lambda |

For a clean reproducible build, use the [`arduino-cli` recipe](#option-c--platformio)
or the **arduino-cli one-liner** below — it handles the patches automatically.

### arduino-cli one-liner (recommended for headless builds)

```bash
arduino-cli compile \
  --fqbn "esp32:esp32:XIAO_ESP32C6:UploadSpeed=921600,CDCOnBoot=cdc,PartitionScheme=huge_app" \
  --build-property "build.extra_flags=-DTINYTOSH_BOARD=XIAO_ESP32_C6" \
  --output-dir build/output \
  TinytoshESP32/TinytoshESP32.ino
```

Output: `build/output/TinytoshESP32.ino.merged.bin` — this is your `Tinytosh-C6.bin`.

---

## Option C — PlatformIO

`platformio.ini` snippet:

```ini
[env:xiao-esp32c6]
platform = espressif32
board = esp32-c6-devkitc-1
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = huge_app.csv
build_flags = -DTINYTOSH_BOARD=XIAO_ESP32_C6
```

The `board_build.partitions = huge_app.csv` line is **required** — the default
1.2 MB partition is too small for Tinytosh on the C6.

Then:

```bash
pio run -e xiao-esp32c6 -t upload
pio device monitor
```

---

## Generating the C6 firmware .bin for the web flasher

The web flasher expects `Tinytosh-C6.bin` next to `manifest_c6.json`. To
produce it:

### From Arduino IDE

1. **Sketch → Export Compiled Binary** (with the XIAO C6 board + build flag set).
2. The .bin lands in your sketch folder. Rename it to `Tinytosh-C6.bin`.

### From PlatformIO

```bash
pio run -e xiao-esp32c6
# Output: .pio/build/xiao-esp32c6/firmware.bin
cp .pio/build/xiao-esp32c6/firmware.bin Tinytosh-C6.bin
```

### From esptool (merging partitions for a single-file flash)

```bash
python3 -m esptool --chip esp32c6 merge_bin \
  --output Tinytosh-C6.bin \
  --flash_mode dio --flash_size 4MB \
  0x0   .pio/build/xiao-esp32c6/bootloader.bin \
  0x8000 .pio/build/xiao-esp32c6/partitions.bin \
  0x10000 .pio/build/xiao-esp32c6/firmware.bin
```

(Replace `--flash_mode dio` with `qio` if your C6 variant uses QIO flash —
the XIAO C6 typically uses **QIO**.)

---

## Deploying the web flasher to GitHub Pages

```bash
git checkout -b feature/xiao-esp32-c6
git add firmware/ index.html style.css script.js manifest.json manifest_c6.json
git commit -m "Add XIAO ESP32-C6 support + web flasher"
git push origin feature/xiao-esp32-c6

# Enable GitHub Pages on this branch at / (root)
gh repo edit --enable-pages --pages-source-branch feature/xiao-esp32-c6
```

Then visit `https://<user>.github.io/Tinytosh-C6/` and the **Connect & Flash
XIAO ESP32-C6** button will be live.

---

## Troubleshooting

### "Web installer can't see my serial port"

- Make sure you're using a **Chromium-based browser** (Chrome, Edge, Opera, Brave).
- WebSerial only works on `localhost` or **HTTPS** — `file://` won't cut it.
- Hold **BOOT** on the XIAO while plugging in, release after 2 s.

### Serial Monitor shows nothing

- `Tools → USB CDC On Boot` must be **Enabled** — this is mandatory for XIAO C6
  since it has no USB-to-serial bridge chip.

### OLED stays blank

- Confirm wiring: SDA → D4, SCL → D5.
- Try re-plugging; the I²C scanner won't run if SDA is shorted to GND.
- The web panel lets you reassign pins — if you moved the OLED to different
  GPIOs, update them there and reboot.

### WiFi keeps dropping on C6

- The XIAO C6's ceramic antenna works well for desk-side setups. If you're in
  a metal enclosure or far from the router, reflowing the antenna-select pad
  and using the U.FL external antenna will help significantly. See Seeed's
  wiki for the antenna-switch procedure.

---

## Credits

Original firmware by **Vladimir Gitsarev** — <https://github.com/VladimirGitsarev/Tinytosh>.
XIAO ESP32-C6 port + web flasher fork by the Tinytosh-C6 contributors.

Licensed under MIT.

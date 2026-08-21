#include "BatteryService.h"
#include <Arduino.h>

BatteryService::BatteryService() {}

void BatteryService::update(BatteryData& data) {
  uint32_t sumMilliVolts = 0;

  // ESP32 Arduino Core v3.x exposes analogReadMilliVolts() which
  // does its own internal calibration per the eFuse data. We average
  // ADC_SAMPLES readings to smooth out noise from the charger IC.
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sumMilliVolts += analogReadMilliVolts(BAT_ADC_PIN);
    delay(2);
  }

  // Convert mV at the ADC pin to V at the BAT pad (compensate for the 1:2
  // divider). Seeed's reference example uses *2 / 16 / 1000.0.
  float voltsAtAdc = (float)sumMilliVolts / (float)ADC_SAMPLES / 1000.0f;
  float voltsAtBat = voltsAtAdc * DIVIDER_RATIO;

  data.voltage      = voltsAtBat;
  data.percent      = voltageToPercent(voltsAtBat);
  data.last_update  = millis();

  // Cheap heuristic: while USB is plugged in the red LED draws a small
  // voltage through the BAT pin via the charger. We can't read the USB
  // 5V rail directly without extra hardware, but if the BAT voltage
  // sits at or above 4.15V the charger is almost certainly active.
  // (Fully-discharged cell tops out around 4.20V when charging.)
  data.charging = (voltsAtBat >= 4.15f);
}

bool BatteryService::hasBattery(const BatteryData& data) {
  return data.voltage >= 2.5f && data.voltage <= 4.5f;
}

int BatteryService::voltageToPercent(float voltage) {
  // Out-of-range → unknown
  if (voltage < 2.5f || voltage > 4.5f) return -1;

  // Piecewise-linear LiPo discharge curve, anchored at the canonical
  // voltage-vs-% points. Each segment is a linear interpolation.
  //   4.20V → 100%
  //   3.85V →  75%
  //   3.70V →  50%
  //   3.55V →  25%
  //   3.30V →   5%
  //   <3.30 →   0%
  static const struct {
    float v;
    int   p;
  } CURVE[] = {
    {4.20f, 100},
    {3.85f,  75},
    {3.70f,  50},
    {3.55f,  25},
    {3.30f,   5},
  };

  if (voltage >= 4.20f) return 100;
  if (voltage <= 3.30f) return 0;

  for (size_t i = 0; i + 1 < sizeof(CURVE) / sizeof(CURVE[0]); i++) {
    float vHi = CURVE[i].v;
    int   pHi = CURVE[i].p;
    float vLo = CURVE[i + 1].v;
    int   pLo = CURVE[i + 1].p;
    if (voltage <= vHi && voltage >= vLo) {
      float span = vHi - vLo;
      if (span <= 0) return pHi;
      float frac = (voltage - vLo) / span;
      return pLo + (int)((pHi - pLo) * frac + 0.5f);
    }
  }
  return 0;
}
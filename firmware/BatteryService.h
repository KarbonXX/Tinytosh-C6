// BatteryService.h — Reads BAT voltage on the Seeed XIAO ESP32-C6.
//
// Hardware: solder a 200kΩ resistor in a 1:2 voltage divider configuration
// between the BAT+ pad and the A0 (D0 = GPIO0) analog input. The on-board
// divider on most XIAO boards already does this for the BAT reading — Seeed's
// wiki example assumes it. If you see half the expected voltage, your board
// doesn't have the divider and you need to add the resistor.
//
// The charger chip is the SGM40567-4.2 — it charges up to 4.20V and there is
// NO on-board over-discharge protection. Always use a LiPo cell with its own
// internal BMS, or wire an external TP4056 + protection circuit.
//
// LiPo discharge curve (rough, for the percentage display):
//   4.20V = 100%   (full charge)
//   3.85V = ~75%
//   3.70V = ~50%
//   3.55V = ~25%
//   3.30V = ~5%    (low-battery warning threshold)
//   <3.00V = 0%    (cell damaged if sustained)
//
#ifndef BATTERY_SERVICE_H
#define BATTERY_SERVICE_H

#include "structs.h"

class BatteryService {
public:
    BatteryService();

    // Update `data` with a fresh ADC reading from the BAT pad.
    // Cheap to call (a few ms) — safe to invoke from loop() or the
    // periodic update task.
    void update(BatteryData& data);

    // Returns true if a LiPo is connected (voltage in the 2.5V..4.5V range).
    // Returns false if the reading is implausible (no battery / disconnected).
    static bool hasBattery(const BatteryData& data);

    // Map a raw voltage reading to a 0..100 percentage using a piecewise
    // LiPo discharge curve. Returns -1 if voltage is out of plausible range.
    static int voltageToPercent(float voltage);

private:
    // Number of ADC samples to average per update. 16 reads ≈ 5 ms at
    // ESP32 default ADC speed. Seeed's reference example uses 16.
    static constexpr int ADC_SAMPLES = 16;

    // The XIAO ESP32-C6 maps A0 → D0 → GPIO0. Pin constant lives here so
    // other modules don't sprinkle GPIO0 literals around.
    static constexpr int BAT_ADC_PIN = 0;

    // 1:2 voltage divider compensation — Seeed's official example.
    static constexpr float DIVIDER_RATIO = 2.0f;
};

#endif
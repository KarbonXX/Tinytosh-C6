// WiFi Persistence Test for XIAO ESP32-C6
// Run this to verify WiFi credentials survive a power cycle.
// 
// Workflow:
//   1. Upload this sketch
//   2. Open Serial Monitor at 115200
//   3. Connect to "TinytoshTest" WiFi network, password "test1234"
//   4. Configure your home WiFi through the captive portal
//   5. Wait for "WiFi saved!" message
//   6. UNPLUG the XIAO, wait 10 seconds, PLUG IT BACK IN
//   7. Open Serial Monitor again — does it print "WiFi AUTO-CONNECTED on boot"?

#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

const char* AP_SSID = "TinytoshTest";
const char* AP_PASS = "test1234";
const char* PREF_NAMESPACE = "wm_test";

void checkPersistedCreds() {
  // Read WiFiManager's stored creds directly from NVS
  Preferences prefs;
  prefs.begin(PREF_NAMESPACE, true);
  String ssid = prefs.getString("ssid", "<NOT SET>");
  String pass = prefs.getString("pass", "<NOT SET>");
  prefs.end();
  
  Serial.println();
  Serial.println("=================================");
  Serial.println("NVS STORED WIFI CREDENTIALS:");
  Serial.print("  SSID: ""); Serial.print(ssid); Serial.println(""");
  Serial.print("  PASS: ""); Serial.print(pass); Serial.println(""");
  Serial.println("=================================");
  Serial.println();
  
  if (ssid.length() == 0 || ssid == "<NOT SET>") {
    Serial.println("*** NO CREDENTIALS STORED ***");
    Serial.println("This means WiFiManager hasn't saved them yet.");
  } else {
    Serial.println("*** CREDENTIALS FOUND ***");
    Serial.println("If you just powered on and see this WITHOUT configuring,");
    Serial.println("then WiFi persistence IS working.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println();
  Serial.println("�════════════════════════════════════════╗");
  Serial.println("║   WiFi Persistence Test - XIAO C6    ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();
  
  // Step 1: Check if creds already exist
  checkPersistedCreds();
  
  // Step 2: Try to auto-connect with saved creds
  WiFiManager wm;
  wm.setConnectTimeout(15);
  wm.setConnectRetries(3);
  wm.setConfigPortalTimeout(180);  // 3 minute portal timeout
  
  Serial.println("Attempting WiFiManager autoConnect()...");
  Serial.println("(If creds exist, this should connect in ~5-15 seconds)");
  Serial.println("(If no creds, this opens captive portal)");
  Serial.println();
  
  if (wm.autoConnect(AP_SSID, AP_PASS)) {
    Serial.println();
    Serial.println("✓✓✓ WiFi AUTO-CONNECTED on boot! ✓✓✓");
    Serial.print("  IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.println("If you reached this WITHOUT manually entering creds,");
    Serial.println("then WiFi PERSISTENCE IS WORKING.");
  } else {
    Serial.println();
    Serial.println("✗ WiFi FAILED to auto-connect.");
    Serial.println();
    Serial.println("If you JUST entered creds and saw this, WiFi is broken.");
    Serial.println("Re-entering creds in the portal should work.");
  }
  
  Serial.println();
  Serial.println("Test complete. Looping forever...");
}

void loop() {
  delay(1000);
}

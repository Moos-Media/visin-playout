// A non-blocking everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <chrono>
#include <thread>

#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#ifdef ESP32
// Cannot use 6 as output for ESP. Pins 6-11 are connected to SPI flash. Use 16 instead.
#define LED_PIN 4
#else
#define LED_PIN 6
#endif

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 50
uint16_t FRAMERATE = 50;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Replace with your network credentials
const char *ssid = "ESP32-Light-Server";
const char *password = "adminLukas";
const char *serverName = "http://192.168.4.2:8001/api/show/getCurrentBoard";
const char *brightnessURL = "http://192.168.4.2:8001/api/show/getBrightness";
WiFiClient client;
HTTPClient http;
int frameCounter = 0;

// setup() function -- runs once at startup --------------------------------
void setup()
{
  Serial.begin(115200);
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)

  WiFi.softAP(ssid, password);
  // Send request
  http.addHeader("Content-Type", "application/json");
  http.begin(client, serverName);
}

// loop() function -- runs repeatedly as long as board is on ---------------
void loop()
{
  int responseCode = http.POST("");

  if (responseCode == 200)
  {
    // Parse response
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());

    strip.clear();
    for (int i = 0; i < LED_COUNT; i++)
    {
      String colorName = doc["boardInfo"]["board"][i]["color"].as<String>();

      // Conected LEDs in demonstration unit is wired wrongly.
      // All Colors are sent as GRB instead of RGB.
      if (colorName == "WHITE")
      {
        strip.setPixelColor(i, 255, 255, 255);
      }
      else if (colorName == "GREEN")
      {
        strip.setPixelColor(i, 186, 128, 36);
      }
      else if (colorName == "COLOR1")
      {
        strip.setPixelColor(i, 107, 24, 162);
      }
      else if (colorName == "COLOR2")
      {
        strip.setPixelColor(i, 17, 192, 17);
      }
      else if (colorName == "COLOR3")
      {
        strip.setPixelColor(i, 178, 223, 52);
      }
      else if (colorName == "COLOR4")
      {
        strip.setPixelColor(i, 49, 216, 127);
      }
      else if (colorName == "COLOR5")
      {
        strip.setPixelColor(i, 102, 230, 47);
      }
      else if (colorName == "COLOR6")
      {
        strip.setPixelColor(i, 184, 0, 172);
      }
    }

    strip.show();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FRAMERATE));

    // Disconnect
    http.end();
  }

  frameCounter += 1;

  if (frameCounter > 200)
  {
    // End old connection
    frameCounter = 0;
    http.end();

    // Connect to new URL
    http.begin(client, brightnessURL);

    int responseCode = http.POST("");

    if (responseCode == 200)
    {
      // Parse response
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getStream());

      int newBrightness = doc["brightness"].as<int>();
      strip.setBrightness(newBrightness);
    }

    http.end();
    http.begin(client, serverName);
  }
}

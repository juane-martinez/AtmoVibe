#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>       //https://github.com/me-no-dev/ESPAsyncWebServer and download this library too: https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWiFiManager.h>     //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h>             //https://github.com/bblanchon/ArduinoJson/tree/5.x
#include <SimpleTimer.h>             //https://github.com/jfturcot/SimpleTimer
#include "ThingSpeak.h"              //https://github.com/mathworks/thingspeak-arduino/
#include <FS.h>                      //library to access the filesystem
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>            //https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>        //https://github.com/adafruit/Adafruit_SSD1306
//If using an earlier version of the Arduino IDE (prior to 1.8.10), also locate and install Adafruit_BusIO (newer versions will install this dependency automatically).

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
float t = 0;

//define custom fields
char limite_malo[100];
char limite_bueno[100];
char api_key[100];
char numero_canal[100];
char offset_calibracion[100];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

AsyncWebServer server(80);
DNSServer dns;
SimpleTimer timer;


//rutina de reinicio
void reinicio() {
  ESP.restart();
}


void setup() {

  Serial.begin(115200);

  //oled
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();

  //logo iberotecno
  // 'iberomap', 128x32px
  const unsigned char logo [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x20, 0x0f, 0xf7, 0xfc, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x60, 0xf0, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x1f, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xc1, 0xe0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x3e, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x38, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0xd8, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1f, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x38, 0xc1, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x60, 0x31, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xc0, 0x08, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0xe4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x07, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x78, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x81, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x10, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x20, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x41, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xce, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x07, 0xe0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x07, 0x80, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x18, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  display.drawBitmap(0, 0, logo, 128, 64, WHITE);
  display.display();
  delay(3000);

  //reset pins
  pinMode(26, INPUT);           // set pin to input
  //  digitalWrite(26, LOW);

  //
  //***clean FS and wifi settings for testing if pin 26 high on boot
  if (digitalRead(26) == HIGH) {
    Serial.println("Formating SPIFFS...");
    if (SPIFFS.format()) {
      Serial.println("SPIFFS Formatted ok.");
      delay(1000);
    }
    else {
      Serial.println("SPIFFS Format error. Restarting.");
      ESP.restart();
      delay(1000);
    }

    //wifiManager.resetSettings();
    WiFi.disconnect(true);   // still not erasing the ssid/pw. Will happily reconnect on next start
    WiFi.begin("0", "0");      // adding this effectively seems to erase the previous stored SSID/PW
    Serial.println("Wifi credentials reset. Restarting.");
    delay(1000);
    ESP.restart();
    delay(1000);
  }


  //read configuration from FS json
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument jsonDocument(1024); // Ajusta el tamaño según sea necesario
        DeserializationError error = deserializeJson(jsonDocument, buf.get());
        
        if (!error) {
          JsonObject json = jsonDocument.as<JsonObject>();
          //json.printTo(Serial); // Esta línea puede ser removida ya que es solo para debugging
          strcpy(limite_malo, json["limite_malo"]);
          strcpy(limite_bueno, json["limite_bueno"]);
          strcpy(api_key, json["api_key"]);
          strcpy(numero_canal, json["numero_canal"]);
          strcpy(offset_calibracion, json["offset_calibracion"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  //end read
  AsyncWiFiManagerParameter custom_limite_bueno("limite_bueno", "Valor aire bueno", limite_bueno, 100);
  AsyncWiFiManagerParameter custom_limite_malo("limite_malo", "Valor aire malo", limite_malo, 100);
  AsyncWiFiManagerParameter custom_api_key("api_key", "Write API Key", api_key, 100);
  AsyncWiFiManagerParameter custom_numero_canal("numero_canal", "Channel ID", numero_canal, 100);
  AsyncWiFiManagerParameter custom_offset_calibracion("offset_calibracion", "Offset", offset_calibracion, 100);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server, &dns);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_limite_bueno);
  wifiManager.addParameter(&custom_limite_malo);
  wifiManager.addParameter(&custom_api_key);
  wifiManager.addParameter(&custom_numero_canal);
  wifiManager.addParameter(&custom_offset_calibracion);

  //scheduled reset
  int horas = 1; // hours for scheduled reset
  timer.setInterval(horas * 3600000, reinicio);

  //WiFiManager
  AsyncWiFiManagerParameter custom_text("<p>Seleccione la red WiFi para conectarse.</p>");
  wifiManager.addParameter(&custom_text);

  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Iberotecno AP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  display.clearDisplay();

  //read updated parameters
  strcpy(limite_malo, custom_limite_malo.getValue());
  strcpy(limite_bueno, custom_limite_bueno.getValue());
  strcpy(api_key, custom_api_key.getValue());
  strcpy(numero_canal, custom_numero_canal.getValue());
  strcpy(offset_calibracion, custom_offset_calibracion.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument jsonDocument(1024); // Ajusta el tamaño según sea necesario
    JsonObject json = jsonDocument.to<JsonObject>();
    json["limite_malo"] = limite_malo;
    json["limite_bueno"] = limite_bueno;
    json["api_key"] = api_key;
    json["numero_canal"] = numero_canal;
    json["offset_calibracion"] = offset_calibracion;


    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    serializeJson(json, configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client;
  //WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client); // Initialize ThingSpeak

  timer.run();
  int limite_malo_convertido = atoi(limite_malo);
  int limite_bueno_convertido = atoi(limite_bueno);
  int offset_calibracion_convertido = atoi(offset_calibracion);
  int offstet_mq_135 = 400;

  unsigned long numero_canal_convertido = strtoul(numero_canal, NULL, 10);


  int medicion = analogRead(34);  // Read sensor value and stores in a variable medicion
  int t = medicion + offstet_mq_135 + offset_calibracion_convertido;
  int icono;
  Serial.print("Medicion raw = ");
  Serial.println(medicion);
  Serial.print("Offset = ");
  Serial.println(offset_calibracion_convertido);
  Serial.print("Co2 ppm = ");
  Serial.println(t);
  String analisis;

  if (t <= limite_bueno_convertido)
  {
    Serial.println("Aire puro");
    analisis = "BUENO ";
    icono = 0x02;
  }
  else if ( t >= limite_bueno_convertido && t <= limite_malo_convertido )
  {
    Serial.println("Aire cargandose");
    analisis = "REGULAR ";
    icono = 0x01;
  }
  else if (t >= limite_malo_convertido )
  {
    Serial.println("Aire viciado");
    analisis = "MALO ";
    icono = 0x13;

  }
  Serial.println(WiFi.isConnected() ? "WiFi conectado" : "WiFi desconectado");


  ThingSpeak.setField(1, t); //co2, ppm
  int x = ThingSpeak.writeFields(numero_canal_convertido, api_key);

  //inicio oled

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner

  //rssi
  long rssi = WiFi.RSSI();
  Serial.print("rssi:");
  Serial.println(rssi);
  int bars;
  //  int bars = map(rssi,-80,-44,1,6); // this method doesn't refelct the Bars well
  // simple if then to set the number of bars

  if (rssi >= -55) {
    bars = 5;
  } else if (rssi <= -55 & rssi >= -65) {
    bars = 4;
  } else if (rssi <= -65 & rssi >= -70) {
    bars = 3;
  } else if (rssi <= -70 & rssi >= -78) {
    bars = 2;
  } else if (rssi <= -78 & rssi >= -82) {
    bars = 1;
  } else {
    bars = 0;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Cantidad de CO2");
  for (int b = 0; b <= bars; b++) {
    //display.fillRect(59 + (b*5),33 - (b*5),3,b*5,WHITE);
    //display.fillRect(59 + (b*5),33 - (b*3),3,b*4,WHITE);
    display.fillRect(95 + (b * 5), 10 - (b * 2), 3, b * 2, WHITE);
  }
 
  display.setTextColor(WHITE);
  display.setTextSize(1);

  //valor co2
  display.setTextSize(4);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 17);
  display.print(t);

  //ppm
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(100, 17);
  display.println(F("PPM"));

  //calidad
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.setCursor(0, 55);
  display.print(" AIRE ");
  display.println(analisis);

  //icono
  display.setTextSize(4);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(100, 32);
  display.write(icono);


  display.display();

  if (analisis == "MALO ") {
    //
  }


  // end oled
  delay(15000); // Wait 15 seconds to update the channel again

  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));

  }
}
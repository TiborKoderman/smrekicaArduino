#include "WiFi.h"
#include "PubSubClient.h"
#include <Ethernet.h>

#include <FastLED.h>

#define FASTLEDPIN     15
#define NUM_LEDS    100
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

int CurrentMode = 1;

#define UPDATES_PER_SECOND 100

char ssid[32] = "SSID";
char password[32] = "Password";
char clientID[32] = "Smrekica";

WiFiClient ethClient;


CRGBPalette16 currentPalette;
TBlendType    currentBlending;

PubSubClient client(ethClient);
const char* mqtt_server = "mqtt_server_ip";

void setup(){
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, FASTLEDPIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  Serial.begin(115200);

  currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback);

    trywifi();
}

void trywifi()
{
  for(int i = 0; i< 1 && WiFi.status() != WL_CONNECTED; i++)
    {
    Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, password);
    delay(10000);
    }
 if(WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected to the WiFi network");
      Serial.print("Ip: ");
      Serial.println(WiFi.localIP()); 
      mqttconnect();
    }
    
}

void mqttconnect() {
  // Loop until reconnected
  
    Serial.print(client.state());
    Serial.print("MQTT connecting ...");
    // client ID 
    //const char* clientId = "ESP32";
    // connect now 
    if (client.connect("Smrekica", "","")) {
      Serial.println("connected");
      // subscribe topic with default QoS 0
      client.subscribe("treeColor");
      client.subscribe("treeMode");
      
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      // Wait 5 seconds before retrying
    }
}

void mqtt_callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:"); 
    Serial.write(payload, length);
    Serial.println();
    String messageTemp;
    
    for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    messageTemp += ((char)payload[i]);
    }

    if(String(topic)=="treeColor")
    {
      for(int i =0; i<NUM_LEDS;i++)
        {
          leds[i] = (int)strtol(messageTemp.c_str(), NULL,16);
        }
        FastLED.show();
        CurrentMode = 0;
    }

    if(String(topic)=="treeMode")
    {
      if(messageTemp == "next")
        Serial.println(++CurrentMode);
      else if(messageTemp=="prev")
        Serial.println(--CurrentMode);
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,

    CRGB::Blue,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Blue,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Blue,
    
};


void drawTransFlag()
{
  int i = 0;

  for(i;i<20;i++)
    leds[i] = (int)strtol("0x5bcefa", NULL,16);
  for(i;i<40;i++)
    leds[i] = (int)strtol("0xf5a9b8", NULL,16);
  for(i;i<60;i++)
    leds[i] = (int)strtol("0xffffff", NULL,16);
  for(i;i<80;i++)
    leds[i] = (int)strtol("0xf5a9b8", NULL,16);
  for(i;i<100;i++)
    leds[i] = (int)strtol("0x5bcefa", NULL,16);
  FastLED.show();
}


void loop(){
if( CurrentMode != 0)
  {
  static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
  }
  if( CurrentMode == 1)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND;}
  if( CurrentMode == 2)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
  if( CurrentMode == 3)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
  if( CurrentMode == 4)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
  //if( CurrentMode == 5)  { drawTransFlag();}
     if(CurrentMode>4)
      CurrentMode = 1;
     if(CurrentMode < 0)
      CurrentMode=4;
  FastLED.delay(1000 / UPDATES_PER_SECOND);
  client.loop();
}

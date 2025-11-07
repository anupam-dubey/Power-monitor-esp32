#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>
#include <PZEM004Tv30.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi.h>

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "projectiot"
#define WLAN_PASS       "iot123456"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "poweriot2025"
#define AIO_KEY         "aio_ffxa27nOcvqfRSsqPf17KsrOvikI"



// Define the UART2 RX and TX pins on ESP32 (Connect these to PZEM-004T)
#define PZEM_RX_PIN 16  // ESP32 RX (Connect to PZEM TX)
#define PZEM_TX_PIN 17  // ESP32 TX (Connect to PZEM RX)

// Initialize the PZEM sensor using Hardware Serial2
PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define RLY1 26
#define RLY2 27
#define CLED 25
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish tvoltage = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/voltage");
Adafruit_MQTT_Publish tcurrent = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/current");
Adafruit_MQTT_Publish tpower = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/power");
Adafruit_MQTT_Publish tfreq = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/frequency");
Adafruit_MQTT_Publish tpf = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/powerfactor");
Adafruit_MQTT_Publish tenergy = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/energy");
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe tl1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/load1");


int wificounter=0;
int readcounter=0;
bool nowifi=0;
byte sendflag=0;
void setup() {
  Serial.begin(115200);
  pinMode(RLY1,OUTPUT);pinMode(CLED,OUTPUT);
  digitalWrite(RLY1,LOW);digitalWrite(CLED,HIGH);
  
  
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 2 seconds
   
  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("IOT POWER"));
  display.setCursor(0,16);             // Start at top-left corner
  display.println(F("Monitoring"));
  display.setCursor(0,32);             // Start at top-left corner
  display.println(F("GGITS EX"));
  display.setCursor(0,48);             // Start at top-left corner
  display.println(F("2025-2026"));
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  
   WiFi.begin(WLAN_SSID, WLAN_PASS);
   wificounter=0;nowifi=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    wificounter++;
    Serial.print(".");
    if(wificounter>50)
    break;
  }
  if(wificounter>50)
  {
   Serial.println();
  Serial.println("WiFi not connected"); 
  display.clearDisplay();
    display.setCursor(0,0);  //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("NO WIFI");
     display.display();nowifi=1;
    delay(2000);
  }
  else
  {
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  display.clearDisplay();
    display.setCursor(0,0);  //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("Connected!");
     display.display();
    delay(2000);
  }  
   while (true) {
      Serial.println("Connecting to PZEM...");
      if(pzem.readAddress())
        break;
      delay(1000);
   }
 readcounter=0;sendflag=0;
 // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&tl1);

}

void loop() {
  Serial.print("Custom Address:");
   
    // Read data from the PZEM sensor
    float voltage   = pzem.voltage();
    float current   = pzem.current();
    float power     = pzem.power();
    float energy    = pzem.energy();
    float frequency = pzem.frequency();
    float pf        = pzem.pf();
   
  

    // Check if the data is valid
    if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    
    } else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("PowerFactor: ");        Serial.print(pf,1);        Serial.println(" ");
        Serial.print("Frequency: ");       Serial.print(frequency,0);     Serial.println("Hz");
        display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(WHITE); 
        display.setCursor(0,0); display.print("V=");display.print(voltage);
        display.setCursor(0,15); display.print("I=");display.print(current);
        display.setCursor(0,30); display.print("(W)=");display.print(power);
        display.setCursor(0,45); display.print("(KWH)=");display.print(energy);
        display.display();    
       if(nowifi==0 && sendflag==1)
       {
        tvoltage.publish(voltage);
        tfreq.publish(frequency);
        tcurrent.publish(current);
        tpf.publish(pf);
        tenergy.publish(energy);
        tpower.publish(power);
        sendflag=0;
       }
    }

    if(nowifi==0)
  {
  MQTT_connect();
  }
    delay(2000);
    readcounter++;
    if(readcounter>10)
    {
      readcounter=0;sendflag=1;
    }
     Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &tl1) {
      // convert mqtt ascii payload to int
    char *value = (char *)tl1.lastread;
      Serial.print(F("Got: "));
      Serial.println(value);
      // Apply message to lamp
    String message = String(value);
    message.trim();
    if (message == "ON") {digitalWrite(RLY1, LOW);}
    if (message == "OFF") {digitalWrite(RLY1, HIGH);}
   
    }
     
  }
}
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

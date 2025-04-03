#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>
#include <PZEM004T.h>

/*
   An example on how to use ESP32 hardware serial with PZEM004T
*/

HardwareSerial PzemSerial2(2);     // Use hwserial UART2 at pins IO-16 (RX2) and IO-17 (TX2)
PZEM004T pzem(&PzemSerial2);
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define RLY1 26
#define RLY2 27
#define CLED 25
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
IPAddress ip(192,168,1,1);

void setup() {
  Serial.begin(115200);
  pinMode(RLY1,OUTPUT);pinMode(RLY2,OUTPUT);pinMode(CLED,OUTPUT);
  digitalWrite(RLY1,HIGH);digitalWrite(RLY2,HIGH);digitalWrite(CLED,HIGH);
  
  
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Parameter1"));
  display.setCursor(0,14);             // Start at top-left corner
  display.println(F("Parameter2"));
  display.setCursor(0,28);             // Start at top-left corner
  display.println(F("Parameter3"));
  display.setCursor(0,40);             // Start at top-left corner
  display.println(F("Parameter4"));
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
   while (true) {
      Serial.println("Connecting to PZEM...");
      if(pzem.setAddress(ip))
        break;
      delay(1000);
   }

}

void loop() {
  Serial.print("Custom Address:");
   

    // Read the data from the sensor
    float voltage = pzem.voltage(ip);
    float current = pzem.current(ip);
    float power = pzem.power(ip);
    float energy = pzem.energy(ip);
    float pf = pzem.pf(ip);
    float frequency = pzem.frequency(ip);
  

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
       
    }

    Serial.println();
    delay(2000);
}

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <WiFi.h>

#define dataPin 6 // data pin for the LED

Adafruit_NeoPixel strip = Adafruit_NeoPixel(3,dataPin, NEO_GRB + NEO_KHZ800); //declare led strip
WiFiServer server(80); // declare wifi server (wifi shield)

char ssid[] = "ledcontrol001";     // the name of your network

int status = WL_IDLE_STATUS;     // the Wifi radio's status
boolean parseData = false;        // indicates if data to be parsed is available
String command = "";              // String to hold incoming data from the client
uint8_t colorIndex = 0;           // holds position of color in color array
uint8_t patternIndex = 0;         // holds the pattern number
uint8_t onOff = 0;                // holds state of ledstrip (on or off)

//color array
//ROYGBIV
uint32_t colors[] = {
  0xFF0000,
  0xFF4500,
  0xFFFF00,
  0x00FF00,
  0x0000FF,
  0x4B0082,
  0x9400D3
};

void setup() {
  Serial.begin(9600); // initialize serial
  strip.begin(); // initialize led strip
  strip.show(); // turn the leds to black/no color

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Done");
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = Serial.read(); // read a byte, then
        if (c == '#')parseData = false; // if the byte is a # character indicate end of command
        if (parseData)command += c; // capture incoming command
        if (c == '$')parseData = true;  // if the byte is a $ character indicate begining of command    
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  if (!parseData && command != ""){
    colorIndex = stringtoInt(command.substring(command.indexOf("C")+1, command.indexOf("P"))); // get the color index
    colorIndex = colorIndex-1;
    patternIndex = stringtoInt(command.substring(command.indexOf("P")+1, command.indexOf("\n"))); // get the pattern index
    onOff = command[0] - '0';
    Serial.print("Command: ");Serial.println(command);
    Serial.print("Status: ");Serial.println(onOff);
    Serial.print("Color index: ");Serial.println(colorIndex);
    Serial.print("Pattern: ");Serial.println(patternIndex);
    if (onOff == 0){
      Serial.println("Clear color");
      colorFill(strip.Color(0,0,0)); // clear led strip
    }
    command = "";
  }
  
  if (onOff == 1){
    if (patternIndex == 1){ // if there is no pattern
      colorFill(colors[colorIndex]); // fill led strip with chosen color
      patternIndex = 0;
    }
    else if (patternIndex > 1){
      switch (patternIndex){
        case 2: 
          colorWipe(colors[colorIndex], 50); // apply color wipe pattern
          colorWipe(strip.Color(0,0,0), 50); // clear colors
          break;
        case 3: theaterChase(colors[colorIndex], 50); // apply chase pattern
          break;
        case 4: theaterChaseRainbow(50); // apply theater chase rainbow
          break;
        case 5: rainbowCycle(50); // rainbow cycle
          break;
      }
    }
  }
  delay(10);
}
  

// convert from string to int
int stringtoInt(String buff){
  int r = 0;
  for (uint8_t i = 0; i < buff.length(); i++){
   r = (r*10) + (buff[i] - '0');
  }
  return r;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
// Fill the dots one after the other with a color
void colorFill(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
        delay(wait);
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}


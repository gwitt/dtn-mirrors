// Simple strand test for Adafruit Dot Star RGB LED strip.
// This is a basic diagnostic tool, NOT a graphics demo...helps confirm
// correct wiring and tests each pixel's ability to display red, green
// and blue and to forward data down the line.  By limiting the number
// and color of LEDs, it's reasonably safe to power a couple meters off
// the Arduino's 5V pin.  DON'T try that with other code!

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 104 // Number of LEDs in strip
#define NUMSPARKS 10
#define NUMGLOWS 60
#define GLOWRADIUS 20
#define GLOWTIME 1000

// Here's how to control the LEDs from any two pins:
#define DATAPIN    4
#define CLOCKPIN   5

#define matPin 2
#define gndPin 3


Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

unsigned long glowstarts[NUMGLOWS];
unsigned long glowlocs[NUMGLOWS];

unsigned long sparkstarts[NUMSPARKS];
int sparklengths[NUMSPARKS];
int sparklocs[NUMSPARKS];

unsigned char pixels[NUMPIXELS];

void setup() {
  pinMode(matPin, INPUT);
  digitalWrite(matPin, HIGH);

  pinMode(gndPin, OUTPUT);
  digitalWrite(gndPin, LOW);
  
  for (int i=0; i<NUMSPARKS; i++){
    sparklocs[i]= random(NUMPIXELS);
    sparkstarts[i]= random(2000);
    sparklengths[i]= random(200);
  }
  
  for (int i=0 ; i<NUMGLOWS; i++){
    glowstarts[i]= random(500);
    glowlocs[i]= GLOWRADIUS + random(NUMPIXELS-2*GLOWRADIUS);
  }

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}

// Runs 10 LEDs at a time along strip, cycling through red, green and blue.
// This requires about 200 mA for all the 'on' pixels + 1 mA per 'off' pixel.

void loop() {
  for (int i=0; i< NUMPIXELS; i++) pixels[i]= 0;
  
  for (int i=0; i<NUMGLOWS; i++){
    if (millis() > glowstarts[i] + GLOWTIME){
      if (!digitalRead(A0)){
        glowstarts[i]= millis()+random(1000);
        glowlocs[i]= random(NUMPIXELS);
        //glowlocs[i]= GLOWRADIUS + random(NUMPIXELS-2*GLOWRADIUS);
      }
    }
    
    int level=0;
    if (millis () > glowstarts[i]) level= map(millis(), glowstarts[i], glowstarts[i]+GLOWTIME/2, 0, 255);
    if (millis () > glowstarts[i]+GLOWTIME/2) level= map(millis(), glowstarts[i]+GLOWTIME/2, glowstarts[i]+GLOWTIME, 100, 0);
    if (millis() > glowstarts[i]+GLOWTIME) level=0;
    addLight(glowlocs[i], level);
    for (int j=0; j<GLOWRADIUS; j++){
      level=level/2;
      int loc= glowlocs[i];
      addLight(loc-j, level);
      addLight(loc+j, level);
    }
    
  }
    
  for (int i=0; i< NUMSPARKS; i++){
    if (millis() > sparkstarts[i] + sparklengths[i]){
      //sparklocs[i]= random(NUMPIXELS);
      //sparkstarts[i]= millis()+random(2000);
      //sparklengths[i]= random(200, 800);
    }
    //if (millis() > sparkstarts[i]) addLight(sparklocs[i], 255);
    
  }
  
  if(digitalRead(matPin)){
    for (int i=0; i<NUMPIXELS; i++) if (pixels[i] >0) pixels[i]--;
  }
  
  for (int i=0; i<NUMPIXELS; i++) strip.setPixelColor(i, pixels[i]);
  strip.show();                     // Refresh strip
  //delay(50);                        // Pause 20 milliseconds (~50 FPS)
  
}



void addLight(int loc, int val){
  if(loc < 0) return;
  if(loc >= NUMPIXELS) return;
  
  int oldVal= pixels[loc];
  int newVal= oldVal+val;
  if (newVal > 255) newVal= 255;
  pixels[loc]= newVal;
}

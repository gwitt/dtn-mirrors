#include <SPI.h>

#define NUMPIXELS 1200
#define LOOPLENGTH NUMPIXELS/2
#define NUMGLOWS 50
#define GLOWRADIUS 10
#define GLOWTIME 800

#define MATPIN 2
#define GNDPIN 3

unsigned long glowstarts[NUMGLOWS];
unsigned long glowlocs[NUMGLOWS];

uint16_t
    numLEDs;                                // Number of pixels
uint8_t
   *pixels;                                 // LED values (1 byte ea.)
   
void setup() {
  pinMode(MATPIN, INPUT);
  digitalWrite(MATPIN, HIGH);
  pinMode(GNDPIN, OUTPUT);
  digitalWrite(GNDPIN, LOW);
  
  numLEDs= NUMPIXELS;
  pixels= NULL;

  if(pixels) free(pixels);
  
  pixels = (uint8_t *)malloc(numLEDs);
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4); // 8 MHz (6 MHz on Pro Trinket 3V)
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  
  clear();
  show();  // Turn all LEDs off ASAP
  delay(5);
  clear();
  show();  // Turn all LEDs off ASAP
  
  for (int i=0 ; i<NUMGLOWS; i++){
    glowstarts[i]= random(500);
    glowlocs[i]= GLOWRADIUS + random(NUMPIXELS-2*GLOWRADIUS);
  }
}

void loop() {
  
  clear();
  
  for (int i=0; i<NUMGLOWS; i++){
    if (millis() > glowstarts[i] + GLOWTIME){
      if (!digitalRead(MATPIN)){
        glowstarts[i]= millis()+random(1000);
        glowlocs[i]= random(LOOPLENGTH);
      }
    }
    
    int level=0;
    if (millis () > glowstarts[i]) level= map(millis(), glowstarts[i], glowstarts[i]+GLOWTIME/2, 0, 255);
    if (millis () > glowstarts[i]+GLOWTIME/2) level= map(millis(), glowstarts[i]+GLOWTIME/2, glowstarts[i]+GLOWTIME, 100, 0);
    if (millis() > glowstarts[i]+GLOWTIME) level=0;
    addLight(glowlocs[i], level);
    for (int j=0; j<GLOWRADIUS; j++){
      //level=level/2;
      int loc= glowlocs[i];
      addLight(loc-j, level);
      addLight(loc+j, level);
    }
  }
  
  if(digitalRead(MATPIN)){
    for (int i=0; i<NUMPIXELS; i++) if (pixels[i] >0) pixels[i]--;
  }
  
  for (int i=0; i<NUMPIXELS; i++) setPixelValue(i, pixels[i]);
  show();                     // Refresh strip
  //delay(50);                        // Pause 20 milliseconds (~50 FPS)
  
}

void addLight(int loc, int val){
  if(loc < 0) return;
  if(loc >= NUMPIXELS) return;
  
  int oldVal= getPixelValue(loc);
  int newVal= oldVal+val;
  if (newVal > 255) newVal= 255;
  setPixelValue(loc, newVal);
  setPixelValue(loc+LOOPLENGTH, newVal);
}

// ISSUE DATA TO LED STRIP -------------------------------------------------
// All of this is based on Adafruit's DotStar Library
// We don't have enough RAM to use their library w/ 1200 Pixels
// So their code was modified to only use one byte per pixel
// cause we don't need RGB

void show(void) {

  if(!pixels) return;

  uint8_t *ptr = pixels, i;            // -> LED data
  uint16_t n   = numLEDs;              // Counter

  uint8_t next;
  for(i=0; i<3; i++) (void)SPI.transfer(0x00);    // First 3 start-frame bytes
  SPDR = 0x00;                       // 4th is pipelined
  do {                               // For each pixel...
    while(!(SPSR & _BV(SPIF)));      //  Wait for prior byte out
    SPDR = 0xFF;                     //  Pixel start
    for(i=0; i<3; i++) {             //  For R,G,B...
      next = *ptr;                   //   Read, scale
      while(!(SPSR & _BV(SPIF)));    //    Wait for prior byte out
      SPDR = next;                   //    Write scaled color
    }
    ptr++;                         //   inc pointer
  } while(--n);
  
  while(!(SPSR & _BV(SPIF)));          // Wait for last byte out

  // Four end-frame bytes are seemingly indistinguishable from a white
  // pixel, and empirical testing suggests it can be left out...but it's
  // always a good idea to follow the datasheet, in case future hardware
  // revisions are more strict (e.g. might mandate use of end-frame
  // before start-frame marker).  i.e. let's not remove this.
  for(i=0; i<4; i++) (void)SPI.transfer(0xFF);
}
  
void clear() { // Write 0s (off) to full pixel buffer
  memset(pixels, 0, numLEDs);                   // 1 byte/pixel
}

void setPixelValue(uint16_t n, uint8_t v) {
  if(n < numLEDs) {
    uint8_t * p = &pixels[n];
    p[0] = v;
  }
}

uint8_t getPixelValue(uint16_t n) {
  if(n >= numLEDs) return 0;
  uint8_t *p = &pixels[n];
  return p[0];
}


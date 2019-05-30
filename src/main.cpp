#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_SleepyDog.h>
#include <MicroDebug.h>

#define ON_LED 13

#define CAP_THRESHOLD    150  // Threshold for a capacitive touch (higher = less sensitive).
#define CAP_SAMPLES      20   // Number of samples to take for a capacitive touch read.

uint8_t prev[] = {128, 128};              // Start brightness in middle
float lightLevel = 100;

#define LIGHT_ON_INFLUANCE  0.05
#define LIGHT_OFF_THRESHOLD 500
#define LIGHT_ON_THRESHOLD  200
#define LIGHT_UPDATE_TIME   (1000/10)

bool displayOn = false;
uint32_t lightReadTimeout = 0;

uint32_t displayOffTimeOut = 0;
#define DISPLAY_TIME_ON (5L * 60L * 60L * 1000L)
//#define DISPLAY_TIME_ON (10L * 1000L)

static void status(int i);

static void fire();
static void split(uint8_t y1, uint8_t y2, uint8_t offset, int start, int end);

static void colourCycle();

typedef void (*displayFn)();

static displayFn modes[] = {
  fire,
  colourCycle
};
static int mode = 1;

void setup()
{
  Serial.begin(115200);
//  while (!Serial) { }

  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(255); // LEDs full blast!

  pinMode(ON_LED, OUTPUT);
}

void loop() 
{
  bool sleep = false;
  
  if(CircuitPlayground.slideSwitch())
  {
//    if(millis() >= lightReadTimeout) 
//    {
//      int value = CircuitPlayground.lightSensor();
//      lightLevel = displayOn ?
//                    (float)lightLevel * (1-LIGHT_ON_INFLUANCE) + (float)value * LIGHT_ON_INFLUANCE : (float)value;
//      
//      DBUG("Light Sensor: ");
//      DBUG(value);
//      DBUG(", ");
//      DBUGLN(lightLevel);
//      lightReadTimeout = millis() + LIGHT_UPDATE_TIME;
//    }

//    int lightThreshold = displayOn ? LIGHT_OFF_THRESHOLD : LIGHT_ON_THRESHOLD;
//    if(lightLevel <= lightThreshold)
//    {
      if(false == displayOn)
      {
        // Turning the animations on start the timer
        displayOffTimeOut = millis() + DISPLAY_TIME_ON;
        DBUG(millis());
        DBUG(", ");
        DBUG(DISPLAY_TIME_ON);
        DBUG(", ");
        DBUGLN(displayOffTimeOut);
        displayOn = true;
      }

      if(millis() < displayOffTimeOut)
      {
        modes[mode]();
      } else {
        sleep = true;
        status(1);

        if(CircuitPlayground.leftButton() || CircuitPlayground.rightButton()) {
          displayOn = false;
        }
      }
//    } else {
//      displayOn = false;
//      sleep = true;
//      status(lightLevel / 100);
//    }
  }
  else
  {
    displayOn = false;
    sleep = true;
  }

  // If the display is off, turn off the LEDs and sleep for a second
  if(sleep)
  {
    for(uint8_t i=0; i < CircuitPlayground.strip.numPixels(); i++) CircuitPlayground.strip.setPixelColor(i, 0);
    CircuitPlayground.strip.show();
//    Watchdog.sleep(1000);
    delay(1000);
  }
}

static void status(int i) 
{
  for(; i > 0; i--) 
  {
    digitalWrite(ON_LED, HIGH);
    delay(50);
    digitalWrite(ON_LED, LOW);
    if(i > 1) {
      delay(150);
    }
  }
}

static void fire()
{
  for (int i = 0; i < 2; i++)
  {
    uint8_t lvl = random(64, 192);            // End brightness at 128±64
    split(prev[i], lvl, 32, i*5, 5 + (i*5));  // Start subdividing, ±32 at midpoint
    prev[i] = lvl;                            // Assign end brightness to next start
  }
}

static void split(uint8_t y1, uint8_t y2, uint8_t offset, int start, int end)
{
  if(offset) { // Split further into sub-segments w/midpoint at ±offset
    uint8_t mid = (y1 + y2 + 1) / 2 + random(-offset, offset);
    split(y1 , mid, offset / 2, start, end); // First segment (offset is halved)
    split(mid, y2 , offset / 2, start, end); // Second segment (ditto)
  } else { // No further subdivision - y1 determines LED brightness
    uint32_t c = (((int)(pow((float)y1 / 255.0, 2.7) * 255.0 + 0.5) // Gamma
                 * 0x1004004) >> 8) & 0xFF3F03; // Expand to 32-bit RGB color
    for(uint8_t i=start; i<end; i++) CircuitPlayground.strip.setPixelColor(i, c);
    CircuitPlayground.strip.show();
  }
}

static void colourCycle()
{
  uint32_t offset = millis() / 1000;
  uint32_t colour = CircuitPlayground.colorWheel(offset & 0xff);
  // Loop through each pixel and set it to an incremental color wheel value.
  for(uint16_t i = 0; i < CircuitPlayground.strip.numPixels(); ++i) {
    CircuitPlayground.strip.setPixelColor(i, colour);
  }
  // Show all the pixels.
  CircuitPlayground.strip.show();
}

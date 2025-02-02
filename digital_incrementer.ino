#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "dht_nonblocking.h"
#include "bitmaps.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET 4

#define SCREEN_ADDRESS 0x3C

// Indexes into the 'icons' array for snow animation.
#define XPOS   0 
#define YPOS   1
#define DELTAY 2
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define NUMFLAKES 10

#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 4;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

// Pin Values
// 7 Digit Display
int E = 13;
int D = 12;
int C = 11;
int Dp = 10;
int G = 9;
int F = 8;
int A = 7;
int B = 6;

// Button Pin
int buttonPin = 5;

// Temperature Pin
int temperaturePin = 4;

static int8_t f, icons[NUMFLAKES][3];

// Time management without delay snowflake animation
unsigned long previousMillisSnowflakes = 0;  // Store the last time snowflakes updated
const long intervalSnowflakes = 100; 

// Time Management for 7 segment display
unsigned long previousMillis = 0;
unsigned long previousSegmentMillis = 0;
const long intervalSegment = 500;  // 500ms interval
int step = 0;              // Keeps track of which digit to display next
int first, last;           // Store first and last digit globally
bool isTwoDigit = false;   // Flag to check if number has two digits

// Snowflake Icon
static const unsigned char PROGMEM snowflake_icon[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };


// ---------------------------- Start ----------------------------

void setup() {
  // ------------- Initialise Libraries ---------------
  Wire.begin();
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

   // Allow turn OLED to turn on
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  delay(100); // Initial DeBounce

  // Clear the buffer for oled.
  display.clearDisplay();

  // --------------------------------------------------

  // Initialise pins
  pinMode(E, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(Dp, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialise snowflakes animation 
  for (int f = 0; f < NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1, display.width() - LOGO_WIDTH);  // Random X position
    icons[f][YPOS]   = random(-LOGO_HEIGHT, 0);  // Start above the screen
    icons[f][DELTAY] = random(1, 4);  // Random falling speed
  }

}

// ---------------------------- Loop ----------------------------


int incrementHolder = 0; // To initially control manually changing digits.
int buttonState = 0; // Control button.
int x = 39, y = 7;    // Starting position.
int xSpeed = 2, ySpeed = 1; // Speed of movement.

float ifTemp = 0; // Temperature variable for checks.

// Animation Counters
int sun_counter = 0, cloud_counter = 0;

void loop() {
  
   // --------- Time Management ----------
  unsigned long currentMillis = millis(); 

  // ---------- Temperature Calculations -------------

  int tempReading = analogRead(DHT_SENSOR_PIN);
  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius

  float temperature;
  float humidity; // May use this in the future

  // ------- Animation Manager ------------

  // Temperature Control -> {[t < 15, snowflakes], [t 15-20, cloud], [t > 20, sun]}
  if(ifTemp < 15) 
  {
    if (currentMillis - previousMillisSnowflakes >= intervalSnowflakes) 
    {
      previousMillisSnowflakes = currentMillis;
      snowflakes(snowflake_icon, LOGO_WIDTH, LOGO_HEIGHT);  // Run the snowflakes animation
    }
  }
  else if(ifTemp >= 15 && ifTemp <= 20)
  {
      // Update scroll x&y
    x += xSpeed;
    y += ySpeed;

    // Check boundaries and bounce
    if (x <= 0 || x + 50 >= SCREEN_WIDTH) xSpeed *= -1; // Reverse direction if hitting left/right wall
    if (y <= 0 || y + 50 >= SCREEN_HEIGHT) ySpeed *= -1; 
    display.clearDisplay();
    display.drawBitmap(x, y, cloudallArray[cloud_counter], 50, 50, SSD1306_WHITE);
    display.display(); // Refresh screen

    cloud_counter = (cloud_counter + 1) % epd_bitmap_allArray_LEN; 
  }
  else 
  {
    // Update scroll x&y
    x += xSpeed;
    y += ySpeed;

    // Check boundaries and bounce
    if (x <= 0 || x + 50 >= SCREEN_WIDTH) xSpeed *= -1; // Reverse direction if hitting left/right wall
    if (y <= 0 || y + 50 >= SCREEN_HEIGHT) ySpeed *= -1; 
    display.clearDisplay();
    display.drawBitmap(x, y, epd_bitmap_allArray[sun_counter], 40, 40, SSD1306_WHITE);
    display.display(); // Refresh screen

    sun_counter = (sun_counter + 1) % epd_bitmap_allArray_LEN;
  }

  

// -------- Displaying Temperature on Segment Display ----------
if (currentMillis - previousSegmentMillis >= intervalSegment) {
  previousSegmentMillis = currentMillis; // Reset timer
  switch (step) {
      case 0:
        displayDigit(first);
     
        break;
      case 1:
        if (isTwoDigit) displayDigit(last);
 
        break;
      case 2:
        displayDigit(10);  // Show separator (.)
        break;
      case 3:
        displayDigit(10);  // Show separator (.)
        step = -1; // Reset to restart sequence
        break;
    }
    step++;
}
  // ---------- Button control ------------
  buttonState = digitalRead(buttonPin);

  // Control button functionality.
  if(buttonState == HIGH) {
    incrementHolder++;
    if(incrementHolder > 10) {
      incrementHolder = 0;
    }
    delay(200); // debounce
  }

  // --------- Temperature Control ----------
  if( measure_environment( &temperature, &humidity ) == true )
  {
      // Calculations to how temp
      displayTemp(temperature);
      ifTemp = temperature;
  }
}

// ---------------------------- Functions ----------------------------

// Uses dht_nonblocking library to grab temperature every 3 seconds.
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 3000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}

// Function to map numbers to 7-segment display 
const byte digitMap[11] = {
  0b01111110, // 0
  0b00110000, // 1
  0b01101101, // 2
  0b01111001, // 3
  0b00110011, // 4
  0b01011011, // 5
  0b01011111, // 6
  0b01110000, // 7
  0b01111111, // 8
  0b01111011,  // 9
  0b10000000  // .
};

// Programmitic animation of snow.
void snowflakes(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  // Initialize snowflake positions only once
  for (f = 0; f < NUMFLAKES; f++) {
    if (icons[f][XPOS] == 0) {  // If the snowflake hasn't been initialized
      icons[f][XPOS] = random(1 - LOGO_WIDTH, display.width());
      icons[f][YPOS] = -LOGO_HEIGHT;
      icons[f][DELTAY] = random(1, 6);
    }

    // Clear the display
    display.clearDisplay();
    
    // Draw each snowflake
    for (f = 0; f < NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
    }

    display.display();  // Show the snowflakes on the screen

    // Update coordinates for each snowflake
    for (f = 0; f < NUMFLAKES; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      if (icons[f][YPOS] >= display.height()) {  // If the snowflake is off the screen
        icons[f][XPOS] = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS] = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  }
}

// Display digit from 7 segment display
void displayDigit(int num) {
  if (num < 0 || num > 10) return; // Ensure valid digit

  byte pattern = digitMap[num]; // use mapping.

  digitalWrite(A, bitRead(pattern, 6));
  digitalWrite(B, bitRead(pattern, 5));
  digitalWrite(C, bitRead(pattern, 4));
  digitalWrite(D, bitRead(pattern, 3));
  digitalWrite(E, bitRead(pattern, 2));
  digitalWrite(F, bitRead(pattern, 1));
  digitalWrite(G, bitRead(pattern, 0));
  digitalWrite(Dp, bitRead(pattern, 7)); // DP (Decimal point)
}

// Calculates the first and last digits to display in Segment Display
void displayTemp(float temperature) {
  int temp = abs(int(temperature)); // Convert float to int
  first = temp;

  // Extract first digit
  while (first >= 10) {
    first /= 10;
  }

  // Extract last digit (only if two-digit number)
  last = (temp >= 10) ? temp % 10 : -1;
  isTwoDigit = (last != -1); // True if number has two digits
}



#include <arduinoFFT.h>
#include <FastLED.h>

//FFT constants
#define SAMPLES 128             //Must be a power of 2
#define SAMPLING_FREQUENCY 1000 //Hz, must be less than 10000 due to ADC
#define SAMPLE_PIN 5

//LED constants
#define LED_PIN     3
#define NUM_LEDS    11
#define BRIGHTNESS  32
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

//LED vars
CRGB leds[NUM_LEDS];
CRGB off[NUM_LEDS];
int brightness = 32;
int dimPeriod = 10;
unsigned long dimTimer;


//FFT vars
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];


//vars for sound detection
double prevReal[SAMPLES];
double baseAvg;
double prevBase;
int baseCount;
double midAvg;
double midPrevAvg;
int midCount;
double highAvg;
double highPrevAvg;
int highCount;

//designs
CRGB designA[NUM_LEDS] {CRGB::Red, CRGB::White, CRGB::Red, CRGB::White,CRGB::Red, CRGB::White,CRGB::Red, CRGB::White, CRGB::Red} ;
CRGB designB[NUM_LEDS] {CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Purple, CRGB::Red, CRGB::Orange, CRGB::Yellow};
CRGB designC[NUM_LEDS] {CRGB::White,CRGB::White,CRGB::White,CRGB::White,CRGB::White,CRGB::White,CRGB::White,CRGB::White,CRGB::White};

 
void setup() {
    Serial.begin(115200);
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

    
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );

    dimTimer = millis();
}

 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(SAMPLE_PIN);
        vImag[i] = 0;

        timedDim();
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    int peakBin = (int)peak * SAMPLES / SAMPLING_FREQUENCY;


    // SHOWING RESULTS


    // Peak Results
//    Serial.println(peak);     //Print out what frequency is the most dominant.


    // Show all outputs 
//    for(int i=0; i<(SAMPLES/2); i++)
//    {         
//        Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
//        Serial.print(" ");
//        Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
//    }

    // Show plot
//    for(int i=0; i<(SAMPLES/2); i++)
//    {         
//        Serial.println(vReal[i], 1);   
//    }


    for (int i = 0; i < (SAMPLES/2); i++) {
      int freq = (i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES;
      if (freq < 100 && freq > 30) {
        baseAvg += vReal[i];
        baseCount++;
      }
      if (freq > 120 && freq < 250) {
        midAvg += vReal[i];
        midCount++;
      }
      if (freq > 250) {
        highAvg += vReal[i];
        highCount++;
      }

    }
      baseAvg /= baseCount;
      midAvg /= midCount;
      highAvg /= highCount;
      
    if (baseAvg > prevBase * 2 && baseAvg > 150) {
      patternB();
    }
    else if (midAvg > midPrevAvg * 1.5 && midAvg > 150) {
      patternA();
    }
    else if (highAvg > highPrevAvg * 1.5 && highAvg > 120) {
      patternC();
    }

    
    prevBase = baseAvg;
    midPrevAvg = midAvg;
    highPrevAvg = highAvg;
    baseCount = 0;
    midCount = 0;
    highCount = 0;
    baseAvg = 0;
    midAvg = 0;
    highAvg = 0;
//
//    while(1);
}

void timedDim() {
  if (millis() - dimTimer > dimPeriod && brightness > 0) {
    brightness --;
    
    FastLED.setBrightness(  brightness );
    FastLED.show();
    dimTimer = millis();
    
  }
}


void patternA() {
  for (int i = 0; i < NUM_LEDS;i++) {
    leds[i] = designA[i];
  }
  brightness = BRIGHTNESS;
}

void patternB() {
  for (int i = 0; i < NUM_LEDS;i++) {
    leds[i] = designB[i];
  }
  brightness = BRIGHTNESS;
}

void patternC() {
  for (int i = 0; i < NUM_LEDS;i++) {
    leds[i] = designC[i];
  }
  brightness = BRIGHTNESS;
}

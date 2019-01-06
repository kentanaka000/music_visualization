#include <arduinoFFT.h>
#include <FastLED.h>

//FFT constants
#define SAMPLES 128             //Must be a power of 2
#define SAMPLING_FREQUENCY 8000 //Hz, must be less than 10000 due to ADC
#define SAMPLE_PIN 5

//LED constants
#define LED_PIN     9
#define NUM_LEDS    11
#define BRIGHTNESS  32
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

//LED vars
CRGB leds[NUM_LEDS];
CRGB off[NUM_LEDS];
int brightness = 32;


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
double otherAvg;
double prevOther;
int otherCount;

//designs
CRGB designA[NUM_LEDS] {CRGB::Red, CRGB::White, CRGB::Red, CRGB::White,CRGB::Red, CRGB::White,CRGB::Red, CRGB::White} ;
CRGB designB[NUM_LEDS] {CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Purple};
 
void setup() {
    Serial.begin(115200);
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

    
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );

    
    
}
 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(SAMPLE_PIN);
        vImag[i] = 0;
     
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
//    Serial.println(vReal[peakBin], 1);  // value of the peak


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
      if (freq < 250 && freq > 100) {
        baseAvg += vReal[i];
        baseCount++;
      }
      if (freq > 500) {
        otherAvg += vReal[i];
        otherCount++;
      }


    }
      baseAvg /= baseCount;
      otherAvg /= otherCount;
    if (otherAvg > prevOther * 3 && otherAvg > 100 && peak > 500) {
      patternA();
    }
    if (baseAvg > prevBase * 2 && baseAvg > 150) {
      patternB();
    }

    
    prevBase = baseAvg;
    prevOther = otherAvg;
    baseCount = 0;
    otherCount = 0;
    baseAvg = 0;
    otherAvg = 0;

    if (brightness > 0) {
      brightness -=2;
      
      FastLED.setBrightness(  brightness );
      FastLED.show();
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

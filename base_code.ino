#include <arduinoFFT.h>
#include <FastLED.h>

//FFT constants
#define SAMPLES 32            //Must be a power of 2
#define SAMPLING_FREQUENCY 1000 //Hz, must be less than 10000 due to ADC
#define SAMPLE_PIN 5

//LED constants
#define LED_PIN     3
#define NUM_LEDS    8
#define MAX_BRIGHTNESS  32
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

//LED vars
CRGB leds[NUM_LEDS];
CRGB rainbowColors[6] = {CRGB::Red, CRGB::Green, CRGB::Orange, CRGB::Blue, CRGB::Yellow,  CRGB::Purple};
int colorIndex = 0;
unsigned long changeTimer;

//FFT vars
arduinoFFT FFT = arduinoFFT();
unsigned long sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];


void setup() {
    Serial.begin(9600);
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

    delay( 2000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  MAX_BRIGHTNESS );

    changeTimer = millis();
}

 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(SAMPLE_PIN);
        vImag[i] = 0;
     
        while(micros() - microseconds < sampling_period_us){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    int peakBin = (int)peak * SAMPLES / SAMPLING_FREQUENCY;

    int averageLoudness = 0;
    for (int i = 2; i < (SAMPLES/2); i++) {
      averageLoudness += vReal[i];
    }
    averageLoudness /= SAMPLES/2 - 2;


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

    
    if ((vReal[peakBin] > averageLoudness + 400 || (vReal[peakBin] > 1000 && peakBin > 1)) && millis() - changeTimer > 200) {
      colorIndex += 1;
      if (colorIndex > 6) {
        colorIndex = 0;
      }

      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = rainbowColors[colorIndex];
      }
      changeTimer = millis();
      FastLED.show();
    }
    
}

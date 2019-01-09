#include <arduinoFFT.h>
#include <FastLED.h>

//FFT constants
#define SAMPLES 32            //Must be a power of 2
#define SAMPLING_FREQUENCY 1000 //Hz, must be less than 10000 due to ADC
#define SAMPLE_PIN 5

//LED constants
#define LED_PIN     3
#define NUM_LEDS    10
#define BRIGHTNESS  32
#define LED_TYPE    WS2811

//LED vars
CRGB leds[NUM_LEDS];

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
    FastLED.setBrightness(  BRIGHTNESS );
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

    CRGB fillColor;
    
    if (vReal[peakBin] > 1500 && peakBin > 2) {
      if(peak < 150) {
        fillColor = CRGB::Blue;
      }
      else if(peak < 300) {
        fillColor = CRGB::Red;
      }
      else {
        fillColor = CRGB::White;
      }

      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = fillColor;
      }
      FastLed.show();
    }
    
}
